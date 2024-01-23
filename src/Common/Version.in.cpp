#include "Common/Version.hpp"

constexpr const char *ProjectVersion = "@PROJECT_VERSION@";

const char *Rapatas::Transmitron::Common::Info::getProjectVersion()
{
  return ProjectVersion;
}
