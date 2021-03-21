#include "Transmitron/App.hpp"

int main(int argc, char **argv)
{
  auto *transmitron = new Transmitron::App;
  wxApp::SetInstance(transmitron);
  wxEntry(argc, argv);
  return 0;
}
