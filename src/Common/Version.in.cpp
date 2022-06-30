#include "Common/Version.hpp"

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "@PROJECT_VERSION@"
#endif // PROJECT_VERSION

const char *Common::getProjectVersion()
{
    return PROJECT_VERSION;
}
