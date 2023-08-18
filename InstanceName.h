//
// Created by caizc on 6/7/22.
//

#ifndef TESTDAMEON__INSTANCENAME_H
#define TESTDAMEON__INSTANCENAME_H
#include <mutex>
#include <string>
namespace enflame {
class InstanceName {
 public:
  explicit InstanceName(std::string briefName);
  InstanceName();
  InstanceName(const InstanceName &) = delete;
  InstanceName(InstanceName &&) = delete;
  InstanceName &operator=(const InstanceName &) = delete;
  InstanceName &operator=(InstanceName &&) = delete;
  virtual ~InstanceName() = default;

  void updateNameNum(const std::string &prefix, int num);
  void updateName(const std::string &prefix, int num);
  void updateName(const std::string &prefix, const std::string &postfix);
  void setNamePrefix(const std::string &prefix);
  void setNamePostfix(const std::string &postfix);
  void setNamePostfix(int num);
  std::string getBriefName();
  std::string getFullname();
  std::string getNamePrefix();
  std::string getNamePostfix();
  std::string fullname() { return getFullname(); }

 private:
  void setBriefName(const std::string &name);
  void updateFullName();
  std::string m_briefName{"Noname"};
  std::string m_prefix{""};
  std::string m_postfix{""};
  std::string m_fullName{"/Noname"};
  std::mutex m_mutexForName;
  const char *m_prefixDelimiter = "/";
  const char *m_numDelimiter = "#";
};
}  // namespace enflame
#endif  // TESTDAMEON__INSTANCENAME_H
