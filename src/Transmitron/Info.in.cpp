#include "Transmitron/Info.hpp"

#ifndef PROJECT_NAME
#define PROJECT_NAME "@PROJECT_NAME@"
#endif // PROJECT_NAME

const char *getProjectName()
{
    return PROJECT_NAME;
}

#ifndef PROJECT_DESCRIPTION
#define PROJECT_DESCRIPTION "@PROJECT_DESCRIPTION@"
#endif // PROJECT_DESCRIPTION

const char *getProjectDescription()
{
    return PROJECT_DESCRIPTION;
}

#ifndef PROJECT_VERSION
#define PROJECT_VERSION "@PROJECT_VERSION@"
#endif // PROJECT_VERSION

const char *getProjectVersion()
{
    return PROJECT_VERSION;
}
