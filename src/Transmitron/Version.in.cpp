#include "Transmitron/Version.hpp"

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "@GIT_DESCRIBE@"
#endif // PROJECT_VERSION

const char *getProjectVersion()
{
    return PROJECT_VERSION;
}
