#ifndef HELPERS_URL_HPP
#define HELPERS_URL_HPP

#include <string>

namespace Helpers
{
namespace Url
{

std::string encode(const std::string &data);
std::string decode(const std::string &data);

inline bool encodable(char c);
inline bool isHexChar(char c);

}
}

#endif // HELPERS_URL_HPP
