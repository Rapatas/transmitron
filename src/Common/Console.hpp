#ifndef COMMON_CONSOLE_HPP
#define COMMON_CONSOLE_HPP

namespace Common::Console
{

#ifdef _WIN32

bool redirect();
bool release();
void adjustBuffer(int16_t minLength);
bool create(int16_t minLength);
bool attachToParent(int16_t minLength);

#endif // _WIN32

} // namespace Common::Console

#endif // COMMON_CONSOLE_HPP
