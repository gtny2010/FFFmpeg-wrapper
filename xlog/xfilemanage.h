#ifndef XFILEMANAGE_H
#define XFILEMANAGE_H
#include <string>
namespace xlog {
class xFileManage {
 public:
  explicit xFileManage(std::string logDir, int saveDays = 30,
                       int packDeadline = 1);

 public:
  void compressAndPack();
  void removeStaledLog();

 private:
  bool isNeedCompress(time_t oneTime);
  bool isNeedRemove(time_t oneTime);

 private:
  std::string m_logDir{};
  int m_logSaveDays{30};
  int m_logPackDeadline{1};
};
}  // namespace xlog
#endif  // XFILEMANAGE_H
