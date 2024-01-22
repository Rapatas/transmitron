#pragma once

#include <vector>
#include "Common/Filesystem.hpp"

namespace Common
{

class XdgBaseDir
{
public:

  static Common::fs::path configHome();
  static Common::fs::path cacheHome();
  static Common::fs::path dataHome();
  static Common::fs::path stateHome();
  static std::vector<Common::fs::path> dataDirs();
  static std::vector<Common::fs::path> configDirs();

private:

  static XdgBaseDir &instance();
  XdgBaseDir();

  Common::fs::path mHome;
  Common::fs::path mConfigHome;
  Common::fs::path mCacheHome;
  Common::fs::path mDataHome;
  Common::fs::path mStateHome;
  std::vector<Common::fs::path> mDataDirs;
  std::vector<Common::fs::path> mConfigDirs;

  static std::string readHome();
  std::string readConfigHome();
  std::string readDataHome();
  std::string readCacheHome();
  std::string readStateHome();
  std::vector<std::string> readDataDirs() const;
  std::vector<std::string> readConfigDirs() const;
};

} // namespace Common
