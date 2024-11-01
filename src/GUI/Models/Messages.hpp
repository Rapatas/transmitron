#pragma once

#include "Common/Filesystem.hpp"
#include <set>
#include <memory>

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "MQTT/Message.hpp"
#include "GUI/ArtProvider.hpp"
#include "GUI/Models/FsTree.hpp"

namespace Rapatas::Transmitron::GUI::Models
{

class Messages :
  public FsTree
{
public:

  enum Column : uint8_t
  {
    Name,
    Max
  };

  explicit Messages(const ArtProvider &artProvider);

  wxDataViewItem createMessage(
    wxDataViewItem parent,
    const MQTT::Message &message
  );
  wxDataViewItem replace(
    wxDataViewItem item,
    const MQTT::Message &message
  );

  bool load(const std::string &messagesDir);
  [[nodiscard]] MQTT::Message getMessage(wxDataViewItem item) const;
  [[nodiscard]] std::set<std::string> getKnownTopics() const;

private:

  struct Message :
    public FsTree::Leaf,
    public MQTT::Message
  {
    explicit Message() = default;
    explicit Message(const MQTT::Message &msg);
  };

  std::shared_ptr<spdlog::logger> mLogger;
  std::string mMessagesDir;
  const ArtProvider &mArtProvider;

  [[nodiscard]] bool isLeaf(const Common::fs::directory_entry &entry) const override;
  std::unique_ptr<Leaf> leafLoad(Id id, const Common::fs::path &path) override;
  void leafValue(Id id, wxDataViewIconText &value, unsigned int col) const override;
  bool leafSave(Id id) override;

};

} // namespace Rapatas::Transmitron::GUI::Models

