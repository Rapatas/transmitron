#include "XdgBaseDir.hpp"

#include <streambuf>

#include <fmt/core.h>

#include "Log.hpp"

using namespace Rapatas::Transmitron;
using namespace Rapatas::Transmitron::Common;

XdgBaseDir::XdgBaseDir() {
  auto logger = Log::create("Common::XDG");

  mHome = readHome();
  mConfigHome = readConfigHome();
  mCacheHome = readCacheHome();
  mDataHome = readDataHome();
  mStateHome = readStateHome();

  mHome.make_preferred();
  mConfigHome.make_preferred();
  mCacheHome.make_preferred();
  mDataHome.make_preferred();
  mStateHome.make_preferred();

  logger->debug("HOME:            {}", mHome.string());
  logger->debug("XDG_CONFIG_HOME: {}", mConfigHome.string());
  logger->debug("XDG_DATA_HOME:   {}", mDataHome.string());
  logger->debug("XDG_CACHE_HOME:  {}", mCacheHome.string());
  logger->debug("XDG_STATE_HOME:  {}", mStateHome.string());

  logger->debug("XDG_DATA_DIRS:");
  for (const auto &dir : readDataDirs()) {
    Common::fs::path path{dir};
    path.make_preferred();

    logger->debug(" - {}", path.string());
    mDataDirs.emplace_back(path);
  }

  logger->debug("XDG_CONFIG_DIRS:");
  for (const auto &dir : readConfigDirs()) {
    Common::fs::path path{dir};
    path.make_preferred();
    logger->debug(" - {}", path.string());
    mConfigDirs.emplace_back(path);
  }
}

XdgBaseDir &XdgBaseDir::instance() {
  static XdgBaseDir result;
  return result;
}

Common::fs::path XdgBaseDir::configHome() { return instance().mConfigHome; }

Common::fs::path XdgBaseDir::cacheHome() { return instance().mCacheHome; }

Common::fs::path XdgBaseDir::dataHome() { return instance().mDataHome; }

Common::fs::path XdgBaseDir::stateHome() { return instance().mStateHome; }

std::vector<Common::fs::path> XdgBaseDir::dataDirs() {
  return instance().mDataDirs;
}

std::vector<Common::fs::path> XdgBaseDir::configDirs() {
  return instance().mConfigDirs;
}
