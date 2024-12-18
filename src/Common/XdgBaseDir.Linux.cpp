#ifndef _WIN32

#include "XdgBaseDir.hpp"

#include <pwd.h>
#include <unistd.h>

#include <fmt/core.h>
#include <sys/types.h>

#include "Env.hpp"
#include "String.hpp"

using namespace Rapatas::Transmitron::Common;

std::string XdgBaseDir::readHome() {
  auto home = Env::get("HOME");
  if (!home.empty()) { return home; }

  const auto initlen = sysconf(_SC_GETPW_R_SIZE_MAX);
  constexpr size_t DefaultLen = 1024;
  const size_t len = (initlen == -1) //
    ? DefaultLen
    : static_cast<size_t>(initlen);

  struct passwd pwd{};
  struct passwd *resultp = nullptr;
  std::string buffer;
  buffer.resize(len);
  getpwuid_r(getuid(), &pwd, buffer.data(), buffer.size(), &resultp);
  return pwd.pw_dir;
}

std::string XdgBaseDir::readConfigHome() {
  auto var = Env::get("XDG_CONFIG_HOME");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.config", mHome.string());
}

std::string XdgBaseDir::readDataHome() {
  auto var = Env::get("XDG_DATA_HOME");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.local/share", mHome.string());
}

std::string XdgBaseDir::readCacheHome() {
  auto var = Env::get("XDG_CACHE_HOME");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.cache", mHome.string());
}

std::string XdgBaseDir::readStateHome() {
  auto var = Env::get("XDG_STATE_HOME");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/.local/state", mHome.string());
}

// NOLINTNEXTLINE
std::vector<std::string> XdgBaseDir::readDataDirs() const {
  auto var = Env::get("XDG_DATA_DIRS");
  if (var.empty()) { var = "/usr/local/share:/usr/share"; }
  auto dirs = String::split(var, ':');
  for (auto &dir : dirs) {
    if (dir.back() == '/') { dir.pop_back(); }
  }
  return dirs;
}

// NOLINTNEXTLINE
std::vector<std::string> XdgBaseDir::readConfigDirs() const {
  auto var = Env::get("XDG_CONFIG_DIRS");
  if (var.empty()) { var = "/etc/xdg"; }
  auto dirs = String::split(var, ':');
  for (auto &dir : dirs) {
    if (dir.back() == '/') { dir.pop_back(); }
  }
  return dirs;
}

#endif // _WIN32
