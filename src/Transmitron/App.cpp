#include <filesystem>
#include <stdexcept>

#include <fmt/core.h>
#include <wx/aui/auibook.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/settings.h>

#include "App.hpp"
#include "Events/Connection.hpp"
#include "Common/Log.hpp"
#include "Common/XdgBaseDir.hpp"
#include "Info.hpp"
#include "Resources/gear/gear-18x18.hpp"
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

constexpr size_t DefaultWindowWidth = 800;
constexpr size_t DefaultWindowHeight = 600;
constexpr size_t MinWindowWidth = 550;
constexpr size_t MinWindowHeight = 300;
constexpr size_t LabelFontSize = 15;

App::App() :
  LabelFontInfo(LabelFontSize)
{
  Common::Log::instance().initialize();
  mLogger = Common::Log::create("Transmitron::App");
}

bool App::OnInit()
{
  wxImage::AddHandler(new wxPNGHandler);
  wxImage::AddHandler(new wxICOHandler);

  mFrame = new wxFrame(
    nullptr,
    -1,
    "Homepage - Transmitron",
    wxDefaultPosition,
    wxSize(DefaultWindowWidth, DefaultWindowHeight)
  );
  mFrame->SetMinSize(wxSize(MinWindowWidth, MinWindowHeight));

  setupIcon();

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

  mLayoutsModel = new Models::Layouts();
  mLayoutsModel->load(getConfigDir().string());

  mProfilesModel = new Models::Profiles(mLayoutsModel);
  mProfilesModel->load(getConfigDir().string());

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
  auto *homepage = new Tabs::Homepage(
    mNote,
    LabelFontInfo,
    mProfilesModel,
    mLayoutsModel
  );
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
      mProfilesModel->getClientOptions(profileItem),
      mProfilesModel->getSnippetsModel(profileItem),
      mProfilesModel->getKnownTopicsModel(profileItem),
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
  mNote->AddPage(settingsTab, "", false, *bin2cGear18x18());
  ++mCount;
}

std::filesystem::path App::getConfigDir()
{
  const auto configHome = Common::XdgBaseDir::configHome();
  auto config = fmt::format("{}/{}", configHome.string(), getProjectName());

  if (!fs::is_directory(config) || !fs::exists(config))
  {
    if (!fs::create_directory(config))
    {
      mLogger->warn("Could not create directory '%s'", config);
      return {};
    }
  }

  mLogger->info("Config dir: {}", config);

  return config;
}

void App::setupIcon()
{
  std::filesystem::path p = fmt::format(
    "{}/share/transmitron/transmitron.ico",
    getInstallPrefix().string()
  );
  p.make_preferred();
  mLogger->debug("Loading icon from {}", p.string());
  mFrame->SetIcons(wxIconBundle(p.string()));
}

std::filesystem::path App::getExecutablePath()
{
  auto pathfinder = []()
  {
#if defined WIN32
    std::array<wchar_t, MAX_PATH> moduleFileName {};
    const auto r = GetModuleFileNameW(nullptr, moduleFileName.data(), MAX_PATH);
    if (r == 0)
    {
      throw std::runtime_error("Could not determine installation prefix");
    }

    return std::wstring(moduleFileName.data(), moduleFileName.size());
#else
    return std::filesystem::read_symlink("/proc/self/exe");
#endif
  };

  static const std::filesystem::path p = pathfinder();
  mLogger->info("Executable path: {}", p.string());
  return p;
}

std::filesystem::path App::getInstallPrefix()
{
  const std::filesystem::path executable = getExecutablePath();
  const auto executableDir = executable.parent_path();
  auto prefix = executableDir.parent_path();
  prefix.make_preferred();
  return prefix;
}
