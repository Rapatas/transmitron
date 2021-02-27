#include <fstream>
#include <wx/log.h>
#include <fmt/core.h>
#include <cppcodec/base32_rfc4648.hpp>
#include "Transmitron/Info.hpp"
#include "Connections.hpp"

#define wxLOG_COMPONENT "models/connections"

namespace fs = std::filesystem;
using namespace Transmitron::Models;
using namespace Transmitron;

Connections::Connections()
{
  mConnections.push_back(nullptr);
}

bool Connections::load(const std::string &configDir)
{
  if (configDir.empty())
  {
    wxLogWarning("No directory provided");
    return false;
  }

  mConnectionsDir = configDir + "/connections";

  bool exists = fs::exists(mConnectionsDir);
  bool isDir = fs::is_directory(mConnectionsDir);

  if (exists && !isDir && !fs::remove(mConnectionsDir))
  {
    wxLogWarning("Could not remove file %s", mConnectionsDir);
    return false;
  }

  if (!exists && !fs::create_directory(mConnectionsDir))
  {
    wxLogWarning(
      "Could not create connections directory: %s",
      mConnectionsDir
    );
    return false;
  }

  for (const auto &entry : fs::directory_iterator(mConnectionsDir))
  {
    wxLogMessage("Checking %s", entry.path().u8string());
    if (entry.status().type() != fs::file_type::directory) { continue; }

    // Get name.
    std::vector<uint8_t> decoded;
    try
    {
      decoded = cppcodec::base32_rfc4648::decode(
        entry.path().stem().u8string()
      );
    }
    catch (cppcodec::parse_error &e)
    {
      wxLogError(
        "Could not decode '%s': %s",
        entry.path().u8string(),
        e.what()
      );
      continue;
    }
    std::string name{decoded.begin(), decoded.end()};

    // Get BrokerOptions.
    auto brokerOptionsFilepath = fmt::format(
      "{}/{}",
      entry.path().c_str(),
      BrokerOptionsFilename
    );

    std::ifstream brokerOptionsFile(brokerOptionsFilepath);
    if (!brokerOptionsFile.is_open())
    {
      wxLogWarning("Could not open '%s'", entry.path().u8string());
      continue;
    }

    std::stringstream buffer;
    buffer << brokerOptionsFile.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      wxLogWarning("Could not parse '%s'", entry.path().u8string());
      continue;
    }

    auto j = nlohmann::json::parse(buffer.str());
    auto brokerOptions = ValueObjects::BrokerOptions::fromJson(j);

    mConnections.push_back(
      std::make_shared<Types::Connection>(name, brokerOptions, true, entry.path())
    );
  }

  return true;
}

bool Connections::updateBrokerOptions(
  wxDataViewItem &item,
  const ValueObjects::BrokerOptions &brokerOptions
) {
  const auto &connection =  mConnections.at(toIndex(item));
  connection->setBrokerOptions(brokerOptions);

  std::string dir = toDir(connection->getName());

  if (
    !connection->getSaved()
    && !fs::create_directory(dir)
  ) {
      wxLogError("Could not create connection directory");
      return false;
  }

  auto brokerOptionsFilepath = fmt::format(
    "{}/{}",
    dir,
    BrokerOptionsFilename
  );

  std::ofstream brokerOptionsFile(brokerOptionsFilepath);
  if (!brokerOptionsFile.is_open())
  {
    wxLogError("Could not save broker options");
    return false;
  }

  brokerOptionsFile << brokerOptions.toJson().dump(2);
  brokerOptionsFile.close();

  connection->setBrokerOptions(brokerOptions);
  connection->setSaved(true);
  ItemChanged(item);

  return true;
}

bool Connections::updateName(
  wxDataViewItem &item,
  const std::string &name
) {
  const auto &connection =  mConnections.at(toIndex(item));
  if (connection->getName() == name)
  {
    return true;
  }

  std::string before = toDir(connection->getName());
  std::string after = toDir(name);

  fs::rename(before, after);
  if (errno != 0)
  {
    wxLogError("Could not rename connection to '%s'", name);
    return false;
  }

  return true;
}

wxDataViewItem Connections::createConnection()
{
  const char *newConnectionName = "New Connection";
  std::string uniqueName{newConnectionName};
  unsigned postfix = 0;
  while (std::any_of(
      std::begin(mConnections) + 1,
      std::end(mConnections),
      [=](auto connection)
      {
        return connection->getName() == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", newConnectionName, postfix);
  }

  auto connection = std::make_shared<Types::Connection>(uniqueName);
  mConnections.push_back(connection);

  wxDataViewItem parent(nullptr);
  auto item = toItem(mConnections.size() - 1);
  ItemAdded(parent, item);

  return item;
}

std::shared_ptr<Types::Connection> Connections::getConnection(const wxDataViewItem &item) const
{
  return mConnections.at(toIndex(item));
}

unsigned Connections::GetColumnCount() const
{
  return (unsigned)Column::Max;
}

wxString Connections::GetColumnType(unsigned int col) const
{
  switch ((Column)col)
  {
    case Column::Name: { return wxDataViewTextRenderer::GetDefaultType(); } break;
    case Column::URL:  { return wxDataViewTextRenderer::GetDefaultType(); } break;
    default: { return "string"; }
  }
}

void Connections::GetValue(
  wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) const {
  const auto &connection = mConnections.at(toIndex(item));

  switch ((Column)col) {
    case Column::Name: {
      variant = connection->getName();
    } break;
    case Column::URL: {
      variant =
        connection->getBrokerOptions().getHostname()
        + ":"
        + std::to_string(connection->getBrokerOptions().getPort());
    } break;
    default: {}
  }
}

bool Connections::SetValue(
  const wxVariant &variant,
  const wxDataViewItem &item,
  unsigned int col
) {
  return false;
}

bool Connections::IsEnabled(
  const wxDataViewItem &item,
  unsigned int col
) const {
  return true;
}

wxDataViewItem Connections::GetParent(
  const wxDataViewItem &item
) const {
  return wxDataViewItem(nullptr);
}

bool Connections::IsContainer(
  const wxDataViewItem &item
) const {
  if (!item.IsOk())
  {
    return true;
  }
  return false;
}

unsigned int Connections::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {
  for (size_t i = 1; i != mConnections.size(); ++i)
  {
    array.Add(toItem(i));
  }
  return mConnections.size() - 1;
}

std::string Connections::toDir(const std::string &name) const
{
  auto encV = cppcodec::base32_rfc4648::encode(name);
  std::string encoded{std::begin(encV), std::end(encV)};
  return fmt::format("{}/{}", mConnectionsDir, encoded);
}

size_t Connections::toIndex(const wxDataViewItem &item)
{
  return reinterpret_cast<size_t>(item.GetID());
}

wxDataViewItem Connections::toItem(size_t index)
{
  return wxDataViewItem(reinterpret_cast<void*>(index));
}

