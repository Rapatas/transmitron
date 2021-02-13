#include "App.hpp"

#include <wx/notebook.h>
#include <fmt/core.h>

#include "Info.hpp"
#include "LogFormat.hpp"
#include "Resources/pin/not-pinned-18x18.hpp"
#include "Resources/pin/pinned-18x18.hpp"
#include "Resources/plus/plus-18x18.hpp"
#include "Resources/qos/qos-0.hpp"
#include "Resources/qos/qos-1.hpp"
#include "Resources/qos/qos-2.hpp"
#include "Resources/send/send-18x18.hpp"
#include "Resources/history/history-18x18.hpp"
#include "Resources/preview/preview-18x18.hpp"
#include "Resources/snippets/snippets-18x18.hpp"
#include "Resources/subscription/subscription-18x18.hpp"
#include "Resources/send/send-18x14.hpp"
#include "Resources/history/history-18x14.hpp"
#include "Resources/preview/preview-18x14.hpp"
#include "Resources/snippets/snippets-18x14.hpp"
#include "Resources/subscription/subscription-18x14.hpp"
#include "Tabs/Client.hpp"
#include "Tabs/Homepage.hpp"

#define wxLOG_COMPONENT "App"

using namespace Transmitron;
namespace fs = std::filesystem;

bool App::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);

  bin2c_init_HISTORY_18X14_HPP();
  bin2c_init_HISTORY_18X18_HPP();
  bin2c_init_NOT_PINNED_18X18_HPP();
  bin2c_init_PINNED_18X18_HPP();
  bin2c_init_PLUS_18X18_HPP();
  bin2c_init_PREVIEW_18X14_HPP();
  bin2c_init_PREVIEW_18X18_HPP();
  bin2c_init_QOS_0_HPP();
  bin2c_init_QOS_1_HPP();
  bin2c_init_QOS_2_HPP();
  bin2c_init_SEND_18X14_HPP();
  bin2c_init_SEND_18X18_HPP();
  bin2c_init_SNIPPETS_18X14_HPP();
  bin2c_init_SNIPPETS_18X18_HPP();
  bin2c_init_SUBSCRIPTION_18X14_HPP();
  bin2c_init_SUBSCRIPTION_18X18_HPP();

  auto log = new wxLogStderr();
  log->SetFormatter(new LogFormat());
  wxLog::SetActiveTarget(log);

  wxImageList * il = new wxImageList;
  il->Add(*bin2c_plus_18x18_png);

  auto frame = new wxFrame(
    nullptr,
    -1,
    getProjectName(),
    wxDefaultPosition,
    wxSize(800, 600)
  );
  frame->SetMinSize(wxSize(400, 300));

  mNote = new wxAuiNotebook(frame, -1);
  mNote->SetImageList(il);

  mConnectionsModel = new Models::Connections();
  mConnectionsModel->load(getConfigDir());

  newConnectionTab();

  auto secret = new wxPanel(mNote);
  mCount += mNote->AddPage(secret, "", false, 0);

  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &App::onPageSelected, this);
  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &App::onPageClosed, this);

  frame->Show();

  return true;
}

void App::onPageSelected(wxBookCtrlEvent& event)
{
  if (event.GetSelection() != mCount - 1)
  {
    event.Skip();
    return;
  }

  newConnectionTab();
  event.Veto();
};

void App::onPageClosed(wxBookCtrlEvent& event)
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

void App::newConnectionTab()
{
  auto homepage = new Tabs::Homepage(mNote, mConnectionsModel);
  mCount += mNote->InsertPage(mCount - 1, homepage, "Homepage");
  mNote->SetSelection(mCount - 2);

  homepage->Bind(Events::CONNECTION, [this, &homepage](Events::Connection e){
    auto c = e.getConnection();
    size_t selected = mNote->GetSelection();

    auto client = new Tabs::Client(mNote, c);
    mNote->RemovePage(selected);
    mNote->InsertPage(selected, client, "");
    mNote->SetSelection(selected);
    mNote->SetPageText(selected, c->getName());
  });
}

std::string App::getConfigDir()
{
  std::string result;

  char *xdg_config_home = getenv("XDG_CONFIG_HOME");
  if (xdg_config_home == nullptr)
  {
    char *user = getenv("USER");
    if (user != nullptr)
    {
      result = fmt::format("/home/{}/.config/{}", user, getProjectName());
    }
  }
  else
  {
    std::string xdgConfigHome(xdg_config_home);

    if (xdgConfigHome.back() != '/')
    {
      xdgConfigHome += '/';
    }

    result = xdgConfigHome + getProjectName();
  }

  if (!fs::is_directory(result) || !fs::exists(result))
  {
    if (!fs::create_directory(result))
    {
      wxLogWarning("Could not create directory '%s'", result.c_str());
      result = {};
    }
  }

  return result;
}

