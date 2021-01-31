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

Connections::Connections() {}

Connections::~Connections()
{
  for (const auto &c : mConnections)
  {
    delete c;
  }
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
    try { decoded = cppcodec::base32_rfc4648::decode(entry.path().stem().u8string()); }
    catch (cppcodec::parse_error &e)
    {
      wxLogError(
        "Could not load '%s': %s",
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
      continue;
    }

    std::stringstream buffer;
    buffer << brokerOptionsFile.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      continue;
    }

    auto j = nlohmann::json::parse(buffer.str());
    auto brokerOptions = ValueObjects::BrokerOptions::fromJson(j);

    Types::Connection *connection = nullptr;
    connection = new Types::Connection(name, brokerOptions, true);
    mConnections.push_back(connection);
    wxLogMessage("Success: %s", connection->getName());
  }

  return true;
}

bool Connections::updateBrokerOptions(
  wxDataViewItem &item,
  const ValueObjects::BrokerOptions &brokerOptions
) {
  auto connection = reinterpret_cast<Types::Connection*>(item.GetID());
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
  auto connection = reinterpret_cast<Types::Connection*>(item.GetID());
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
  auto connection = new Types::Connection;
  unsigned postfix = 0;

  std::string uniqueName = connection->getName();

  while (std::any_of(
      std::begin(mConnections),
      std::end(mConnections),
      [=](Types::Connection *connection)
      {
        return connection->getName() == uniqueName;
      }
  )) {
    ++postfix;
    uniqueName = fmt::format("{} - {}", connection->getName(), postfix);
  }

  connection->setName(uniqueName);
  mConnections.push_back(connection);

  wxDataViewItem parent(nullptr);
  wxDataViewItem item(reinterpret_cast<void*>(mConnections.back()));
  ItemAdded(parent, item);

  return item;
}

Types::Connection Connections::getConnection(wxDataViewItem &item) const
{
  return *static_cast<Types::Connection*>(item.GetID());
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
  auto c = static_cast<Types::Connection*>(item.GetID());

  switch ((Column)col) {
    case Column::Name: {
      variant = c->getName();
    } break;
    case Column::URL: {
      variant =
        c->getBrokerOptions().getHostname()
        + ":"
        + std::to_string(c->getBrokerOptions().getPort());
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
  return false;
}

unsigned int Connections::GetChildren(
  const wxDataViewItem &parent,
  wxDataViewItemArray &array
) const {

  for (const auto &s : mConnections)
  {
    array.Add(wxDataViewItem(static_cast<void*>(s)));
  }
  return mConnections.size();
}

std::string Connections::toDir(const std::string &name) const
{
  auto encV = cppcodec::base32_rfc4648::encode(name);
  std::string encoded{std::begin(encV), std::end(encV)};
  return fmt::format("{}/{}", mConnectionsDir, encoded);
}
