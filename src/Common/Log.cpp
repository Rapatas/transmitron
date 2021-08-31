#include "Log.hpp"
#include <spdlog/spdlog.h>

using namespace Common;

Log &Log::instance()
{
  static Log log;
  return log;
}

Log::~Log()
{
  spdlog::drop_all();
  spdlog::shutdown();
}

bool Log::initialize()
{
  mSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  mSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] %^[%l]%$ [%n] %v");
#ifndef NDEBUG
  mSink->set_level(spdlog::level::level_enum::trace);
#else
  mSink->set_level(spdlog::level::level_enum::critical);
#endif
  return true;
}

std::shared_ptr<spdlog::logger> Log::create(const std::string &name)
{
  return instance().createPrivate(name);
}

std::shared_ptr<spdlog::logger> Log::createPrivate(const std::string &name)
{
  auto logger = std::make_shared<spdlog::logger>(name, mSink);
  logger->set_level(spdlog::level::level_enum::trace);
  return logger;
}
