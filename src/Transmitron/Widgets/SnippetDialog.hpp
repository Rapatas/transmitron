#ifndef TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP
#define TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP

#include <wx/dialog.h>
#include <wx/button.h>
#include <wx/textctrl.h>
#include <wx/dataview.h>
#include "Transmitron/Models/SnippetFolders.hpp"

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
    size_t optionsHeight
  );
  virtual ~SnippetDialog() = default;

private:

  const size_t OptionsHeight;
  wxObjectDataPtr<Models::SnippetFolders> mSnippetFoldersModel;
  wxDataViewCtrl *mSnippetsCtrl;
  wxTextCtrl *mSnippetName;

};

}

#endif // TRANSMITRON_WIDGETS_SNIPPETDIALOG_HPP
