#ifndef TRANSMITRON_MODELS_SNIPPETS_HPP
#define TRANSMITRON_MODELS_SNIPPETS_HPP

#include <memory>
#include <list>
#include <filesystem>
#include <wx/dataview.h>
#include "MQTT/Message.hpp"

namespace Transmitron::Models
{

class Snippets :
  public wxDataViewModel
{
public:

  enum Column : unsigned
  {
    Name,
    Max
  };

  explicit Snippets();

  bool load(const std::string &connectionDir);

  MQTT::Message getMessage(wxDataViewItem item) const;
  wxDataViewItem getRootItem() const;
  wxDataViewItem createFolder(
    wxDataViewItem parent
  );
  wxDataViewItem createSnippet(
    wxDataViewItem parent,
    std::shared_ptr<MQTT::Message> message
  );
  wxDataViewItem insert(
    const std::string &name,
    std::shared_ptr<MQTT::Message> message,
    wxDataViewItem parent
  );
  wxDataViewItem replace(
    wxDataViewItem item,
    std::shared_ptr<MQTT::Message> message
  );
  bool remove(wxDataViewItem item);

  wxDataViewItem moveBefore(wxDataViewItem item, wxDataViewItem sibling);
  wxDataViewItem moveInside(wxDataViewItem item, wxDataViewItem parent);
  wxDataViewItem move(
    wxDataViewItem item,
    wxDataViewItem parent,
    wxDataViewItem sibling
  );

  bool hasChildNamed(wxDataViewItem parent, const std::string &name) const;

  virtual unsigned GetColumnCount() const override;
  virtual wxString GetColumnType(unsigned int col) const override;
  virtual void GetValue(
    wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual bool SetValue(
    const wxVariant &variant,
    const wxDataViewItem &item,
    unsigned int col
  ) override;
  virtual bool IsEnabled(
    const wxDataViewItem &item,
    unsigned int col
  ) const override;
  virtual wxDataViewItem GetParent(
    const wxDataViewItem &item
  ) const override;
  virtual bool IsContainer(
    const wxDataViewItem &item
  ) const override;
  virtual unsigned int GetChildren(
    const wxDataViewItem &parent,
    wxDataViewItemArray &array
  ) const override;

private:

  struct Node
  {
    enum class Type
    {
      Folder,
      Snippet,
    };

    using Id_t = size_t;

    Id_t parent;
    std::string name;
    std::string encoded;
    Type type;
    std::list<Id_t> children;
    std::shared_ptr<MQTT::Message> message;
    bool saved;
  };

  Node::Id_t mNextAvailableId = 0;
  std::map<Node::Id_t, Node> mNodes;
  std::string mSnippetsDir;

  void loadDirectoryRecursive(
    const std::filesystem::path &path,
    Node::Id_t parentId
  );
  void loadSnippet(
    const std::filesystem::path &path,
    Node::Id_t parentId
  );
  void saveAll();
  bool save(Node::Id_t id);
  Node::Id_t getNextId();
  bool isRecursive(wxDataViewItem parent, wxDataViewItem item) const;
  std::string getNodePath(Node::Id_t id) const;

  static std::string decode(const std::string &encoded);
  static Node::Id_t toId(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Id_t id);

};

}

#endif // TRANSMITRON_MODELS_SNIPPETS_HPP
