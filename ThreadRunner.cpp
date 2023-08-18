//
// Created by caizc on 6/7/22.
//

#include "ThreadRunner.h"

#include "xlog/xlog.h"

// using namespace xlog;

namespace enflame {
ThreadRunner::~ThreadRunner() {
  // note: do not call virtual functions here.
  // stopAndWait must be called prior to deconstruction.
  xassert(!isRunning() && !isPreparing());
  // if this assert is triggered it is highly possible that
  // stopAndWait() is not called before deconstruction

  // when ThreadRunner::run() exits, the thread is not joined.
  if (m_thread && m_thread->joinable()) {
    m_thread->join();
  }
  LOG_INFO("[{}] Runner destructed.", fullname());
}

bool ThreadRunner::isSuppored(ThreadMode mode) const {
  auto supporedMode = getSupportedMode();
  return (int(supporedMode) & int(mode)) == int(mode);
}

void ThreadRunner::setRunningMode(ThreadMode mode) {
  xassert(!isRunning() && !isPreparing());
  xassert(isSuppored(mode));
  xassert(mode != ThreadMode::AllModes);
  m_runningMode = mode;
}

// must be called frequencty in async mode
void ThreadRunner::pump() {
  if (!isRunning()) {
    return;
  }
  if (isSuppored(ThreadMode::Async) && getRunningMode() == ThreadMode::Async) {
    runPump();
  }
}

ThreadMode ThreadRunner::getSupportedMode() const {
  // Override this method to tell the supper-class what mode the sub-class
  // supports.  Defaulted to ThreadMode::threaded.
  return ThreadMode::Threaded;
}

void ThreadRunner::runPre() {
  // defaulted to termination.
  // Must not run in async mode when it is not implemented.
  xassert(false);
}

void ThreadRunner::runPump() {
  // This method must exit ASAP, usuallly at several-milliseconds level.
  // Do do long time operation here. This defaulted implementation is to do
  // nohting, so that calling pump() in Thread mode is safe but of no use.

  // For threaed mode, the implementation should be like this:
  //  if (!needToStop) {
  //    // codes in loop
  //  }
}

void ThreadRunner::runPost() {
  // defaulted to termination.
  // Must not run in async mode when it is not implemented.
  xassert(false);
}

void ThreadRunner::run() {
  // No need to implement this method if it supports Async mode.
  // Implement runPre(), runPump(), runPost() instead.

  // For threaed mode, the implementation should be like this:
  //  while (!needToStop) {
  //    // codes in loop
  //  }
  xassert(false);
}

void ThreadRunner::startRun() {
  if (isRunning()) {
    LOG_WARN("[{}] Multiple attemps to start run.", fullname());
    return;
  }

  if (m_runningMode == ThreadMode::Async) {
    LOG_INFO("[{}] +++ start a new async-runner +++", fullname());
    m_state.store(ThreadState::Running, std::memory_order_release);
    runPre();
    return;
  }

  xassert(m_runningMode == ThreadMode::Threaded);
  LOG_INFO("[{}] +++ start a new thread +++", fullname());
  // maybe we could join the thread first if m_thread holds a running thread
  xassert(!m_thread);
  m_state.store(ThreadState::Running, std::memory_order_release);
  // do not use detached thread if the thread accesses local variables
  m_thread = std::make_unique<std::thread>(&ThreadRunner::threadFuncForRun, this);

  std::string thdName;
  auto fn = fullname();
  if (fn.length() > 15) {
    thdName += "*";
    thdName += fn.substr(fn.length() - 14);
  } else
    thdName = fn;
  // set name will failed when thead name length is beyond 16
  int ret = pthread_setname_np(m_thread->native_handle(), thdName.c_str());
  std::string retStr = (ret == 0) ? "Success" : "Fail";
  LOG_INFO(
      "Created a new Thread, fullname: {}, set threadname: {}, set result: "
      "{}",
      fn, thdName, retStr);
}

void ThreadRunner::prepareAsync() {
  // if m_state is 'expected', then set m_state 'desired'
  // and return true. This function is not frequently called,
  // so we can use the default memory order, i.e. memory_order_seq_cst.
  // More efficiently, we can use memory_order_acq_rel
  ThreadState expected = ThreadState::Fresh;
  ThreadState desired = ThreadState::Preparing;
  if (m_state.compare_exchange_strong(expected, desired)) {
    xassert(ThreadState::Preparing == getState());
    LOG_INFO("[{}] === prepare in a new thread ===", fullname());
    m_prepare = std::async(std::launch::async, &ThreadRunner::threadFuncForPrepare, this);
  }
}

void ThreadRunner::prepareBlocked() {
  if (m_prepare.valid()) {
    // already running. Waiting for completion.
    m_prepare.wait();
    return;
  }
  ThreadState expected = ThreadState::Fresh;
  ThreadState desired = ThreadState::Preparing;
  if (m_state.compare_exchange_strong(expected, desired)) {
    xassert(ThreadState::Preparing == getState());
    LOG_INFO("[{}] === prepare in blocked mode ===", fullname());
    m_prepare = std::async(std::launch::deferred, &ThreadRunner::threadFuncForPrepare, this);
    m_prepare.wait();
  }
}

void ThreadRunner::stopAndWait() {
  // note: do not call virtual functions here!
  LOG_INFO("[{}] Stop thread and wait ...", fullname());
  if (m_prepare.valid()) m_prepare.wait();

  stop();
  if (m_thread && m_thread->joinable()) {
    m_thread->join();
  }
  m_thread.reset();
  xassert(!m_thread);
  LOG_INFO("[{}] End of waiting.", fullname());
}

bool ThreadRunner::isPreparing() const { return ThreadState::Preparing == getState(); }

bool ThreadRunner::isRunning() const { return ThreadState::Running == getState(); }

void ThreadRunner::stop() {
  LOG_INFO("[{}] Stopping thread ...", fullname());
  m_stopFlag.store(true, std::memory_order_release);
  if (getRunningMode() == ThreadMode::Async) {
    runPost();
    m_state.store(ThreadState::Stopped, std::memory_order_release);
    m_stopFlag.store(false, std::memory_order_release);
    LOG_INFO("[{}] --- Exits async runner ---.", fullname());
  }
}

double ThreadRunner::get_task_cpu_usage() {
  auto now = std::chrono::steady_clock::now();
  auto duration = now - m_lastCpuMeasureTP;
  if (duration < std::chrono::seconds(3)) {
    return m_cpuUsage;
  }

  char fname[] = "/proc/thread-self/stat";
  FILE *fp = fopen(fname, "r");
  if (!fp) return -1;
  int ucpu = 0, scpu = 0, tot_cpu = 0;
  if (fscanf(fp, "%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d %d", &ucpu, &scpu) == 2) {
    tot_cpu = ucpu + scpu;
  }
  fclose(fp);

  auto msCount = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
  auto cpuUseTime = double(tot_cpu - m_lastCpuTime) * 10;
  m_cpuUsage = cpuUseTime / msCount;
  m_lastCpuTime = tot_cpu;
  m_lastCpuMeasureTP = now;
  return m_cpuUsage;
}

void ThreadRunner::threadFuncForRun() {
  xassert(isSuppored(ThreadMode::Threaded));
  xassert(getRunningMode() == ThreadMode::Threaded);

  bool prepareResult = false;
  if (!m_prepare.valid()) {  // has not been prepared
    LOG_INFO("[{}] Not prepared yet. Prepare before running.", fullname());
    prepareResult = prepareToRun();
  } else {
    prepareResult = m_prepare.get();  // prepared or Error
    m_state.store(ThreadState::Running, std::memory_order_release);
  }
  if (!prepareResult) {
    m_state.store(ThreadState::Error, std::memory_order_release);
    LOG_ERROR("[{}] Prepare failed!  Abort running thread.", fullname());
    return;
  }

  LOG_INFO("[{}] *** thread is running ***.", fullname());
  if (isSuppored(ThreadMode::Async)) {
    // this object supports Async mode. Call runPre(), runPump(), runPost().
    runPre();
    while (!needToStop()) {
      runPump();
      if (m_breathingMs > 0) {
        msleep(m_breathingMs);
      }
    }
    runPost();
  } else {
    // Async mode is not suppored.  Call run() instead of
    // runPre/runPump/RunPost
    LOG_INFO("[{}] *** thread is running function: run() ***.", fullname());
    run();
  }
  m_state.store(ThreadState::Stopped, std::memory_order_release);
  m_stopFlag.store(false, std::memory_order_release);
  LOG_INFO("[{}] --- Exits thread ---.", fullname());
}

bool ThreadRunner::threadFuncForPrepare() {
  bool ret = prepareToRun();
  if (ret) {
    m_state.store(ThreadState::Prepared, std::memory_order_release);
    LOG_INFO("[{}] Preparation finished.", fullname());
  } else {
    m_state.store(ThreadState::Error, std::memory_order_release);
    LOG_ERROR("[{}] Preparation failed.", fullname());
  }
  return ret;
}

bool ThreadRunner::prepareToRun() {
  return true;  // run before creating the thread
}

bool ThreadRunner::prepareMultiple(const ThreadRunnerList &runners, bool isSerial, int timeOutMs) {
  auto isTimeOut = [timeOutMs](decltype(std::chrono::steady_clock::now()) &start) {
    bool ret = false;
    if (timeOutMs > 0) {
      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = now - start;  // seconds
      ret = (int(diff.count() * 1000.) > timeOutMs);
    }
    return ret;
  };

  // prepare all runners
  for (auto &runner : runners) {
    if (runner) {
      if (isSerial)
        runner->prepareBlocked();
      else
        runner->prepareAsync();
    }
  }

  // check if all runners are prepared
  bool allPrepared = false;
  auto start = std::chrono::steady_clock::now();
  while (!allPrepared) {
    allPrepared = true;
    for (auto &runner : runners) {
      if (runner) {
        if (runner->isPreparing()) {
          LOG_INTERVAL(300, xlog::Log_Level::LL_WARN, "thread [{}] is preparing...", runner->fullname());
          allPrepared = false;
          break;
        }
      }
    }
    ThreadRunner::msleep(10);
    if (isTimeOut(start)) break;
  }
  return allPrepared;
}

bool ThreadRunner::startRunMultiple(const ThreadRunnerList &runners, bool isSerial, int timeOutMs) {
  auto isTimeOut = [timeOutMs](decltype(std::chrono::steady_clock::now()) &start) {
    bool ret = false;
    if (timeOutMs > 0) {
      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = now - start;  // seconds
      ret = (int(diff.count() * 1000.) > timeOutMs);
    }
    return ret;
  };

  // prepare all runners
  prepareMultiple(runners, isSerial, timeOutMs);

  // start componets
  for (auto &runner : runners) {
    if (runner) {
      runner->startRun();
      if (isSerial) {  // serial start run
        auto start = std::chrono::steady_clock::now();
        while (!runner->isRunning()) {
          LOG_INTERVAL(300, xlog::Log_Level::LL_WARN, "thread [{}] is starting to run...", runner->fullname());
          ThreadRunner::msleep(5);
          if (isTimeOut(start)) break;
        }
      }
    }
  }

  // check if all workers are running
  bool allRunning = false;
  auto start = std::chrono::steady_clock::now();
  while (!allRunning) {
    allRunning = true;
    for (auto &runner : runners) {
      if (runner) {
        if (!runner->isRunning()) {
          allRunning = false;
          LOG_INTERVAL(300, xlog::Log_Level::LL_WARN, "thread [{}] is starting to run...", runner->fullname());
          break;
        }
      }
    }
    ThreadRunner::msleep(10);
    if (isTimeOut(start)) break;
  }
  return allRunning;
}

bool ThreadRunner::stopAndWaitMultiple(const ThreadRunnerList &runners, bool /*isSerial*/, int timeOutMs) {
  // stop runners
  for (auto &runner : runners) {
    if (runner) {
      runner->stop();
    }
  }
  // check if all workers are running
  bool allStopped = false;
  auto start = std::chrono::steady_clock::now();
  while (!allStopped) {
    allStopped = true;
    for (auto &runner : runners) {
      if (runner) {
        if (runner->isRunning() || runner->isPreparing()) {
          allStopped = false;
          break;
        }
      }
    }
    ThreadRunner::msleep(10);
    if (timeOutMs > 0) {
      auto now = std::chrono::steady_clock::now();
      std::chrono::duration<double> diff = now - start;  // seconds
      if (int(diff.count() * 1000.) > timeOutMs) {
        break;
      }
    }
  }
  return allStopped;
}

void ThreadRunner::pumpMultiple(const ThreadRunnerList &runners) {
  // stop runners
  for (auto &runner : runners) {
    if (runner) {
      xassert(runner);
      xassert(runner->isRunning());
      runner->pump();
    }
  }
}
}  // namespace enflame