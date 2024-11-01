#include <fstream>
#include <memory>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "Common/Log.hpp"
#include "MQTT/Message.hpp"
#include "Messages.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI::Models;
using namespace Common;

constexpr std::string_view NewName {"New message"};

Messages::Messages(const ArtProvider &artProvider) :
  FsTree(
    "Messages",
    static_cast<size_t>(Column::Max),
    artProvider
  ),
  mArtProvider(artProvider)
{
  mLogger = Common::Log::create("Models::Messages");
}

bool Messages::load(const std::string &messagesDir)
{
  mMessagesDir = messagesDir + "/snippets";
  return FsTree::load(mMessagesDir);
}

MQTT::Message Messages::getMessage(wxDataViewItem item) const
{
  auto *leaf = getLeaf(item);
  if (leaf == nullptr) { return {}; }

  // NOLINTNEXTLINE
  return *dynamic_cast<Message*>(leaf);
}

std::set<std::string> Messages::getKnownTopics() const
{
  std::set<std::string> result;
  for (const auto &[nodeId, leaf]: getLeafs()) {
    const auto &message = *dynamic_cast<Message*>(leaf);
    result.insert(message.topic);
  }
  return result;
}

wxDataViewItem Messages::createMessage(
  wxDataViewItem parentItem,
  const MQTT::Message &message
) {
  const wxDataViewItem parent(nullptr);
  const std::string uniqueName = createUniqueName(parent, NewName);
  auto leaf = std::make_unique<Message>(message);
  return leafCreate(parentItem, std::move(leaf), uniqueName);
}

wxDataViewItem Messages::replace(
  wxDataViewItem item,
  const MQTT::Message &message
) {
  return FsTree::leafReplace(item, std::make_unique<Message>(message));
}

void Messages::leafValue(Id id, wxDataViewIconText &value, unsigned int col) const
{
  (void)col;

  const auto item = toItem(id);
  auto *leaf = getLeaf(item);
  auto &message = *dynamic_cast<Message*>(leaf);

  wxIcon icon;
  if (message.payload.empty())
  {
    icon.CopyFromBitmap(mArtProvider.bitmap(Icon::File));
  }
  else
  {
    icon.CopyFromBitmap(mArtProvider.bitmap(Icon::FileFull));
  }
  value.SetIcon(icon);
}

std::unique_ptr<FsTree::Leaf> Messages::leafLoad(Id id, const Common::fs::path &path)
{
  (void)id;

  std::ifstream messageFile(path);
  if (!messageFile.is_open())
  {
    return nullptr;
  }

  std::stringstream buffer;
  buffer << messageFile.rdbuf();
  const std::string &sbuffer = buffer.str();

  MQTT::Message message;

  if (!sbuffer.empty())
  {
    if (!nlohmann::json::accept(sbuffer))
    {
      return nullptr;
    }

    auto data = nlohmann::json::parse(sbuffer);
    message = MQTT::Message::fromJson(data);
  }

  auto leaf = std::make_unique<Message>(message);

  leaf->payload   = message.payload;
  leaf->qos       = message.qos;
  leaf->retained  = message.retained;
  leaf->timestamp = message.timestamp;
  leaf->topic     = message.topic;

  return std::move(leaf);
}

bool Messages::leafSave(Id id)
{
  const auto path = getNodePath(id);
  std::ofstream output(path);
  if (!output.is_open())
  {
    return false;
  }

  const auto item = toItem(id);
  auto *leaf = getLeaf(item);
  auto &message = *dynamic_cast<Message*>(leaf);

  output << MQTT::Message::toJson(message);

  return true;
}

bool Messages::isLeaf(const Common::fs::directory_entry &entry) const
{
  return entry.status().type() != fs::file_type::directory;
}

Messages::Message::Message(
  const MQTT::Message &msg
) :
  MQTT::Message(msg)
{}

