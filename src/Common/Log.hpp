#pragma once

#include <memory>
#include <string>

#include <spdlog/logger.h>

namespace Rapatas::Transmitron::Common {

class Log
{
public:

  virtual ~Log();
  Log(const Log &other) = delete;
  Log(Log &&other) = delete;
  Log &operator=(const Log &other) = delete;
  Log &operator=(Log &&other) = delete;

  void initialize(bool verbose);

  static Log &instance();
  static std::shared_ptr<spdlog::logger> create(const std::string &name);

private:

  Log() = default;

  std::vector<spdlog::sink_ptr> mSinks;

  std::shared_ptr<spdlog::logger> createPrivate(const std::string &name);
};

} // namespace Rapatas::Transmitron::Common
