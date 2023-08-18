//
// Created by caizc on 6/7/22.
//

#include "InstanceName.h"

#include <assert.h>

namespace enflame {
InstanceName::InstanceName(std::string briefName)
    : m_briefName(std::move(briefName)) {
  updateFullName();
}

InstanceName::InstanceName() { updateFullName(); }

void InstanceName::updateName(const std::string &prefix, int num) {
  updateName(prefix, std::to_string(num));
}

void InstanceName::updateNameNum(const std::string &prefix, int num) {
  updateName(prefix, std::to_string(num));
}

void InstanceName::updateName(const std::string &prefix,
                              const std::string &postfix) {
  setNamePrefix(prefix);
  setNamePostfix(postfix);
}

void InstanceName::setBriefName(const std::string &name) {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  assert(!name.empty());
  m_briefName = name;
  updateFullName();
}
void InstanceName::setNamePrefix(const std::string &prefix) {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  m_prefix = prefix;
  updateFullName();
}

void InstanceName::setNamePostfix(const std::string &postfix) {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  assert(!postfix.empty());
  m_postfix = postfix;
  updateFullName();
}

void InstanceName::setNamePostfix(int num) {
  setNamePostfix(std::to_string(num));
}

std::string InstanceName::getNamePostfix() {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  return m_postfix;
}

std::string InstanceName::getBriefName() {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  return m_briefName;
}

std::string InstanceName::getFullname() {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  return m_fullName;
}

std::string InstanceName::getNamePrefix() {
  std::lock_guard<std::mutex> lock(m_mutexForName);
  return m_prefix;
}

void InstanceName::updateFullName() {
  assert(!m_briefName.empty());
  m_fullName = m_briefName;
  if (m_prefix.length() > 0) {
    m_fullName = m_prefix + m_prefixDelimiter + m_fullName;
  }
  if (m_postfix.length() > 0) {
    m_fullName = m_fullName + m_numDelimiter + m_postfix;
  }
}

}  // namespace enflame