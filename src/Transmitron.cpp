#include "Client.hpp"
#include "Transmitron.hpp"
#include "LogFormat.hpp"
#include "images/send/send-18x18.hpp"
#include "images/pin/pinned-18x18.hpp"
#include "images/pin/not-pinned-18x18.hpp"
#include "images/plus/plus-18x18.hpp"
#include "images/qos/qos-0.hpp"
#include "images/qos/qos-1.hpp"
#include "images/qos/qos-2.hpp"
#include "Info.hpp"

#include <wx/notebook.h>

#define wxLOG_COMPONENT "Transmitron"

bool Transmitron::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);
  bin2c_init_SEND_18X18_HPP();
  bin2c_init_PINNED_18X18_HPP();
  bin2c_init_NOT_PINNED_18X18_HPP();
  bin2c_init_PLUS_18X18_HPP();
  bin2c_init_QOS_0_HPP();
  bin2c_init_QOS_1_HPP();
  bin2c_init_QOS_2_HPP();

  auto log = new wxLogStderr();
  log->SetFormatter(new MyLogFormatter());
  wxLog::SetActiveTarget(log);

  wxImageList * il = new wxImageList;
  il->Add(*bin2c_plus_18x18_png);

  auto frame = new wxFrame(nullptr, -1, getProjectName(), wxDefaultPosition, wxSize(800, 500));

  mNote = new wxAuiNotebook(frame, -1);
  mNote->SetImageList(il);

  newConnectionTab();

  auto secret = new wxPanel(mNote);
  mCount += mNote->AddPage(secret, "", false, 0);

  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &Transmitron::onPageSelected, this);
  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &Transmitron::onPageClosed, this);

  frame->Show();

  return true;
}

void Transmitron::onPageSelected(wxBookCtrlEvent& event)
{
  if (event.GetSelection() != mCount - 1)
  {
    event.Skip();
    return;
  }

  newConnectionTab();
  event.Veto();
};

void Transmitron::onPageClosed(wxBookCtrlEvent& event)
{
  --mCount;

  if (mCount == 1)
  {
    newConnectionTab();
  }

  if (event.GetSelection() == mCount - 1)
  {
    mNote->ChangeSelection(event.GetSelection() - 1);
  }
}

void Transmitron::newConnectionTab()
{
  auto homepage = new Homepage(mNote);
  mCount += mNote->InsertPage(mCount - 1, homepage, "Homepage");
  mNote->SetSelection(mCount - 2);

  homepage->Bind(EVT_CONNECTION, [this, &homepage](ConnectionEvent e){
    auto c = e.getConnection();
    size_t selected = mNote->GetSelection();

    auto client = new Client(mNote, c);
    mNote->RemovePage(selected);
    mNote->InsertPage(selected, client, "");
    mNote->SetSelection(selected);
    mNote->SetPageText(selected, c.getName());
    client->resize();
  });
}
