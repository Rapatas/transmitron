#include "Common/Version.hpp"

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "@PROJECT_VERSION@"
#endif // PROJECT_VERSION

const char *Rapatas::Transmitron::Common::Info::getProjectVersion()
{
    return PROJECT_VERSION;
}
