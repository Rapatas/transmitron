#ifndef COMMON_URL_HPP
#define COMMON_URL_HPP

#include <string>

namespace Common
{
namespace Url
{

std::string encode(const std::string &data);
std::string decode(const std::string &data);

inline bool encodable(char c);
inline bool isHexChar(char c);

}
}

#endif // COMMON_URL_HPP
