#include <wx/frame.h>
#include <cctype>
#include <wx/artprov.h>
#include <wx/dataview.h>
#include <wx/gdicmn.h>
#include <wx/sizer.h>
#include <wx/clipbrd.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <fmt/format.h>

#include "Common/Log.hpp"
#include "TopicCtrl.hpp"
#include "GUI/Events/TopicCtrl.hpp"
#include "GUI/Models/KnownTopics.hpp"

using namespace Rapatas::Transmitron;
using namespace GUI;
using namespace GUI::Widgets;

constexpr size_t FontSize = 9;

TopicCtrl::TopicCtrl(
  wxWindow *parent,
  wxWindowID id
) :
  wxTextCtrl(parent, id),
  mFont(wxFontInfo(FontSize).FaceName("Consolas"))
{
  SetFont(mFont);
  SetWindowStyle(wxWANTS_CHARS);

  SetHint("Topic...");

  mLogger = Common::Log::create("GUI::TopicCtrl");

  Bind(wxEVT_LEFT_UP,     &TopicCtrl::onLeftUp,        this);
  Bind(wxEVT_LEFT_DOWN,   &TopicCtrl::onLeftDown,      this);
  Bind(wxEVT_LEFT_DCLICK, &TopicCtrl::onDoubleClicked, this);
  Bind(wxEVT_RIGHT_DOWN,  &TopicCtrl::onRightClicked,  this);
  Bind(wxEVT_KILL_FOCUS,  &TopicCtrl::onLostFocus,     this);
  Bind(wxEVT_KEY_DOWN,    &TopicCtrl::onKeyDown,       this);
  Bind(wxEVT_KEY_UP,      &TopicCtrl::onKeyUp,         this);

#ifdef WIN32

  // Required until stop Ctrl-A and Return since causing a wxBell on Windows.
  Bind(wxEVT_CHAR, &TopicCtrl::onChar, this);

#endif

  Bind(wxEVT_COMMAND_TEXT_UPDATED,  &TopicCtrl::onValueChanged,    this);
  Bind(wxEVT_COMMAND_MENU_SELECTED, &TopicCtrl::onContextSelected, this);
}

void TopicCtrl::setReadOnly(bool readonly)
{
  mReadOnly = readonly;

  if (mReadOnly)
  {
    auto *target = new NotAllowedDropTarget;
    SetDropTarget(target);
    Bind(wxEVT_CONTEXT_MENU, &TopicCtrl::onContext, this);
  }
  else
  {
    Unbind(wxEVT_CONTEXT_MENU, &TopicCtrl::onContext, this);
  }
}

void TopicCtrl::addKnownTopics(
  const wxObjectDataPtr<Models::KnownTopics> &knownTopicsModel
) {
  mKnownTopicsModel = knownTopicsModel;
}

void TopicCtrl::onContextSelected(wxCommandEvent &event)
{
  switch (static_cast<ContextIDs>(event.GetId()))
  {
    case ContextIDs::Copy: {
      if (wxTheClipboard->Open())
      {
        long fromIn = 0;
        long toIn = 0;
        GetSelection(&fromIn, &toIn);
        const auto since = static_cast<size_t>(fromIn);
        const auto until = static_cast<size_t>(toIn);

        auto topic = GetValue();
        if (since != until)
        {
          topic = topic.substr(since, until - since);
        }
        auto *dataObject = new wxTextDataObject(topic);
        wxTheClipboard->SetData(dataObject);
        wxTheClipboard->Close();
      }
    } break;
  }
}

void TopicCtrl::onLeftUp(wxMouseEvent &event)
{
  long since = 0;
  long until = 0;
  GetSelection(&since, &until);
  mFakeSelection = false;
  if (since == until)
  {
    mCursonPosition = since;

    if (mFirstClick)
    {
      SetSelection(-1, -1);
      mFakeSelection = true;
    }
  }
  mFirstClick = false;
  event.Skip();
}

void TopicCtrl::onLeftDown(wxMouseEvent &event)
{
  long since = 0;
  long until = 0;
  GetSelection(&since, &until);
  if (since != until)
  {
    mFirstClick = false;
  }
  popupShow();
  event.Skip();
}

void TopicCtrl::onRightClicked(wxMouseEvent &event)
{
  long since = 0;
  long until = 0;
  GetSelection(&since, &until);
  mFakeSelection = false;
  if (since == until)
  {
    SetSelection(-1, -1);
    mFakeSelection = true;
  }
  mFirstClick = false;
  event.Skip();
}

void TopicCtrl::onValueChanged(wxCommandEvent &/* event */)
{
  popupRefresh();
}

void TopicCtrl::onCompletionDoubleClicked(wxDataViewEvent &event)
{
  if (!event.GetItem().IsOk()) { return; }
  autoCompleteSelect();
}

void TopicCtrl::onCompletionLeftUp(wxMouseEvent &event)
{
  const auto selected = mAutoCompleteList->GetSelection();
  if (!selected.IsOk()) { return; }

  const auto point = event.GetPosition();
  wxDataViewItem item;
  mAutoCompleteList->HitTest(point, item, mAutoCompleteTopic);
  if (item != selected) { return; }

  autoCompleteSelect();
}

void TopicCtrl::onDoubleClicked(wxMouseEvent &event)
{
  if (mFakeSelection)
  {
    SetSelection(mCursonPosition, mCursonPosition);
  }
  event.Skip();
}

void TopicCtrl::onContext(wxContextMenuEvent &event)
{
  wxMenu menu;
  auto *item = new wxMenuItem(nullptr, static_cast<unsigned>(ContextIDs::Copy), "Copy");
  item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
  menu.Append(item);
  PopupMenu(&menu);

  event.Skip(false);
}

void TopicCtrl::onLostFocus(wxFocusEvent &event)
{
  mFirstClick = true;
  SetSelection(0, 0);
  popupHide();
  event.Skip();
}

void TopicCtrl::onKeyUp(wxKeyEvent &event)
{
  if (event.GetKeyCode() == WXK_RETURN && !GetValue().empty())
  {
    auto *event = new Events::TopicCtrl(Events::TOPICCTRL_RETURN);
    wxQueueEvent(this, event);
    return;
  }

#ifdef WIN32

  if (event.ControlDown() && event.GetKeyCode() == 'A')
  {
    SetSelection(-1, -1);
    event.Skip(false);
    return;
  }

#endif

  event.Skip();
}

void TopicCtrl::onChar(wxKeyEvent &event)
{
  (void)this;

  // Don't skip if Enter.
  if (event.GetKeyCode() == WXK_RETURN)
  {
    return;
  }

  // Don't skip if `Ctrl-A`.
  if (event.ControlDown() && event.GetKeyCode() == 1)
  {
    return;
  }

  event.Skip();
}

void TopicCtrl::onKeyDown(wxKeyEvent &event)
{
  if (event.GetKeyCode() == WXK_ESCAPE)
  {
    popupHide();
    return;
  }

  if (event.GetKeyCode() == WXK_UP)
  {
    autoCompleteUp();
    return;
  }

  if (event.GetKeyCode() == WXK_DOWN)
  {
    autoCompleteDown();
    return;
  }

  if (event.GetKeyCode() == WXK_TAB)
  {
    if (mAutoComplete == nullptr)
    {
      HandleAsNavigationKey(event);
      return;
    }

    autoCompleteSelect();
    return;
  }

  if (event.GetKeyCode() == WXK_RETURN)
  {
    popupHide();
    return;
  }

  if (event.GetKeyCode() == WXK_SPACE && event.ControlDown())
  {
    popupShow();
    return;
  }

  const bool allowedReadOnlyKeys = true // NOLINT
    && event.ControlDown()
    && (event.GetKeyCode() == 'C' || event.GetKeyCode() == 'A');
  if (mReadOnly && !allowedReadOnlyKeys)
  {
    return;
  }

  event.Skip(true);
}

wxDragResult TopicCtrl::NotAllowedDropTarget::OnData(
  wxCoord /* x */,
  wxCoord /* y */,
  wxDragResult /* defResult */
) {
  return wxDragResult::wxDragNone;
}

void TopicCtrl::popupShow()
{
  if (mKnownTopicsModel == nullptr) { return; }
  if (mKnownTopicsModel->GetCount() == 0) { return; }

  if (mAutoComplete != nullptr)
  {
    mAutoComplete->Destroy();
  }

  mPopupShow = true;

  const auto filterPoint = GetScreenPosition();
  const auto filterSize = GetSize();

  const wxPoint popupPoint(filterPoint.x, filterPoint.y + filterSize.y);
  const wxSize popupSize(filterSize.x, 150);

  mAutoComplete = new wxPopupWindow(this);
  mAutoComplete->SetSize(popupSize);
  mAutoComplete->Move(popupPoint);

  mAutoCompleteTopic = new wxDataViewColumn(
    L"topic",
    new wxDataViewTextRenderer(),
    static_cast<unsigned>(Models::KnownTopics::Column::Topic),
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mAutoCompleteList = new wxDataViewCtrl(
    mAutoComplete,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxDV_NO_HEADER | wxDV_ROW_LINES
  );
  mAutoCompleteList->SetSize(popupSize);

  mAutoCompleteList->AssociateModel(mKnownTopicsModel.get());
  mAutoCompleteList->AppendColumn(mAutoCompleteTopic);

  if (mKnownTopicsModel->GetCount() != 0)
  {
    const auto firstItem = mKnownTopicsModel->GetItem(0);
    mAutoCompleteList->Select(firstItem);
  }

  mAutoCompleteList->Bind(
    wxEVT_DATAVIEW_ITEM_ACTIVATED,
    &TopicCtrl::onCompletionDoubleClicked,
    this
  );

  mAutoCompleteList->Bind(
    wxEVT_LEFT_UP,
    &TopicCtrl::onCompletionLeftUp,
    this
  );

  auto *wrapperSizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wrapperSizer->Add(mAutoCompleteList, 1, wxEXPAND);
  mAutoComplete->SetSizer(wrapperSizer);

  mAutoComplete->Show();
}

void TopicCtrl::popupHide()
{
  if (mAutoComplete == nullptr)
  {
    return;
  }

  mPopupShow = false;
  mAutoComplete->Destroy();
  mAutoComplete = nullptr;
}

void TopicCtrl::popupRefresh()
{
  if (mKnownTopicsModel == nullptr)
  {
    return;
  }

  const auto filter = GetValue().ToStdString();

  mKnownTopicsModel->setFilter(filter);
  if (mKnownTopicsModel->GetCount() == 0)
  {
    popupHide();
  }
  else if (mPopupShow && mAutoComplete == nullptr)
  {
    popupShow();
  }
}

void TopicCtrl::autoCompleteUp()
{
  if (mAutoCompleteList == nullptr)
  {
    return;
  }

  const auto selected = mAutoCompleteList->GetSelection();
  const auto row = mKnownTopicsModel->GetRow(selected);
  if (row == 0)
  {
    return;
  }

  const auto newSelection = mKnownTopicsModel->GetItem(row - 1);
  mAutoCompleteList->Select(newSelection);
  mAutoCompleteList->EnsureVisible(newSelection);
}

void TopicCtrl::autoCompleteDown()
{
  if (mAutoCompleteList == nullptr)
  {
    return;
  }

  const auto selected = mAutoCompleteList->GetSelection();
  const auto row = mKnownTopicsModel->GetRow(selected);
  if (row + 1 == mKnownTopicsModel->GetCount())
  {
    return;
  }

  const auto newSelection = mKnownTopicsModel->GetItem(row + 1);
  mAutoCompleteList->Select(newSelection);
  mAutoCompleteList->EnsureVisible(newSelection);
}

void TopicCtrl::autoCompleteSelect()
{
  if (!mPopupShow) { return; }
  if (mAutoCompleteList == nullptr) { return; }

  const auto selected = mAutoCompleteList->GetSelection();
  if (!selected.IsOk()) { return; }

  const auto topic = mKnownTopicsModel->getTopic(selected);
  SetValue(topic);
  const auto position = static_cast<long>(topic.size());
  SetSelection(position, position);
  popupHide();
}
