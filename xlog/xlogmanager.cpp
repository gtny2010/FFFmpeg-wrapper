#include "xlogmanager.h"
#include "xfilemanage.h"
#include "xlog.h"
#include <cassert>
#include <iostream>
namespace xlog {
//////////////////////////////////////////////////////////////////////////////////////
xLogManager::xLogManager(const std::string &logDir, int saveDays,
                         int packDeadline)
    : m_logFile(std::make_shared<xFileManage>(logDir, saveDays, packDeadline)) {
  m_isRun.store(true, std::memory_order::memory_order_release);
  m_logCheckTh = std::thread(&xLogManager::logStateCheck, this);
  pthread_setname_np(m_logCheckTh.native_handle(), "logmanager");
}
xLogManager::~xLogManager() {
  m_isRun.store(false, std::memory_order::memory_order_release);
  if (m_logCheckTh.joinable()) {
    m_logCheckTh.join();
  }
  LOG_INFO("~~~~~~~~~~~~~~~~~~~xlogmanager");
}
bool xLogManager::isLive() {
  return m_isRun.load(std::memory_order::memory_order_acquire);
}
void xLogManager::logStateCheck() {
  assert(m_logFile);
  while (m_isRun.load(std::memory_order::memory_order_acquire)) {
    m_logFile->compressAndPack();
    m_logFile->removeStaledLog();
    std::this_thread::sleep_for(std::chrono::seconds(3));
  }
  m_isRun.store(false, std::memory_order::memory_order_release);
}
} // namespace xlog
