#ifndef TRANSMITRON_MODELS_SNIPPETS_HPP
#define TRANSMITRON_MODELS_SNIPPETS_HPP

#include <memory>
#include <filesystem>
#include <wx/dataview.h>
#include "MQTT/Message.hpp"

namespace Transmitron::Models
{

class Snippets :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    Max
  };

  explicit Snippets();

  bool load(const std::string &connectionDir);

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
    std::vector<Index_t> children;
    std::unique_ptr<MQTT::Message> mMessage;
  };

  std::vector<Node> mNodes;
  std::string mSnippetsDir;

  void loadRecursive(const std::filesystem::path &snippetsDir);

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

  static Node::Index_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Index_t index);

};

}

#endif // TRANSMITRON_MODELS_SNIPPETS_HPP
