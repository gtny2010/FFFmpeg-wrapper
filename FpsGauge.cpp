//
// Created by caizc on 6/14/22.
//

#include "FpsGauge.h"

#include <cassert>
#include <iomanip>
#include <iostream>

#include "xlog/xlog.h"

namespace enflame {

long long FpsGauge::getTotalTicks() const { return m_xTotalTicks.load(std::memory_order_acquire); }

double FpsGauge::getTotalFps() const { return m_xTotalFps.load(std::memory_order_acquire); }

double FpsGauge::getFps() const { return m_xFps.load(std::memory_order_acquire); }

void FpsGauge::debugPrint() { LOG_INFO("[{}]: fps={:.1f}/{:.1f}", fullname(), getFps(), getTotalFps()); }

TimePoint FpsGauge::currentTimePoint() { return std::chrono::high_resolution_clock::now(); }

double FpsGauge::toDoubleSeconds(NanoSeconds period) {
  const long long ratioSecond = 1000000000;
  return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, ratioSecond>>>(period).count() /
         double(ratioSecond);
}

std::string FpsGauge::toDoubleSecondsString(TimePoint curTimePoint, TimePoint lastTimePoint, int precision) {
  const long long ratioMilliSecond = 1000000000;
  NanoSeconds ns = curTimePoint - lastTimePoint;
  double tm = std::chrono::duration_cast<std::chrono::duration<double, std::ratio<1, ratioMilliSecond>>>(ns).count() /
              double(ratioMilliSecond);
  if (precision >= 0) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << tm;
    return ss.str();
  }
  return std::to_string(tm);
}

FpsGauge::FpsGauge(int windowMilliseconds) : m_windowSize(std::chrono::milliseconds(windowMilliseconds)) {
  xassert(windowMilliseconds > 0);
  restart();
}

void FpsGauge::restart() {
  m_history.clear();
  m_xFps.store(0., std::memory_order_release);
  m_xTotalFps.store(0., std::memory_order_release);
  m_xTotalTicks.store(0, std::memory_order_release);
  m_startPoint = currentTimePoint();
  m_lastPrintedPoint = currentTimePoint();
  m_jobCount = 0;
  m_lastTickOpen = false;
}

void FpsGauge::tick() {
  std::lock_guard<std::mutex> lock(m_mutex);
  // if tock() has not been called since the latest tick, call tock()
  if (m_lastTickOpen) tockImpl();

  m_history.emplace_back(TickTock(currentTimePoint(), TimePoint(NanoSeconds(0))));
  m_lastTickOpen = true;
  m_jobCount++;
  updateFps();
}

void FpsGauge::tock() {
  std::lock_guard<std::mutex> lock(m_mutex);
  tockImpl();
  updateFps();
}

void FpsGauge::tickTock() {
  tick();
  tock();
}

void FpsGauge::tockImpl() {
  if (m_history.empty()) return;
  TimePoint now = currentTimePoint();
  // deal with OS time updated
  if (now < m_history.back().first) {
    static size_t timeBackCount = 0;
    static NanoSeconds theMax{0};
    timeBackCount++;
    NanoSeconds t = m_history.back().first - now;
    theMax = (t > theMax) ? t : theMax;
    LOG_INFO("[{}]: Time set back times: {}, the max: {}ns, current duration: {}ns", fullname(), timeBackCount,
             theMax.count(), t.count());
  }

  if (m_lastTickOpen) {
    m_lastTickOpen = false;
  }

  m_xTotalTicks.fetch_add(1, std::memory_order_acq_rel);
  m_history.back().second = now;
}

void FpsGauge::updateFps() {
  if (m_history.empty()) {
    m_xFps.store(0., std::memory_order_release);
    return;
  }

  TimePoint windowEnd = currentTimePoint();
  TimePoint windowStart = windowEnd - m_windowSize;

  // delete obsolete tick-tock pairs
  while (m_history.front().second > m_history.front().first && m_history.front().second < windowStart) {
    m_history.erase(m_history.begin());
    if (m_history.empty()) {
      // no history data.  set fps and workload to zero
      m_xFps.store(0., std::memory_order_release);
      return;
    }
  }

  // caculate current fps
  if (m_history.size() == 1) {
    m_xFps.store(1.0 / toDoubleSeconds(m_windowSize), std::memory_order_release);
  } else {
    m_xFps.store(double(m_history.size() - 1) / toDoubleSeconds(m_history.back().first - m_history.front().first),
                 std::memory_order_release);
  }

  // caculate history fps
  m_xTotalFps.store(double(m_jobCount - 1) / toDoubleSeconds(m_history.back().first - m_startPoint),
                    std::memory_order_release);
}
}  // namespace enflame