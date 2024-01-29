#pragma once

namespace Rapatas::Transmitron::Common::Console
{

#ifdef _WIN32

bool redirect();
bool release();
void adjustBuffer(int16_t minLength);
bool create(int16_t minLength);
bool attachToParent(int16_t minLength);

#endif // _WIN32

} // namespace Rapatas::Transmitron::Common::Console
