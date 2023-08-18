//
// Created by caizc on 6/14/22.
//

#include "SafeQueue.h"

namespace enflame {

void SafeQueue::clear() {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_queue.clear();
  m_size.store(m_queue.size(), std::memory_order_release);
  m_syncId = 0;
  m_frameCount = 0;
  m_inFps.restart();
  m_outFps.restart();
  m_latestTimeStamp = TimePoint(NanoSeconds(0));
  m_firstFetchTime = TimePoint(NanoSeconds(0));
}

// this function only effective when QUEUE_TYPE_SAMPLE
SafeQueue &SafeQueue::configInputFps(float fps) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.inputFps = fps;
  return *this;
}

SafeQueue &SafeQueue::configLogFlag(bool flag) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_logFlag = flag;
  return *this;
}

// this function only effective when QUEUE_TYPE_CLOCK
SafeQueue &SafeQueue::configOutputFps(float fps) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.targetFps = fps;
  return *this;
}

SafeQueue &SafeQueue::configType(QueueType type) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.queueType = type;

  // the size of sample queue must be >= 2
  if (m_settings.queueType == QUEUE_TYPE_SAMPLE) m_settings.length = (m_settings.length <= 1) ? 2 : m_settings.length;

  return *this;
}

SafeQueue &SafeQueue::config(const QueueSettings &settings) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings = settings;
  return *this;
}

// latencyMS only effective when QUEUE_TYPE_DUPLEX
SafeQueue &SafeQueue::configLatency(int latencyMS) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.latencyMS = latencyMS;
  return *this;
}

// length only effective when m_settings.dropOnFull is set
SafeQueue &SafeQueue::configLength(size_t length) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.length = length;

  // the size of sample queue must be >= 2
  if (m_settings.queueType == QUEUE_TYPE_SAMPLE) m_settings.length = (m_settings.length <= 1) ? 2 : m_settings.length;

  return *this;
}

SafeQueue &SafeQueue::configDrop(bool dropOnFull) {
  std::lock_guard<std::mutex> lock(m_mutex);
  m_settings.dropOnFull = dropOnFull;
  return *this;
}

void SafeQueue::setQNamePrefix(const std::string &prefix) {
  std::lock_guard<std::mutex> lock(m_mutex);
  InstanceName::setNamePrefix(prefix);
  m_inFps.setNamePrefix(fullname());
  m_outFps.setNamePrefix(fullname());
}

void SafeQueue::debugPrint() {
  std::lock_guard<std::mutex> lock(m_mutex);
  LOG_INFO("[{}-safe]: size={}", fullname(), size());
}

void SafeQueue::add(const JobPkgPtr &in) {
  xassert(in.get());

  std::lock_guard<std::mutex> lock(m_mutex);
  m_queue.push_back(in);
  if (m_settings.queueType == QUEUE_TYPE_CLOCK) m_inFps.tick();

  while (m_settings.dropOnFull && m_queue.size() > m_settings.length) {
    m_queue.erase(m_queue.begin());
    m_frameCount++;
    if (m_logFlag)
      LOG_INTERVAL(300, xlog::Log_Level::LL_INFO, "[{}] a data is dropped when adding to the full queue. Size={}.",
                   fullname(), m_queue.size());
  }
  m_size.store(m_queue.size(), std::memory_order_release);
}

bool SafeQueue::fetch(JobPkgPtr &out) {
  if (m_settings.queueType == QUEUE_TYPE_BASIC) return fetchBasic(out);
  if (m_settings.queueType == QUEUE_TYPE_CLOCK) return fetchClock(out);
  if (m_settings.queueType == QUEUE_TYPE_SAMPLE) return fetchSample(out);
  if (m_settings.queueType == QUEUE_TYPE_DUPLEX) return fetchDuplex(out);
  if (m_settings.queueType == QUEUE_TYPE_ORDERED) return fetchOrdered(out);
  xassert(false);
  return false;
}

bool SafeQueue::fetchBasic(JobPkgPtr &out) {
  if (empty()) return false;

  std::lock_guard<std::mutex> lock(m_mutex);
  if (!empty()) {
    out = m_queue.front();
    // m_outFps.tick();
    m_queue.erase(m_queue.begin());
    m_frameCount++;
    m_size.store(m_queue.size(), std::memory_order_release);
    return true;
  }
  return false;
}

bool SafeQueue::fetchDuplex(JobPkgPtr &out) {
  xassert(m_settings.latencyMS > 0);

  // to reduce the chance of reversed order, check if the size is less than 2.
  // this is a temporary trick.
  if (size() < 2) return false;

  std::lock_guard<std::mutex> lock(m_mutex);
  if (size() >= 2) {
    std::sort(this->m_queue.begin(), this->m_queue.end(),
              [&](auto lhs, auto rhs) { return lhs->timeStamp < rhs->timeStamp; });

    auto now = std::chrono::high_resolution_clock::now();
    if (m_queue.front()->timeStamp < m_latestTimeStamp && m_latestTimeStamp > TimePoint(NanoSeconds(0))) {
      // this data is in wrong sequence (earlier than the latest output).
      // drop it (regardless of the value of m_settings.dropOnFull).
      // m_outFps.tick();
      m_queue.erase(m_queue.begin());
      m_frameCount++;
      m_size.store(m_queue.size(), std::memory_order_release);
      if (m_logFlag)
        LOG_WARN(
            "[{}-duplex] a frame was dropped when fetching, due to reversed"
            "order. Size={}",
            fullname(), m_queue.size());
    }

    auto checkPoint = now - std::chrono::milliseconds(m_settings.latencyMS);
    if (m_queue.front()->timeStamp < checkPoint) {
      // This data has been staying in the queue for more than required time.
      // pop up data
      out = m_queue.front();
      // m_outFps.tick();
      m_queue.erase(m_queue.begin());
      m_frameCount++;
      m_size.store(m_queue.size(), std::memory_order_release);
      m_latestTimeStamp = out->timeStamp;
      return true;
    }
  }
  return false;
}

bool SafeQueue::fetchClock(JobPkgPtr &out) {
  if (empty()) return false;

  std::lock_guard<std::mutex> lock(m_mutex);
  // before outputing the first frame, the input queue should be in half
  // size of the limit.
  if (!empty() && (m_inFps.getTotalTicks() > static_cast<long long>(m_settings.length) / 2)) {
    double nsPerFrame = double(nanoUnit) / double(m_settings.targetFps);
    auto now = FpsGauge::currentTimePoint();
    auto nextfetchTime = m_latestTimeStamp + std::chrono::nanoseconds(static_cast<long long>(nsPerFrame));
    if (now > nextfetchTime) {
      out = m_queue.front();
      m_outFps.tick();
      m_queue.erase(m_queue.begin());
      m_size.store(m_queue.size(), std::memory_order_release);
      m_latestTimeStamp = now;
      m_frameCount++;

      // update target FPS
      auto inFps = float(m_inFps.getFps());
      auto outFps = float(m_outFps.getFps());
      m_settings.targetFps =
          0.5f + inFps + .1f * (inFps - outFps) + .1f * (size() - .5f * m_settings.length) / m_settings.length;
      return true;
    }
  }
  return false;
}

bool SafeQueue::fetchSample(JobPkgPtr &out) {
  if (empty()) return false;

  std::lock_guard<std::mutex> lock(m_mutex);
  if (!empty()) {
    auto now = FpsGauge::currentTimePoint();
    if (m_frameCount == 0) {
      // return the first frame
      out = m_queue.front();
      // m_outFps.tick();
      m_queue.erase(m_queue.begin());
      m_size.store(m_queue.size(), std::memory_order_release);
      m_firstFetchTime = now;
      m_frameCount++;
      return true;
    }

    double nsElapsed = (now - m_firstFetchTime).count();
    double nsPerFrame = double(nanoUnit) / double(m_settings.inputFps);

    while (true) {
      if (m_queue.size() < 2) break;  // never drop the only frame.
      const long long toleranceFrames = 10;
      if (nsElapsed / nsPerFrame > m_frameCount + toleranceFrames) {  // NOLINT
        // the front data was not fetched on time.  drop it (if it is not
        // the only frame in the queue).
        m_queue.erase(m_queue.begin());
        m_size.store(m_queue.size(), std::memory_order_release);
        m_frameCount++;
        if (m_logFlag)
          LOG_WARN(
              "[{}-sample] a frame was dropped when fetching, due to big "
              "latency. Size={}",
              fullname(), m_queue.size());
      } else {
        break;
      }
    }

    // return the front frame if the elasped time is longer than the frame's
    // time-point
    if (!m_queue.empty() && nsElapsed / nsPerFrame + 1 > m_frameCount) {
      out = m_queue.front();
      m_latestTimeStamp = now;
      // m_outFps.tick();
      m_queue.erase(m_queue.begin());
      m_frameCount++;
      m_size.store(m_queue.size(), std::memory_order_release);
      return true;
    }
  }
  return false;
}

bool SafeQueue::fetchOrdered(JobPkgPtr &out) {
  if (empty()) return false;

  std::lock_guard<std::mutex> lock(m_mutex);
  if (!empty()) {
    std::sort(m_queue.begin(), m_queue.end(), [&](auto lhs, auto rhs) { return lhs->syncId < rhs->syncId; });

    // drop expired or disordered packages
    auto now = FpsGauge::currentTimePoint();
    auto latency = std::chrono::milliseconds(m_settings.latencyMS);
    auto chkpnt = now - std::chrono::milliseconds(m_settings.latencyMS);
    while (!m_queue.empty()) {
      auto &front = m_queue.front();
      bool isExpired = front->timeStamp < chkpnt;
      bool isDisordered = front->syncId < m_syncId;
      if (isExpired || isDisordered) {
        m_queue.erase(m_queue.begin());
        m_size.store(m_queue.size(), std::memory_order_release);
        m_frameCount++;
        if (m_logFlag) LOG_WARN("[{}-ordered] a packages in ordered queue was discarded.", fullname());
        continue;
      }
      bool isInLongIdle = now > m_latestTimeStamp + latency;
      bool isFrameMissing = front->syncId > m_syncId && isInLongIdle;
      if (isFrameMissing) {
        if (m_logFlag) LOG_WARN("[{}-ordered] A missing frame is skipped.", fullname());
        m_syncId = front->syncId;  // skip missing frames
        break;
      }
      break;
    }

    if (empty()) return false;

    // pop the data if the syncId matches or the data has stayed in queue for
    // too long time
    auto &front = m_queue.front();
    if (front->syncId == m_syncId || front->timeStamp < chkpnt) {
      out = front;
      m_syncId = front->syncId + 1;
      m_queue.erase(m_queue.begin());
      // m_outFps.tick();
      m_size.store(m_queue.size(), std::memory_order_release);
      m_frameCount++;
      m_latestTimeStamp = now;  // update last successful fetch time
      return true;
    }
  }
  return false;
}
}  // namespace enflame