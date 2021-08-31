#ifndef COMMON_STRING_H
#define COMMON_STRING_H

#include <string>
#include <vector>

namespace Common
{
namespace String
{

std::vector<std::string> split(const std::string &data, char delim);

}
}

#endif // COMMON_STRING_H
