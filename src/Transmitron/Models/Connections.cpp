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

    fs::path connectionPath = entry.path().u8string() + "/connection.json";

    std::ifstream file(connectionPath);
    if (!file.is_open())
    {
      wxLogWarning("Could not open '%s'", connectionPath.u8string());
      continue;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      wxLogWarning("Could not load '%s' Malformed json", entry.path().u8string());
      continue;
    }

    auto stem = entry.path().stem().u8string();
    std::vector<uint8_t> decoded;

    try { decoded = cppcodec::base32_rfc4648::decode(stem); }
    catch (cppcodec::parse_error &e) {
      wxLogError(
        "Could not load '%s' Malformed base32 encoding.",
        entry.path().u8string()
      );
      continue;
    }

    auto j = nlohmann::json::parse(buffer.str());
    std::string name{decoded.begin(), decoded.end()};
    mConnections.push_back(
      new ConnectionInfo{
        Types::Connection(j, name),
        true
      }
    );
  }

  return true;
}

bool Connections::updateConnection(
  wxDataViewItem &item,
  const Types::Connection &data
) {
  auto info = reinterpret_cast<ConnectionInfo*>(item.GetID());

  std::string dir = toDir(data.getName());

  if (info->saved)
  {
    if (info->connection.getName() != data.getName())
    {
      std::error_code ec;
      fs::rename(
        toDir(info->connection.getName()),
        dir,
        ec
      );
      if (ec)
      {
        wxLogError(
          "Could not rename '%s' to '%s'",
          info->connection.getName(),
          data.getName()
        );
        return false;
      }
    }
  }
  else
  {
    if (!fs::create_directory(dir))
    {
      wxLogError("Could not create connection dir");
      return false;
    }
  }

  std::ofstream file(dir + "/connection.json");
  if (!file.is_open())
  {
    wxLogError("Could not save connection");
    return false;
  }

  file << data.toJson().dump(2);
  file.close();

  info->connection = data;
  info->saved = true;
  ItemChanged(item);

  return true;
}

wxDataViewItem Connections::createConnection(const Types::Connection &data)
{
  if (data.getName().empty())
  {
    wxLogWarning("Refusing to create unnamed connection");
    return {};
  }

  std::string name = data.getName();
  unsigned postfix = 0;

  while (std::any_of(
      std::begin(mConnections),
      std::end(mConnections),
      [=](ConnectionInfo *connectionInfo)
      {
        return connectionInfo->connection.getName() == name;
      }
  )) {
    ++postfix;
    name = fmt::format("{} - {}", data.getName(), postfix);
  }

  mConnections.push_back(new ConnectionInfo{data, false});

  if (postfix)
  {
    mConnections.back()->connection.setName(name);
  }

  wxDataViewItem parent(nullptr);
  wxDataViewItem item(reinterpret_cast<void*>(mConnections.back()));
  ItemAdded(parent, item);

  return item;
}

Types::Connection Connections::getConnection(wxDataViewItem &item) const
{
  return static_cast<ConnectionInfo*>(item.GetID())->connection;
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
  auto c = static_cast<ConnectionInfo*>(item.GetID());

  switch ((Column)col) {
    case Column::Name: {
      variant = c->connection.getName();
    } break;
    case Column::URL: {
      variant =
        c->connection.getHostname()
        + ":"
        + std::to_string(c->connection.getPort());
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
