#include "Log.hpp"
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef _WIN32
#include <spdlog/sinks/win_eventlog_sink.h>
#include "Common/Console.hpp"
#endif // _WIN32

#ifndef NDEBUG
  const bool DEBUG = true;
#else
  const bool DEBUG = false;
#endif

using namespace Rapatas::Transmitron::Common;

using Level = spdlog::level::level_enum;

Log &Log::instance()
{
  static Log log;
  return log;
}

Log::~Log()
{
#ifdef _WIN32
  Console::release();
#else
  (void)this;
#endif // _WIN32
}

void Log::initialize(bool verbose)
{

#ifdef _WIN32
  if (verbose)
  {
    Console::attachToParent(1024);
  }
#endif // _WIN32

  const Level level = !verbose
    ? Level::off
    : DEBUG
      ? Level::trace
      : Level::info;
  (void)level;

  auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  sink->set_level(Level::trace);
  mSinks.push_back(sink);

#ifdef _WIN32
  auto event = std::make_shared<spdlog::sinks::win_eventlog_sink_mt>("Transmitron");
  event->set_level(Level::warn);
  mSinks.push_back(event);
#endif // _WIN32

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
    logger = std::make_shared<spdlog::logger>(name, mSinks.begin(), mSinks.end());
    spdlog::initialize_logger(logger);
    spdlog::cfg::load_env_levels();
  }
  return logger;
}
