#ifndef XLOGUTILITY_H
#define XLOGUTILITY_H
#include <string>
#include <vector>
namespace xlog {
using filelist = std::vector<std::string>;
class xLogUtility {
 public:
  xLogUtility() = default;
  static filelist findAllLogOrZipFile(const std::string &logDir, bool isLog);
  static bool isLogFile(const std::string &path);
  static bool isZipFile(const std::string &path);
  static time_t string2Date(const std::string &time);
  static std::string date2string(time_t time);
  static time_t path2LogDate(const std::string &path);
  static time_t path2ZipDate(const std::string &path);
  static std::string getAppName();
  static std::string getAppDir();
  static bool isFolderExist(const std::string &folder);
  static int32_t createDirectory(const std::string &directoryPath);
};
}  // namespace xlog
#endif  // XLOGUTILITY_H
