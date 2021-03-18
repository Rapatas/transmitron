#include "Connection.hpp"
#include <fstream>
#include <fmt/format.h>
#include <cppcodec/base32_rfc4648.hpp>

using namespace Transmitron::Types;
using namespace Transmitron;

Connection::Connection(
  std::string name,
  ValueObjects::BrokerOptions brokerOptions,
  bool saved,
  std::filesystem::path path
) :
  mSaved(saved),
  mName(std::move(name)),
  mBrokerOptions(std::move(brokerOptions)),
  mPath(std::move(path))
{
  mSnippetsModel = new Models::Snippets;
  mSnippetsModel->load(mPath);
}

const ValueObjects::BrokerOptions &Connection::getBrokerOptions() const
{
  return mBrokerOptions;
}

bool Connection::getSaved() const
{
  return mSaved;
}

std::string Connection::getName() const
{
  return mName;
}

const wxObjectDataPtr<Models::Snippets> Connection::getSnippetsModel()
{
  return mSnippetsModel;
}

std::filesystem::path Connection::getPath() const
{
  return mPath;
}

void Connection::setBrokerOptions(ValueObjects::BrokerOptions brokerOptions)
{
  mBrokerOptions = std::move(brokerOptions);
}

void Connection::setSaved(bool saved)
{
  mSaved = saved;
}

void Connection::setName(std::string name)
{
  mName = std::move(name);
}

