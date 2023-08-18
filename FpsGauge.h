//
// Created by caizc on 6/14/22.
//

#ifndef TESTDAMEON_FPSGAUGE_H
#define TESTDAMEON_FPSGAUGE_H

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

#include "InstanceName.h"
namespace enflame {
using namespace std::chrono_literals;
using NanoSeconds = std::chrono::nanoseconds;
using MilliSeconds = std::chrono::milliseconds;
using MicroSeconds = std::chrono::microseconds;
using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, NanoSeconds>;
using TickTock = std::pair<TimePoint, TimePoint>;

class FpsGauge : public InstanceName {
 public:
  const int DefaultWindowSize = 8000;

  explicit FpsGauge(const std::string &name) : InstanceName(name) { restart(); }
  explicit FpsGauge(int windowMilliseconds);

  long long getTotalTicks() const;
  double getTotalFps() const;
  double getFps() const;
  void restart();     // restart the gauge
  void tick();        // to mark to the start of working state
  void tock();        // to mark to the end of working state
  void tickTock();    // = tick() + tock()
  void debugPrint();  // print debug information

  template <class Rep, class Period>
  void debugPrint(const std::chrono::duration<Rep, Period> &dura);

  static TimePoint currentTimePoint();
  static std::string toDoubleSecondsString(TimePoint curTimePoint, TimePoint lastTimePoint = TimePoint{},
                                           int precision = -1);

 protected:
  // convert period variable to seconds in double type
  static double toDoubleSeconds(NanoSeconds period);
  void tockImpl();
  void updateFps();

  std::atomic<double> m_xFps{0.};
  std::atomic<double> m_xTotalFps{0.};
  std::atomic_llong m_xTotalTicks{0};
  TimePoint m_lastPrintedPoint = TimePoint(NanoSeconds(0));

  long long m_jobCount{0};
  int m_camId{0};

  bool m_lastTickOpen = false;
  TimePoint m_startPoint = std::chrono::time_point<std::chrono::high_resolution_clock>::min();

  // time window length for fps computation
  NanoSeconds m_windowSize = std::chrono::milliseconds(DefaultWindowSize);
  std::deque<TickTock> m_history;
  std::mutex m_mutex;
};

template <class Rep, class Period>
void FpsGauge::debugPrint(const std::chrono::duration<Rep, Period> &dura) {
  auto currPoint = currentTimePoint();
  auto last = m_lastPrintedPoint;
  if (last + dura < currPoint) {
    debugPrint();
    m_lastPrintedPoint = currPoint;
  }
}
}  // namespace enflame

#endif  // TESTDAMEON_FPSGAUGE_H
