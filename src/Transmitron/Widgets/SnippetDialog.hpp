#ifndef TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP
#define TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/dataview.h>
#include "Transmitron/Models/SnippetFolders.hpp"
#include "MQTT/Message.hpp"

namespace Transmitron::Widgets
{

class SnippetDialog :
  public wxDialog
{
public:

  explicit SnippetDialog(
    wxWindow *parent,
    wxWindowID id,
    wxObjectDataPtr<Models::SnippetFolders> snippetFoldersModel,
    size_t optionsHeight,
    std::shared_ptr<MQTT::Message> message
  );
  virtual ~SnippetDialog() = default;

private:

  enum class ContextIDs : unsigned
  {
    Rename,
    Delete,
    New,
  };

  const size_t OptionsHeight;

  wxObjectDataPtr<Models::SnippetFolders> mSnippetFoldersModel;
  wxDataViewCtrl *mSnippetsCtrl;
  wxTextCtrl *mSnippetName;
  std::array<wxDataViewColumn*, Models::SnippetFolders::Column::Max> mColumns;
  std::shared_ptr<MQTT::Message> mMessage;
  bool mExplicitEditRequest = false;

  void onSaveClicked(wxMouseEvent &e);
  void onCancelClicked(wxMouseEvent &e);
  void onNewFolderClicked(wxMouseEvent &e);
  void onKeyPressed(wxKeyEvent &e);
  void onCtrlEdit(wxDataViewEvent &e);
  void onCtrlContext(wxDataViewEvent &e);
  void onContextSelected(wxCommandEvent &e);
  void onContextSelectedRename(wxCommandEvent &e);
  void onContextSelectedDelete(wxCommandEvent &e);
  void onContextSelectedNew(wxCommandEvent &e);

  void handleInserted(wxDataViewItem &inserted);

  bool doSave();
};

}

#endif // TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP
