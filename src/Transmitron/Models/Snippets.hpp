#ifndef TRANSMITRON_MODELS_SNIPPETS_HPP
#define TRANSMITRON_MODELS_SNIPPETS_HPP

#include <memory>
#include <set>
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

    using Index_t = size_t;

    Index_t parent;
    std::string name;
    Type type;
    std::set<Index_t> children;
    std::shared_ptr<MQTT::Message> message;
    bool saved;
    std::filesystem::path fullpath;
  };

  Node::Index_t mNextAvailableIndex = 0;
  std::map<Node::Index_t, Node> mNodes;
  std::string mSnippetsDir;

  void loadRecursive(
    Node::Index_t parentIndex,
    const std::filesystem::path &parentFullpath
  );
  void saveAll();
  bool save(Node::Index_t index);
  Node::Index_t getNextIndex();

  static Node::Index_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Index_t index);

};

}

#endif // TRANSMITRON_MODELS_SNIPPETS_HPP
