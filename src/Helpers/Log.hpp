#ifndef HELPERS_LOG_H
#define HELPERS_LOG_H

#include <filesystem>
#include <spdlog/common.h>
#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Helpers
{

class Log
{
public:

  virtual ~Log();
  Log(const Log &other) = delete;
  Log(Log &&other) = delete;
  Log &operator=(const Log &other) = delete;
  Log &operator=(Log &&other) = delete;

  bool initialize();

  static Log &instance();
  static std::shared_ptr<spdlog::logger> create(const std::string &name);

private:

  Log() = default;

  spdlog::sink_ptr mSink;

  std::shared_ptr<spdlog::logger> createPrivate(const std::string &name);

};

}

#endif // HELPERS_LOG_H
