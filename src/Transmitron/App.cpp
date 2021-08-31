#include <fmt/core.h>
#include <wx/artprov.h>
#include <wx/aui/auibook.h>
#include <wx/notebook.h>
#include <wx/settings.h>

#include "App.hpp"
#include "Events/Connection.hpp"
#include "Helpers/Log.hpp"
#include "Info.hpp"
#include "Resources/history/history-18x14.hpp"
#include "Resources/history/history-18x18.hpp"
#include "Resources/pin/not-pinned-18x18.hpp"
#include "Resources/pin/pinned-18x18.hpp"
#include "Resources/plus/plus-18x18.hpp"
#include "Resources/preview/preview-18x14.hpp"
#include "Resources/preview/preview-18x18.hpp"
#include "Resources/qos/qos-0.hpp"
#include "Resources/qos/qos-1.hpp"
#include "Resources/qos/qos-2.hpp"
#include "Resources/send/send-18x14.hpp"
#include "Resources/send/send-18x18.hpp"
#include "Resources/snippets/snippets-18x14.hpp"
#include "Resources/snippets/snippets-18x18.hpp"
#include "Resources/subscription/subscription-18x14.hpp"
#include "Resources/subscription/subscription-18x18.hpp"
#include "Tabs/Client.hpp"
#include "Tabs/Homepage.hpp"
#include "Tabs/Settings.hpp"
#include "Transmitron/Models/Layouts.hpp"

using namespace Transmitron;
namespace fs = std::filesystem;

constexpr size_t DefaultWindowWidth = 1280;
constexpr size_t DefaultWindowHeight = 720;
constexpr size_t MinWindowWidth = 550;
constexpr size_t MinWindowHeight = 300;
constexpr size_t LabelFontSize = 15;

App::App() :
  LabelFontInfo(LabelFontSize)
{
  Helpers::Log::instance().initialize();
  mLogger = Helpers::Log::create("Transmitron::App");
}

bool App::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);

  mFrame = new wxFrame(
    nullptr,
    -1,
    "Homepage - Transmitron",
    wxDefaultPosition,
    wxSize(DefaultWindowWidth, DefaultWindowHeight)
  );
  mFrame->SetMinSize(wxSize(MinWindowWidth, MinWindowHeight));

  const int noteStyle = wxAUI_NB_DEFAULT_STYLE & ~(0
    | wxAUI_NB_TAB_MOVE
    | wxAUI_NB_TAB_EXTERNAL_MOVE
    | wxAUI_NB_TAB_SPLIT
  );

  mNote = new wxAuiNotebook(
    mFrame,
    -1,
    wxDefaultPosition,
    wxDefaultSize,
    noteStyle
  );

  mProfilesModel = new Models::Profiles();
  mProfilesModel->load(getConfigDir());

  mLayoutsModel = new Models::Layouts();
  mLayoutsModel->load(getConfigDir());

  const auto appearance = wxSystemSettings::GetAppearance();
  mDarkMode = appearance.IsDark() || appearance.IsUsingDarkBackground();

  createSettingsTab();

  createProfilesTab(1);

  auto *newProfilesTab = new wxPanel(mNote);
  mNote->AddPage(newProfilesTab, "", false, *bin2cPlus18x18());
  ++mCount;

  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CHANGING, &App::onPageSelected, this);
  mNote->Bind(wxEVT_AUINOTEBOOK_PAGE_CLOSE, &App::onPageClosing, this);

  mFrame->Show();

  return true;
}

void App::onPageSelected(wxBookCtrlEvent& event)
{
  if (event.GetSelection() == wxNOT_FOUND)
  {
    event.Skip();
    return;
  }

  if ((size_t)event.GetSelection() == mCount - 1)
  {
    createProfilesTab(mCount - 1);
    event.Veto();
    return;
  }

  const auto style = mNote->GetWindowStyle();
  constexpr int Closable = 0
    | wxAUI_NB_CLOSE_BUTTON
    | wxAUI_NB_CLOSE_ON_ACTIVE_TAB;

  if ((size_t)event.GetSelection() == 0)
  {
    if ((style & Closable) != 0)
    {
      mNote->SetWindowStyle(style & ~Closable);
    }
  }
  else
  {
    if ((style & Closable) != Closable)
    {
      mNote->SetWindowStyle(style | Closable);
    }
  }

  const auto windowName = fmt::format(
    "{} - Transmitron",
    mNote->GetPage((size_t)event.GetSelection())->GetName()
  );
  mFrame->SetTitle(windowName);

  event.Skip();
}

void App::onPageClosing(wxBookCtrlEvent& event)
{
  const auto closingIndex = (size_t)event.GetSelection();

  // Settings and Plus.
  if (closingIndex == 0 || closingIndex == mCount - 1)
  {
    event.Veto();
    return;
  }

  --mCount;

  if (mCount == 2)
  {
    createProfilesTab(mCount - 1);
  }

  if ((size_t)event.GetOldSelection() == mCount - 1)
  {
    mNote->ChangeSelection(mCount - 2);
  }

  event.Skip();
}

void App::createProfilesTab(size_t index)
{
  auto *homepage = new Tabs::Homepage(mNote, LabelFontInfo, mProfilesModel);
  mNote->InsertPage(index, homepage, "Homepage");
  ++mCount;
  mNote->SetSelection(index);

  homepage->Bind(Events::CONNECTION, [this](Events::Connection e){
    if (e.GetSelection() == wxNOT_FOUND)
    {
      e.Skip();
      return;
    }

    const auto profileItem = e.getProfile();
    const size_t selected = (size_t)mNote->GetSelection();

    auto *client = new Tabs::Client(
      mNote,
      mProfilesModel->getBrokerOptions(profileItem),
      mProfilesModel->getSnippetsModel(profileItem),
      mLayoutsModel,
      mProfilesModel->getName(profileItem),
      mDarkMode
    );
    mNote->RemovePage(selected);
    mNote->InsertPage(selected, client, "");
    mNote->SetSelection(selected);
    mNote->SetPageText(selected, mProfilesModel->getName(profileItem));
  });
}

void App::createSettingsTab()
{
  auto *settingsTab = new Tabs::Settings(mNote, LabelFontInfo, mLayoutsModel);
  mNote->AddPage(settingsTab, "", false, wxArtProvider::GetBitmap(wxART_EDIT));
  ++mCount;
}

std::string App::getConfigDir()
{
  std::string result;

  char *xdg_config_home = getenv("XDG_CONFIG_HOME"); // NOLINT
  if (xdg_config_home == nullptr)
  {
    char *user = getenv("USER"); // NOLINT
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
      wxLogWarning("Could not create directory '%s'", result);
      result = {};
    }
  }

  return result;
}

