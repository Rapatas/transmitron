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
#include "Transmitron/Events/TopicCtrl.hpp"
#include "Transmitron/Events/TopicCtrl.hpp"
#include "Transmitron/Models/KnownTopics.hpp"

using namespace Transmitron;
using namespace Transmitron::Widgets;

wxDEFINE_EVENT(Events::TOPICCTRL_RETURN, Events::TopicCtrl); // NOLINT

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

  mLogger = Common::Log::create("Transmitron::TopicCtrl");

  Bind(wxEVT_LEFT_UP,     &TopicCtrl::onLeftUp,        this);
  Bind(wxEVT_LEFT_DOWN,   &TopicCtrl::onLeftDown,      this);
  Bind(wxEVT_LEFT_DCLICK, &TopicCtrl::onDoubleClicked, this);
  Bind(wxEVT_RIGHT_DOWN,  &TopicCtrl::onRightClicked,  this);
  Bind(wxEVT_KILL_FOCUS,  &TopicCtrl::onLostFocus,     this);
  Bind(wxEVT_KEY_DOWN,    &TopicCtrl::onKeyDown,       this);
  Bind(wxEVT_KEY_UP,      &TopicCtrl::onKeyUp,         this);

#ifdef WIN32

  // Required to stop Ctrl-A and Return from causing a wxBell on Windows.
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
    auto *dt = new NotAllowedDropTarget;
    SetDropTarget(dt);
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

void TopicCtrl::onContextSelected(wxCommandEvent &e)
{
  switch ((ContextIDs)e.GetId())
  {
    case ContextIDs::Copy: {
      if (wxTheClipboard->Open())
      {
        long fromIn = 0;
        long toIn = 0;
        GetSelection(&fromIn, &toIn);
        const auto from = (size_t)fromIn;
        const auto to   = (size_t)toIn;

        auto topic = GetValue();
        if (from != to)
        {
          topic = topic.substr(from, to - from);
        }
        auto *dataObject = new wxTextDataObject(topic);
        wxTheClipboard->SetData(dataObject);
        wxTheClipboard->Close();
      }
    } break;
  }
}

void TopicCtrl::onLeftUp(wxMouseEvent &e)
{
  long from = 0;
  long to = 0;
  GetSelection(&from, &to);
  mFakeSelection = false;
  if (from == to)
  {
    mCursonPosition = from;

    if (mFirstClick)
    {
      SetSelection(-1, -1);
      mFakeSelection = true;
    }
  }
  mFirstClick = false;
  e.Skip();
}

void TopicCtrl::onLeftDown(wxMouseEvent &e)
{
  long from = 0;
  long to = 0;
  GetSelection(&from, &to);
  if (from != to)
  {
    mFirstClick = false;
  }
  popupShow();
  e.Skip();
}

void TopicCtrl::onRightClicked(wxMouseEvent &e)
{
  long from = 0;
  long to = 0;
  GetSelection(&from, &to);
  mFakeSelection = false;
  if (from == to)
  {
    SetSelection(-1, -1);
    mFakeSelection = true;
  }
  mFirstClick = false;
  e.Skip();
}

void TopicCtrl::onValueChanged(wxCommandEvent &/* e */)
{
  popupRefresh();
}

void TopicCtrl::onCompletionDoubleClicked(wxDataViewEvent &e)
{
  if (!e.GetItem().IsOk()) { return; }
  autoCompleteSelect();
}

void TopicCtrl::onCompletionLeftUp(wxMouseEvent &e)
{
  const auto selected = mAutoCompleteList->GetSelection();
  if (!selected.IsOk()) { return; }

  const auto point = e.GetPosition();
  wxDataViewItem item;
  mAutoCompleteList->HitTest(point, item, mAutoCompleteTopic);
  if (item != selected) { return; }

  autoCompleteSelect();
}

void TopicCtrl::onDoubleClicked(wxMouseEvent &e)
{
  if (mFakeSelection)
  {
    SetSelection(mCursonPosition, mCursonPosition);
  }
  e.Skip();
}

void TopicCtrl::onContext(wxContextMenuEvent &e)
{
  wxMenu menu;
  auto *item = new wxMenuItem(nullptr, (unsigned)ContextIDs::Copy, "Copy");
  item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY));
  menu.Append(item);
  PopupMenu(&menu);

  e.Skip(false);
}

void TopicCtrl::onLostFocus(wxFocusEvent &e)
{
  mFirstClick = true;
  SetSelection(0, 0);
  popupHide();
  e.Skip();
}

void TopicCtrl::onKeyUp(wxKeyEvent &event)
{
  if (event.GetKeyCode() == WXK_RETURN && !GetValue().empty())
  {
    auto *e = new Events::TopicCtrl(Events::TOPICCTRL_RETURN);
    wxQueueEvent(this, e);
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

void TopicCtrl::onKeyDown(wxKeyEvent &e)
{
  if (e.GetKeyCode() == WXK_ESCAPE)
  {
    popupHide();
    return;
  }

  if (e.GetKeyCode() == WXK_UP)
  {
    autoCompleteUp();
    return;
  }

  if (e.GetKeyCode() == WXK_DOWN)
  {
    autoCompleteDown();
    return;
  }

  if (e.GetKeyCode() == WXK_TAB)
  {
    if (mAutoComplete == nullptr)
    {
      HandleAsNavigationKey(e);
      return;
    }

    autoCompleteSelect();
    return;
  }

  if (e.GetKeyCode() == WXK_RETURN)
  {
    popupHide();
    return;
  }

  if (e.GetKeyCode() == WXK_SPACE && e.ControlDown())
  {
    popupShow();
    return;
  }

  const bool allowedReadOnlyKeys = true // NOLINT
    && e.ControlDown()
    && (e.GetKeyCode() == 'C' || e.GetKeyCode() == 'A');
  if (mReadOnly && !allowedReadOnlyKeys)
  {
    return;
  }

  e.Skip(true);
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

  auto *wrapper = new wxPanel(
    mAutoComplete,
    wxID_ANY,
    wxDefaultPosition,
    wxDefaultSize
  );

  mAutoCompleteTopic = new wxDataViewColumn(
    L"topic",
    new wxDataViewTextRenderer(),
    (unsigned)Models::KnownTopics::Column::Topic,
    wxCOL_WIDTH_AUTOSIZE,
    wxALIGN_LEFT
  );

  mAutoCompleteList = new wxDataViewCtrl(
    wrapper,
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

  auto *windowSizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  windowSizer->Add(wrapper, 1, wxEXPAND);
  mAutoComplete->SetSizer(windowSizer);

  auto *wrapperSizer = new wxBoxSizer(wxOrientation::wxVERTICAL);
  wrapperSizer->Add(mAutoCompleteList, 1, wxEXPAND);
  wrapper->SetSizer(wrapperSizer);

  wrapper->SetSize(popupSize);

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
