#include "Common/Info.hpp"

constexpr const char *ProjectName = "@PROJECT_NAME@";
constexpr const char *ProjectDescription = "@PROJECT_DESCRIPTION@";

using namespace Rapatas::Transmitron;

const char *Common::Info::getProjectName() { return ProjectName; }

const char *Common::Info::getProjectDescription() { return ProjectDescription; }
