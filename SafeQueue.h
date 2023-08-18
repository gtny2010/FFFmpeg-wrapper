//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_SAFEQUEUE_H
#define TESTDAMEON_SAFEQUEUE_H
#include "FpsGauge.h"
#include "JobPkgBase.h"
#include "xlog/xlog.h"

namespace enflame {
static const size_t QueueDefaultLength = 1;
static const int QueueDefaultLatencyMs = 1000;
static constexpr long nanoUnit = std::chrono::seconds(1) / std::chrono::nanoseconds(1);

enum QueueType {
  QUEUE_TYPE_BASIC,   // almost all queues
  QUEUE_TYPE_DUPLEX,  // no use
  QUEUE_TYPE_CLOCK,   // screen output queue
  QUEUE_TYPE_SAMPLE,  // capture queue
  QUEUE_TYPE_ORDERED  // no use
};

struct QueueSettings {
  QueueType queueType = QUEUE_TYPE_BASIC;
  size_t length = QueueDefaultLength;     // queue length
  int latencyMS = QueueDefaultLatencyMs;  // not meaningful for basic queue
  float inputFps = 25.f;                  // only effective for QUEUE_TYPE_SAMPLE
  float targetFps = 25.f;                 // only effective for QUEUE_TYPE_CLOCK
  bool dropOnFull = true;                 // if drop the front element when full
  QueueSettings() = default;
  QueueSettings(QueueType type, int len, int latency, bool drop) {
    queueType = type;
    length = size_t(len);
    latencyMS = latency;
    dropOnFull = drop;
  }
  QueueSettings(QueueType type, int len, bool drop) {
    queueType = type;
    length = size_t(len);
    dropOnFull = drop;
  }
};

// This is a ROUGHLY thread-safe queue class.
// Only an adder and/or an fetcher concurrently access the queue.
class SafeQueue : public InstanceName {
  // all public interfaces are thread safe
 public:
  using SelfType = SafeQueue;

  explicit SafeQueue(const std::string &name) : InstanceName(name), m_size(0) { clear(); }
  SafeQueue() : InstanceName("SafeQueueM"), m_size(0) { clear(); }

  // note: actual size may be > m_settings.length
  size_t size() const { return m_size.load(std::memory_order_acquire); }
  bool full() const { return size() >= m_settings.length; }
  bool empty() const { return size() == 0; }
  const auto &getSettings() { return m_settings; }

  void clear();

  JobPkgPtr front() {
    std::lock_guard<std::mutex> lock(m_mutex);
    xassert(!empty());
    return m_queue.front();
  }

  const FpsGauge &getInGauge() const { return m_inFps; }
  const FpsGauge &getOutGauge() const { return m_outFps; }
  SelfType &configLogFlag(bool flag);
  void debugPrint();
  SelfType &configInputFps(float fps);   // only effective for QUEUE_TYPE_SAMPLE
  SelfType &configOutputFps(float fps);  // only effective for QUEUE_TYPE_CLOCK
  SelfType &configType(QueueType type);
  // 'length' is only effective when m_settings.dropOnFull is set;
  SelfType &configLength(size_t length);
  // 'latencyMS' only effective when QUEUE_TYPE_DUPLEX
  SelfType &configLatency(int latencyMS);
  SelfType &configDrop(bool dropOnFull);
  void setQNamePrefix(const std::string &prefix);
  SelfType &config(const QueueSettings &setting);

  void add(const JobPkgPtr &in);
  bool fetch(JobPkgPtr &out);

  template <typename T>
  bool fetchT(std::shared_ptr<T> &out) {
    JobPkgPtr ptr;
    if (fetch(ptr)) {
      out = std::static_pointer_cast<T>(ptr);
      xassert(ptr.get());
      return true;
    } else {
      return false;
    }
  }

 protected:
  bool fetchBasic(JobPkgPtr &out);    // thread safe
  bool fetchDuplex(JobPkgPtr &out);   // thread safe
  bool fetchClock(JobPkgPtr &out);    // thread safe
  bool fetchSample(JobPkgPtr &out);   // thread safe
  bool fetchOrdered(JobPkgPtr &out);  // thread safe
  bool m_logFlag = true;
  uint64_t m_syncId = 0;
  // queue size.  For fast check without using mutex
  std::atomic_size_t m_size{0};
  // time point of last pop-up
  TimePoint m_latestTimeStamp = TimePoint(NanoSeconds(0));
  // time point of first pop-up
  TimePoint m_firstFetchTime = TimePoint(NanoSeconds(0));
  std::vector<JobPkgPtr> m_queue;
  std::mutex m_mutex;
  uint64_t m_frameCount = 0;
  FpsGauge m_inFps{"InFPS"};    // used by QUEUE_TYPE_CLOCK
  FpsGauge m_outFps{"OutFPS"};  // used by QUEUE_TYPE_CLOCK
  QueueSettings m_settings = {QUEUE_TYPE_BASIC, 1, true};
};
}  // namespace enflame

#endif  // TESTDAMEON_SAFEQUEUE_H
