#include "Transmitron/Version.hpp"

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "@PROJECT_VERSION@"
#endif // PROJECT_VERSION

const char *getProjectVersion()
{
    return PROJECT_VERSION;
}
