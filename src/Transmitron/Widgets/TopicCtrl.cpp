#include <cctype>
#include <wx/artprov.h>
#include <wx/dataview.h>
#include <wx/sizer.h>
#include <wx/clipbrd.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <fmt/format.h>

#include "Common/Log.hpp"
#include "TopicCtrl.hpp"
#include "Transmitron/Models/KnownTopics.hpp"

using namespace Transmitron::Widgets;

constexpr size_t FontSize = 9;

TopicCtrl::TopicCtrl(
  wxWindow *parent,
  wxWindowID id
) :
  wxTextCtrl(parent, id),
  mFont(wxFontInfo(FontSize).FaceName("Consolas"))
{
  SetFont(mFont);

  mLogger = Common::Log::create("Transmitron::TopicCtrl");

  Bind(wxEVT_LEFT_UP,     &TopicCtrl::onLeftUp,        this);
  Bind(wxEVT_LEFT_DOWN,   &TopicCtrl::onLeftDown,      this);
  Bind(wxEVT_LEFT_DCLICK, &TopicCtrl::onDoubleClicked, this);
  Bind(wxEVT_RIGHT_DOWN,  &TopicCtrl::onRightClicked,  this);
  Bind(wxEVT_KILL_FOCUS,  &TopicCtrl::onLostFocus,     this);
  Bind(wxEVT_KEY_DOWN,    &TopicCtrl::onKeyDown,       this);
  Bind(wxEVT_COMMAND_TEXT_UPDATED, &TopicCtrl::onValueChanged, this);

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
  popupHide();
  e.Skip();
}

void TopicCtrl::onKeyDown(wxKeyEvent &e)
{
  if (e.GetKeyCode() == WXK_ESCAPE)
  {
    popupHide();
    e.Skip(true);
    return;
  }

  if (e.GetKeyCode() == WXK_UP)
  {
    autoCompleteUp();
    e.Skip(false);
    return;
  }

  if (e.GetKeyCode() == WXK_DOWN)
  {
    autoCompleteDown();
    e.Skip(false);
    return;
  }

  if (e.GetKeyCode() == WXK_TAB)
  {
    autoCompleteSelect();
    e.Skip(false);
    return;
  }

  if (e.GetKeyCode() == WXK_RETURN)
  {
    popupHide();
    e.Skip(false);
  }

  if (e.GetKeyCode() == WXK_SPACE && e.ControlDown())
  {
    popupShow();
    e.Skip(true);
  }

  const bool allowedReadOnlyKeys =
    e.ControlDown()
    && (e.GetKeyCode() == 'C' || e.GetKeyCode() == 'A');

  if (mReadOnly && !allowedReadOnlyKeys)
  {
    e.Skip(false);
    return;
  }

  e.Skip();
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
  if (mKnownTopicsModel == nullptr)
  {
    return;
  }

  if (mKnownTopicsModel->GetCount() == 0)
  {
    return;
  }

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
  mAutoComplete->Position(popupPoint - popupSize, popupSize);
  mAutoComplete->SetSize(popupSize);

  wxDataViewColumn* const topic = new wxDataViewColumn(
    L"topic",
    new wxDataViewTextRenderer(),
    (unsigned)Models::KnownTopics::Column::Topic,
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

  mAutoCompleteList->AssociateModel(mKnownTopicsModel.get());
  mAutoCompleteList->AppendColumn(topic);

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

  auto *sizer = new wxBoxSizer(wxOrientation::wxHORIZONTAL);
  sizer->Add(mAutoCompleteList, 1, wxEXPAND);
  mAutoComplete->SetSizer(sizer);

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
