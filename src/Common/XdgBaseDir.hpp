#ifndef COMMON_XDGBASEDIR_H
#define COMMON_XDGBASEDIR_H

#include <vector>
#include <filesystem>

namespace Common
{

class XdgBaseDir
{
public:

  static std::filesystem::path configHome();
  static std::filesystem::path cacheHome();
  static std::filesystem::path dataHome();
  static std::filesystem::path stateHome();
  static std::vector<std::filesystem::path> dataDirs();
  static std::vector<std::filesystem::path> configDirs();

private:

  static XdgBaseDir &instance();
  XdgBaseDir();

  std::filesystem::path mHome;
  std::filesystem::path mConfigHome;
  std::filesystem::path mCacheHome;
  std::filesystem::path mDataHome;
  std::filesystem::path mStateHome;
  std::vector<std::filesystem::path> mDataDirs;
  std::vector<std::filesystem::path> mConfigDirs;

  static std::string readHome();
  std::string readConfigHome();
  std::string readDataHome();
  std::string readCacheHome();
  std::string readStateHome();
  std::vector<std::string> readDataDirs() const;
  std::vector<std::string> readConfigDirs() const;
};

}

#endif // COMMON_XDGBASEDIR_H
