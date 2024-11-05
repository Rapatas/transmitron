#ifdef _WIN32

#include "Common/Console.hpp"

#include <fcntl.h>
#include <fstream>
#include <io.h>
#include <iostream>
#include <stdio.h>
#include <windows.h>

using namespace Rapatas::Transmitron::Common;

bool Console::release() {
  auto redirectToNul = [](FILE *stream) {
    FILE *fp = nullptr;
    if (freopen_s(&fp, "NUL:", "r", stream) != 0) { return false; }
    setvbuf(stream, NULL, _IONBF, 0);
    return true;
  };

  bool hadFailure = false;
  // Just to be safe, redirect standard IO to NUL before releasing.
  hadFailure |= !redirectToNul(stdin);
  hadFailure |= !redirectToNul(stdout);
  hadFailure |= !redirectToNul(stderr);
  hadFailure |= !FreeConsole();
  return !hadFailure;
}

bool Console::redirect() {
  auto redirect = [](DWORD handle, FILE *stream, std::string name) {
    if (GetStdHandle(handle) == INVALID_HANDLE_VALUE) { return false; }
    FILE *fp = nullptr;
    if (freopen_s(&fp, name.data(), "r", stream) != 0) { return false; }
    setvbuf(stream, NULL, _IONBF, 0);
    return true;
  };

  bool hadFailure = false;
  hadFailure |= redirect(STD_INPUT_HANDLE, stdin, "CONIN$");
  hadFailure |= redirect(STD_OUTPUT_HANDLE, stdout, "CONOUT$");
  hadFailure |= redirect(STD_ERROR_HANDLE, stderr, "CONOUT$");

  // Make C++ standard streams point to console as well.
  std::ios::sync_with_stdio(true);

  // Clear the error state for each of the C++ standard streams.
  std::wcout.clear();
  std::cout.clear();
  std::wcerr.clear();
  std::cerr.clear();
  std::wcin.clear();
  std::cin.clear();

  return !hadFailure;
}

void Console::adjustBuffer(int16_t minLength) {
  // Set the screen buffer to be big enough to scroll some text
  CONSOLE_SCREEN_BUFFER_INFO conInfo;
  GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
  if (conInfo.dwSize.Y < minLength) { conInfo.dwSize.Y = minLength; }
  SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
}

bool Console::create(int16_t minLength) {
  // Release any current console and redirect IO to NUL
  (void)release();

  // Attempt to create new console
  if (AllocConsole()) {
    adjustBuffer(minLength);
    return redirect();
  }
  return false;
}

bool Console::attachToParent(int16_t minLength) {
  // Release any current console and redirect IO to NUL
  (void)release();

  // Attempt to attach to parent process's console
  if (AttachConsole(ATTACH_PARENT_PROCESS)) {
    adjustBuffer(minLength);
    return redirect();
  }

  return false;
}

#endif // _WIN32
