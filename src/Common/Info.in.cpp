#include "Common/Info.hpp"

#ifndef PROJECT_NAME
#define PROJECT_NAME "@PROJECT_NAME@"
#endif // PROJECT_NAME

const char *Common::getProjectName()
{
    return PROJECT_NAME;
}

#ifndef PROJECT_DESCRIPTION
#define PROJECT_DESCRIPTION "@PROJECT_DESCRIPTION@"
#endif // PROJECT_DESCRIPTION

const char *Common::getProjectDescription()
{
    return PROJECT_DESCRIPTION;
}