#include "xlogutility.h"

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
namespace xlog {
filelist xLogUtility::findAllLogOrZipFile(const std::string &logDir, bool isLog) {
  if (logDir.empty()) {
    return {};
  }
  auto dir = opendir(logDir.c_str());
  if (!dir) {
    return {};
  }

  std::vector<std::string> allFiles{};
  struct dirent *one{nullptr};
  while ((one = readdir(dir)) != nullptr) {
    if (strcmp(one->d_name, ".") == 0 || strcmp(one->d_name, "..") == 0) {
      continue;
    }
    if (one->d_type == DT_REG) {
      auto filepath = logDir + "/" + std::string(one->d_name);
      if (isLog && xLogUtility::isLogFile(filepath)) {
        allFiles.push_back(filepath);
      } else if (!isLog && xLogUtility::isZipFile(filepath)) {
        allFiles.push_back(filepath);
      }
    }
  }
  closedir(dir);  // dont not forget close dir
  return allFiles;
}

bool xLogUtility::isLogFile(const std::string &path) {
  auto ext = path.substr(path.rfind('.') + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return (ext == "txt");
}
bool xLogUtility::isZipFile(const std::string &path) {
  auto pos = path.rfind('.');
  auto ext = path.substr(path.find_last_of('.', pos - 1) + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
  return (ext == "tar.gz");
}
time_t xLogUtility::string2Date(const std::string &time) {
  auto splitCount = [](const std::string &time) -> std::vector<std::string> {
    size_t pos = std::string::npos, index = 0;
    std::vector<std::string> split{};
    do {
      pos = time.find('-', index);
      if (pos != std::string::npos) {
        auto tmp = time.substr(index, pos - index);
        split.push_back(tmp);
        index = pos + 1;
      }
    } while (pos != std::string::npos);
    auto tmp = time.substr(index);
    split.push_back(tmp);
    return split;
  };
  auto isNumber = [](const std::string &time) {
    auto ok = false;
    for (const auto &c : time) {
      if (c == '-') continue;
      if (c > '9' || c < '0') {
        ok = false;
        break;
      }
      ok = true;
    }
    return ok;
  };
  if (time.empty() || !isNumber(time)) {
    return 0;
  }

  auto split = splitCount(time);
  if (split.size() != 3) {
    return 0;
  }

  struct tm tmpTime {};
  tmpTime.tm_year = std::stoi(split.at(0)) - 1900;
  tmpTime.tm_mon = std::stoi(split.at(1)) - 1;
  tmpTime.tm_mday = std::stoi(split.at(2));
  auto realTime = mktime(&tmpTime);
  if (-1 == realTime) {
    return 0;
  }
  return realTime;
}
std::string xLogUtility::date2string(time_t time) {
  auto dateTime = localtime(&time);
  if (dateTime) {
    std::string month{}, day{};
    if (dateTime->tm_mon < 9) {
      month = "0" + std::to_string(dateTime->tm_mon + 1);
    } else {
      month = std::to_string(dateTime->tm_mon + 1);
    }
    if (dateTime->tm_mday < 10) {
      day = "0" + std::to_string(dateTime->tm_mday);
    } else {
      day = std::to_string(dateTime->tm_mday);
    }
    auto zipName = std::to_string(dateTime->tm_year + 1900) + "-" + month + "-" + day;
    return zipName;
  }
  return "";
}
time_t xLogUtility::path2LogDate(const std::string &path) {
  auto timeString = path;
  auto pos = timeString.rfind('/');
  if (pos > 0) {
    timeString = timeString.substr(pos + 1);
  }
  pos = timeString.rfind('_');
  if (pos > 0) {
    timeString = timeString.substr(pos + 1);
  }
  timeString = timeString.substr(0, timeString.find('.'));
  return string2Date(timeString);
}
time_t xLogUtility::path2ZipDate(const std::string &path) {
  auto timeString = path;
  auto pos = timeString.rfind('/');
  if (pos > 0) {
    timeString = timeString.substr(pos + 1);
  }
  pos = timeString.find('.');
  if (pos > 0) {
    timeString = timeString.substr(0, pos);
  }
  return string2Date(timeString);
}
std::string xLogUtility::getAppName() {
  std::string name{};
  char path[1000]{0};
  auto len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len < 0 || size_t(len) > sizeof(path)) {
    name = "";
  } else {
    name = std::string(path);
    name = name.substr(name.rfind('/') + 1);
  }
  return name;
}
std::string xLogUtility::getAppDir() {
  std::string dir{};
  char path[1000]{0};
  auto len = readlink("/proc/self/exe", path, sizeof(path) - 1);
  if (len < 0 || size_t(len) > sizeof(path)) {
    dir = "";
  } else {
    dir = std::string(path);
    dir = dir.substr(0, dir.rfind('/') + 1);
  }
  return dir;
}

bool xLogUtility::isFolderExist(const std::string &folder) {
  int ret = 0;

  ret = access(folder.c_str(), 0);
  if (ret == 0)
    ret = true;
  else
    ret = false;
  return ret;
}

int32_t xLogUtility::createDirectory(const std::string &directoryPath) {
  if (directoryPath.empty()) {
    return -1;
  }
  if (directoryPath.size() > FILENAME_MAX) {
    return -1;
  }
  std::string tmpDirPath(FILENAME_MAX, 0);
  for (unsigned long i = 0; i < directoryPath.size(); ++i) {
    tmpDirPath.at(i) = directoryPath.at(i);
    if (tmpDirPath.at(i) == '/') {
      if (!isFolderExist(tmpDirPath)) {
        int ret = mkdir(tmpDirPath.c_str(), 0777);
        if (ret != 0) return -1;
      }
    }
  }
  return 0;
}

}  // namespace xlog
