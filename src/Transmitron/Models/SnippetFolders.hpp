#ifndef TRANSMITRON_MODELS_SNIPPETFOLDERS_HPP
#define TRANSMITRON_MODELS_SNIPPETFOLDERS_HPP

#include <memory>
#include <filesystem>
#include <wx/dataview.h>
#include "Snippets.hpp"
#include "MQTT/Message.hpp"

namespace Transmitron::Models
{

class SnippetFolders :
  public wxDataViewModel
{
public:

  enum class Column : unsigned
  {
    Name,
    Max
  };

  explicit SnippetFolders();

  bool load(const wxObjectDataPtr<Snippets> snippetsModel);

  wxDataViewItem getRootItem() const;

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
    using Index_t = size_t;

    Index_t parent;
    std::string name;
    std::vector<Index_t> children;
    wxDataViewItem snippetItem;
  };

  std::vector<Node> mNodes;
  std::string mSnippetFoldersDir;

  void loadRecursive(
    const wxDataViewItem &parent,
    wxObjectDataPtr<Snippets> snippetsModel
  );

  static Node::Index_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(Node::Index_t index);

};

}

#endif // TRANSMITRON_MODELS_SNIPPETFOLDERS_HPP
