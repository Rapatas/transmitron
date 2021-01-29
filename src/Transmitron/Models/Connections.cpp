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
  auto config = getConfigPath();

  if (!fs::is_directory(config) || !fs::exists(config))
  {
    if (!fs::create_directory(config))
    {
      wxLogWarning("Could not create config directory: %s", config.c_str());
      return;
    }
  }

  for (const auto &entry : fs::directory_iterator(config))
  {
    if (entry.path().extension() != ".json") { continue; }

    wxLogMessage("Loading %s", entry.path().c_str());

    std::ifstream file(entry.path());
    std::stringstream buffer;
    buffer << file.rdbuf();
    if (!nlohmann::json::accept(buffer.str()))
    {
      wxLogWarning(
        "Could not load %s: Malformed json.",
        entry.path().u8string()
      );
      continue;
    }

    try {
      auto stem = entry.path().stem().u8string();
      auto nameV = cppcodec::base32_rfc4648::decode(stem);
      std::string name{nameV.begin(), nameV.end()};

      auto j = nlohmann::json::parse(buffer.str());
      mConnections.push_back(new ConnectionInfo{Types::Connection(j, name), true});

    } catch (cppcodec::parse_error &e) {
      wxLogError(
        "Could not load %s: Malformed base32 encoding.",
        entry.path().u8string()
      );
    }
  }
}

Connections::~Connections()
{
  for (const auto &c : mConnections)
  {
    delete c;
  }
}

void Connections::updateConnection(wxDataViewItem &item, const Types::Connection &data)
{
  auto c = reinterpret_cast<Types::Connection*>(item.GetID());

  if (c->getName() != data.getName())
  {
    fs::remove(toFileName(c->getName()));
  }

  std::ofstream file(toFileName(data.getName()));
  if (!file.is_open())
  {
    wxLogError("Could not save connection");
    return;
  }

  file << data.toJson().dump(2);
  file.close();

  *c = data;
  ItemChanged(item);
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

std::filesystem::path Connections::getConfigPath()
{
  char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home == nullptr)
  {
    char *user = getenv("USER");
    if (user == nullptr)
    {
      throw std::runtime_error("Environment Variable $USER not set");
    }
    return fmt::format("/home/{}/.config/{}", user, getProjectName());
  }
  else
  {
    std::string xdgConfigHome(xdg_config_home);

    if (xdgConfigHome.back() != '/')
    {
      xdgConfigHome += '/';
    }

    return xdgConfigHome + getProjectName();
  }
}

std::filesystem::path Connections::toFileName(const std::string &name)
{
  auto encV = cppcodec::base32_rfc4648::encode(name);
  std::string encoded{std::begin(encV), std::end(encV)};
  return fmt::format("{}/{}.json", getConfigPath().u8string(), encoded);
}
