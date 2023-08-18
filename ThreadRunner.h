//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON_THREADRUNNER_H
#define TESTDAMEON_THREADRUNNER_H

#include <syscall.h>
#include <unistd.h>

#include <atomic>
#include <future>
#include <memory>
#include <thread>
#include <vector>

#include "InstanceName.h"

namespace enflame {
enum class ThreadMode { Threaded = 0x1, Async = 0x2, AllModes = 0x3 };
enum class ThreadState {
  Fresh,
  Preparing,
  Prepared,
  Running,
  Stopped,
  Error,
};

class ThreadRunner;
using ThreadRunnerList = std::vector<std::shared_ptr<ThreadRunner>>;

class ThreadRunner : public InstanceName {
 public:
  explicit ThreadRunner(const std::string &name) : InstanceName(name) {}
  ThreadRunner() = delete;
  ThreadRunner(const ThreadRunner &) = delete;
  ThreadRunner(ThreadRunner &&) = delete;
  ThreadRunner &operator=(const ThreadRunner &) = delete;
  ThreadRunner &operator=(ThreadRunner &&) = delete;
  ~ThreadRunner() override;

  void prepareAsync();
  void prepareBlocked();
  void startRun();
  void stopAndWait();
  bool isPreparing() const;
  bool isRunning() const;
  void stop();

  bool isSuppored(ThreadMode mode) const;
  void setBreathing(size_t ms) { m_breathingMs = ms; }
  void setRunningMode(ThreadMode mode);
  ThreadMode getRunningMode() const { return m_runningMode; }
  void pump();  // must be called frequencty in async mode

  ThreadState getState() const { return m_state.load(std::memory_order_acquire); }

  double get_task_cpu_usage();
  static long get_task_id() { return syscall(SYS_gettid); }
  static void msleep(size_t msec) { std::this_thread::sleep_for(std::chrono::milliseconds(msec)); }
  static bool prepareMultiple(const ThreadRunnerList &runners, bool isSerial = true, int timeOutMs = -1);
  static bool startRunMultiple(const ThreadRunnerList &runners, bool isSerial = true, int timeOutMs = -1);
  static bool stopAndWaitMultiple(const ThreadRunnerList &runners, bool isSerial = false, int timeOutMs = -1);
  static void pumpMultiple(const ThreadRunnerList &runners);

 protected:
  void threadFuncForRun();
  bool threadFuncForPrepare();
  bool needToStop() const { return m_stopFlag.load(std::memory_order_acquire); }

 private:
  // Override this method to support Threaded only mode.  No need to override
  // to support Async mode.
  virtual void run();

  // override method to do preparations for both modes.
  virtual bool prepareToRun();

  // Override to notify the supper-class what mode the sub-class supports.
  // Default to return ThreadMode::threaded.
  virtual ThreadMode getSupportedMode() const;

  // Override this method to support Async mode, and no need to implement
  // run() anymore.
  virtual void runPre();

  // Override this method to support Async mode, and no need to implement
  // run() anymore.
  virtual void runPost();

  // Override this method to support Async mode. It must exit ASAP,
  // usuallly at several-milliseconds level. Don't do long time operation
  // inside. This default implementation is emtpy to do nohting, so that calling
  // pump() in Threaded mode is safe and doesn't cause any side effects.
  virtual void runPump();

 private:
  std::future<bool> m_prepare;
  std::unique_ptr<std::thread> m_thread{};
  std::atomic_bool m_stopFlag{false};
  std::atomic<ThreadState> m_state{ThreadState::Fresh};
  // sleeping time between callings of pump() in threaded mode
  size_t m_breathingMs{5};
  ThreadMode m_runningMode{ThreadMode::Threaded};

  double m_cpuUsage{0};
  long m_lastCpuTime{0};
  std::chrono::steady_clock::time_point m_lastCpuMeasureTP;
};
using ThreadRunnerPtr = std::shared_ptr<ThreadRunner>;
}  // namespace enflame

#endif  // TESTDAMEON_THREADRUNNER_H
