#ifndef _WIN32

#include <pwd.h>
#include <unistd.h>
#include <sys/types.h>

#include <fmt/core.h>

#include "XdgBaseDir.hpp"
#include "String.hpp"
#include "Env.hpp"

using namespace Rapatas::Transmitron::Common;

std::string XdgBaseDir::readHome()
{
  auto home = Env::get("HOME");
  if (!home.empty())
  {
    return home;
  }
  struct passwd *pwd = getpwuid(getuid());
  return pwd->pw_dir;
}

std::string XdgBaseDir::readConfigHome()
{
  auto var = Env::get("XDG_CONFIG_HOME");
  if (!var.empty() && var.back() == '/')
  {
    var.pop_back();
  }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.config", mHome.string());
}

std::string XdgBaseDir::readDataHome()
{
  auto var = Env::get("XDG_DATA_HOME");
  if (!var.empty() && var.back() == '/')
  {
    var.pop_back();
  }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.local/share", mHome.string());
}

std::string XdgBaseDir::readCacheHome()
{
  auto var = Env::get("XDG_CACHE_HOME");
  if (!var.empty() && var.back() == '/')
  {
    var.pop_back();
  }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.cache", mHome.string());
}

std::string XdgBaseDir::readStateHome()
{
  auto var = Env::get("XDG_STATE_HOME");
  if (!var.empty() && var.back() == '/')
  {
    var.pop_back();
  }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.local/state", mHome.string());
}

// NOLINTNEXTLINE
std::vector<std::string> XdgBaseDir::readDataDirs() const
{
  auto var = Env::get("XDG_DATA_DIRS");
  if (var.empty())
  {
    var = "/usr/local/share:/usr/share";
  }
  auto dirs = String::split(var, ':');
  for (auto &dir : dirs)
  {
    if (dir.back() == '/')
    {
      dir.pop_back();
    }
  }
  return dirs;
}

// NOLINTNEXTLINE
std::vector<std::string> XdgBaseDir::readConfigDirs() const
{
  auto var = Env::get("XDG_CONFIG_DIRS");
  if (var.empty())
  {
    var = "/etc/xdg";
  }
  auto dirs = String::split(var, ':');
  for (auto &dir : dirs)
  {
    if (dir.back() == '/')
    {
      dir.pop_back();
    }
  }
  return dirs;
}

#endif // _WIN32
