#include "Log.hpp"
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifndef NDEBUG
  const bool DEBUG = true;
#else
  const bool DEBUG = false;
#endif

using namespace Common;

using Level = spdlog::level::level_enum;

Log &Log::instance()
{
  static Log log;
  return log;
}

Log::~Log() = default;

void Log::initialize(bool verbose)
{
  const Level level = !verbose
    ? Level::off
    : DEBUG
      ? Level::trace
      : Level::info;

  mSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  mSink->set_level(level);
  const std::string pattern {"[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%n] %v"};
  spdlog::set_pattern(pattern);
}

std::shared_ptr<spdlog::logger> Log::create(const std::string &name)
{
  return instance().createPrivate(name);
}

std::shared_ptr<spdlog::logger> Log::createPrivate(const std::string &name)
{
  auto logger = spdlog::get(name);
  if (!logger)
  {
    auto logger = std::make_shared<spdlog::logger>(name, mSink);
    spdlog::initialize_logger(logger);
    spdlog::cfg::load_env_levels();
  }
  return logger;
}
