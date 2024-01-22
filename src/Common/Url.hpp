#pragma once

#include <string>

namespace Rapatas::Transmitron::Common::Url
{

std::string encode(const std::string &data);
std::string decode(const std::string &data);

inline bool encodable(char value);
inline bool isHexChar(char value);

} // namespace Rapatas::Transmitron::Common::Url
