#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/listctrl.h>
#include <wx/menu.h>
#include <fmt/format.h>

#include "Common/Log.hpp"
#include "TopicCtrl.hpp"

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
  mLogger->info("Showing");

  if (mAutoComplete)
  {
    mAutoComplete->Destroy();
  }

  const auto filterPoint = GetScreenPosition();
  const auto filterSize = GetSize();

  const wxPoint popupPoint(filterPoint.x, filterPoint.y + filterSize.y);
  const wxSize popupSize(filterSize.x, 100);

  mAutoComplete = new wxPopupWindow(this);

  auto *placeholder = new wxListCtrl(
    mAutoComplete,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    wxLC_REPORT
  );

  // Add first column
  wxListItem col0;
  col0.SetId(0);
  col0.SetText( _("Foo")  );
  col0.SetWidth(50);
  placeholder->InsertColumn(0, col0);

  for (size_t i = 0; i != 10; ++i)
  {
    wxListItem item;
    item.SetId((long)i);
    item.SetText(fmt::format("amco/{}/coinman/#", i));
    placeholder->InsertItem(item);
  }

  mAutoComplete->Position(popupPoint - popupSize, popupSize);
  mAutoComplete->Show();
}

void TopicCtrl::popupHide()
{
  mLogger->info("Hiding");

  if (!mAutoComplete)
  {
    return;
  }

  mAutoComplete->Destroy();
  mAutoComplete = nullptr;
}
