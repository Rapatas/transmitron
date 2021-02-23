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

  enum Column : unsigned
  {
    Name,
    Max
  };

  explicit SnippetFolders(const wxObjectDataPtr<Snippets> snippetsModel);

  wxDataViewItem getRootItem() const;
  wxDataViewItem createFolder(
    wxDataViewItem parent
  );
  wxDataViewItem insert(
    const std::string &name,
    std::shared_ptr<MQTT::Message> message,
    wxDataViewItem parent
  );
  bool remove(wxDataViewItem item);

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

  static constexpr size_t FakeRootId = std::numeric_limits<size_t>::max();

  wxObjectDataPtr<Snippets> mSnippetsModel;

  static size_t toIndex(const wxDataViewItem &item);
  static wxDataViewItem toItem(size_t index);

};

}

#endif // TRANSMITRON_MODELS_SNIPPETFOLDERS_HPP
