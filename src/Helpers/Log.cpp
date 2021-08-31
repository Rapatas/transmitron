#include "Log.hpp"
#include <spdlog/spdlog.h>

using namespace Helpers;

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
  return true;
}

std::shared_ptr<spdlog::logger> Log::create(const std::string &name)
{
  return instance().createPrivate(name);
}

std::shared_ptr<spdlog::logger> Log::createPrivate(const std::string &name)
{
  return std::make_shared<spdlog::logger>(name, mSink);
}
