#include "Common/Version.hpp"

constexpr const char *ProjectVersion = "@GIT_DESCRIBE@";

const char *Rapatas::Transmitron::Common::Info::getProjectVersion() {
  return ProjectVersion;
}
