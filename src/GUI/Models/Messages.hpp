#pragma once

#include "Common/Filesystem.hpp"
#include <list>
#include <set>
#include <memory>

#include <spdlog/spdlog.h>
#include <wx/dataview.h>

#include "MQTT/Message.hpp"
#include "GUI/ArtProvider.hpp"

namespace Rapatas::Transmitron::GUI::Models
{

class Messages :
  public wxDataViewModel
{
public:

  enum Column : unsigned
  {
    Name,
    Max
  };

  explicit Messages(
    const ArtProvider &artProvider
  );

  bool load(const std::string &profileDir);

  [[nodiscard]] MQTT::Message getMessage(wxDataViewItem item) const;
  [[nodiscard]] std::string getName(wxDataViewItem item) const;
  [[nodiscard]] std::set<std::string> getKnownTopics() const;
  static wxDataViewItem getRootItem() ;
  wxDataViewItem createFolder(
    wxDataViewItem parent
  );
  wxDataViewItem createMessage(
    wxDataViewItem parent,
    const MQTT::Message &message
  );
  wxDataViewItem insert(
    const std::string &name,
    const MQTT::Message &message,
    wxDataViewItem parent
  );
  wxDataViewItem replace(
    wxDataViewItem item,
    const MQTT::Message &message
  );
  bool remove(wxDataViewItem item);

  wxDataViewItem moveBefore(wxDataViewItem item, wxDataViewItem sibling);
  wxDataViewItem moveAfter(wxDataViewItem item, wxDataViewItem sibling);
  wxDataViewItem moveInsideFirst(wxDataViewItem item, wxDataViewItem parent);
  wxDataViewItem moveInsideLast(wxDataViewItem item, wxDataViewItem parent);
  wxDataViewItem moveInsideAtIndex(
    wxDataViewItem item,
    wxDataViewItem parent,
    size_t index
  );

  [[nodiscard]] bool hasChildNamed(wxDataViewItem parent, const std::string &name) const;

  [[nodiscard]] unsigned GetColumnCount() const override;
  [[nodiscard]] wxString GetColumnType(unsigned int col) const override;
  void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  bool SetValue(
    const wxVariant &value,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  [[nodiscard]] bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  [[nodiscard]] wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  [[nodiscard]] bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &array
  ) const override;

private:

  struct Node
  {
    enum class Type
    {
      Folder,
      Message,
    };

    using Id_t = size_t;

    Id_t parent;
    std::string name;
    std::string encoded;
    Type type;
    std::list<Id_t> children;
    MQTT::Message message;
    bool saved;
  };

  std::shared_ptr<spdlog::logger> mLogger;
  Node::Id_t mNextAvailableId = 0;
  std::map<Node::Id_t, Node> mNodes;
  std::string mMessagesDir;
  const ArtProvider &mArtProvider;

  void loadDirectoryRecursive(
    const Common::fs::path &path,
    Node::Id_t parentId
  );
  void loadMessage(const Common::fs::path &path, Node::Id_t parentId);
  bool indexFileRead(const Common::fs::path &path, Node::Id_t id);
  bool indexFileWrite(Node::Id_t id);
  bool save(Node::Id_t id);
  bool saveMessage(Node::Id_t id);
  bool saveFolder(Node::Id_t id);
  bool moveFile(Node::Id_t nodeId, Node::Id_t newParentId);
  bool moveCheck(
    wxDataViewItem item,
    wxDataViewItem parent,
    size_t index
  );
  void moveUnderNewParent(
    Node::Id_t nodeId,
    Node::Id_t newParentId,
    size_t index
  );
  void moveUnderSameParent(
    Node::Id_t nodeId,
    Node::Id_t newParentId,
    size_t index
  );
  Node::Id_t getNextId();

  [[nodiscard]] bool isRecursive(wxDataViewItem parent, wxDataViewItem item) const;
  [[nodiscard]] std::string getNodePath(Node::Id_t id) const;

  static Node::Id_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Id_t id);
};

} // namespace Rapatas::Transmitron::GUI::Models

