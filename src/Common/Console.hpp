#pragma once

#ifdef _WIN32
#include <cstdint>

namespace Rapatas::Transmitron::Common::Console {

bool redirect();
bool release();
void adjustBuffer(int16_t minLength);
bool create(int16_t minLength);
bool attachToParent(int16_t minLength);

} // namespace Rapatas::Transmitron::Common::Console

#endif // _WIN32
