#pragma once

#include <string>

namespace Common::Url
{

std::string encode(const std::string &data);
std::string decode(const std::string &data);

inline bool encodable(char c);
inline bool isHexChar(char c);

} // namespace Common::Url
