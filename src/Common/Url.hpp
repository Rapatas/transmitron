#pragma once

#include <string>

namespace Rapatas::Transmitron::Common::Url
{

std::string encode(const std::string &data);
std::string decode(const std::string &data);

inline bool encodable(char c);
inline bool isHexChar(char c);

} // namespace Rapatas::Transmitron::Common::Url
