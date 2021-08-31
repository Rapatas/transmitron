#include <filesystem>
#include <fmt/core.h>
#include <streambuf>

#include "XdgBaseDir.hpp"
#include "Log.hpp"

using namespace Common;

XdgBaseDir::XdgBaseDir()
{
  auto logger = Log::create("Common::XDG");

  mHome       = readHome();
  mConfigHome = readConfigHome();
  mCacheHome  = readCacheHome();
  mDataHome   = readDataHome();
  mStateHome  = readStateHome();

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
  for (const auto &dir : readDataDirs())
  {
    std::filesystem::path p{dir};
    p.make_preferred();

    logger->debug(" - {}", p.string());
    mDataDirs.emplace_back(p);
  }

  logger->debug("XDG_CONFIG_DIRS:");
  for (const auto &dir : readConfigDirs())
  {
    std::filesystem::path p{dir};
    p.make_preferred();
    logger->debug(" - {}", p.string());
    mConfigDirs.emplace_back(p);
  }
}

XdgBaseDir &XdgBaseDir::instance()
{
  static XdgBaseDir result;
  return result;
}

std::filesystem::path XdgBaseDir::configHome()
{
  return instance().mConfigHome;
}

std::filesystem::path XdgBaseDir::cacheHome()
{
  return instance().mCacheHome;
}

std::filesystem::path XdgBaseDir::dataHome()
{
  return instance().mDataHome;
}

std::filesystem::path XdgBaseDir::stateHome()
{
  return instance().mStateHome;
}

std::vector<std::filesystem::path> XdgBaseDir::dataDirs()
{
  return instance().mDataDirs;
}

std::vector<std::filesystem::path> XdgBaseDir::configDirs()
{
  return instance().mConfigDirs;
}
