#ifdef _WIN32

#include "XdgBaseDir.hpp"

#include <fmt/core.h>

#include "Env.hpp"
#include "String.hpp"

using namespace Rapatas::Transmitron::Common;

std::string XdgBaseDir::readHome() { return Env::get("USERPROFILE"); }

std::string XdgBaseDir::readConfigHome() { return readDataHome(); }

std::string XdgBaseDir::readDataHome() {
  auto var = Env::get("LOCALAPPDATA");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/AppData/Local", mHome.string());
}

std::string XdgBaseDir::readCacheHome() {
  auto var = Env::get("TEMP");
  if (!var.empty() && var.back() == '/') { var.pop_back(); }
  if (!var.empty()) { return var; }
  if (mHome.empty()) { return {}; }
  return fmt::format("{}/AppData/Local/Temp", mHome.string());
}

std::string XdgBaseDir::readStateHome() { return readCacheHome(); }

std::vector<std::string> XdgBaseDir::readDataDirs() const {
  auto var = Env::get("APPDATA");
  if (var.empty()) { var = fmt::format("{}/AppData/Roaming", mHome.string()); }
  auto dirs = String::split(var, ';');
  for (auto &dir : dirs) {
    if (dir.back() == '/') { dir.pop_back(); }
  }
  return dirs;
}

std::vector<std::string> XdgBaseDir::readConfigDirs() const {
  return readDataDirs();
}

#endif // _WIN32
