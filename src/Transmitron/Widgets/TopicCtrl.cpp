#include "TopicCtrl.hpp"
#include <wx/artprov.h>
#include <wx/clipbrd.h>
#include <wx/log.h>
#include <wx/menu.h>

using namespace Transmitron::Widgets;

const wxFont TopicCtrl::Font = wxFont(wxFontInfo(9).FaceName("Consolas"));

TopicCtrl::TopicCtrl(
  wxWindow *parent,
  wxWindowID id
) :
  wxTextCtrl(parent, id)
{
  SetFont(Font);

  Bind(wxEVT_LEFT_UP,     &TopicCtrl::onLeftUp,        this);
  Bind(wxEVT_LEFT_DOWN,   &TopicCtrl::onLeftDown,      this);
  Bind(wxEVT_LEFT_DCLICK, &TopicCtrl::onDoubleClicked, this);
  Bind(wxEVT_RIGHT_DOWN,  &TopicCtrl::onRightClicked,  this);
  Bind(wxEVT_KILL_FOCUS,  &TopicCtrl::onLostFocus,     this);

  Bind(wxEVT_COMMAND_MENU_SELECTED, &TopicCtrl::onContextSelected, this);
}

void TopicCtrl::setReadOnly(bool readonly)
{
  mReadOnly = readonly;

  if (mReadOnly)
  {
    auto *dt = new NotAllowedDropTarget;
    SetDropTarget(dt);
    Bind(wxEVT_KEY_DOWN,     &TopicCtrl::onKeyDown);
    Bind(wxEVT_CONTEXT_MENU, &TopicCtrl::onContext, this);
  }
  else
  {
    Unbind(wxEVT_KEY_DOWN,     &TopicCtrl::onKeyDown);
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
  e.Skip();
}

void TopicCtrl::onKeyDown(wxKeyEvent &e)
{
  if (
    (e.ControlDown() && e.GetKeyCode() == 'C')
    || (e.ControlDown() && e.GetKeyCode() == 'A')
  ) {
    e.Skip();
  }
  else
  {
    e.Skip(false);
  }
}
