#pragma once

#include <chrono>

#include <spdlog/spdlog.h>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/wx.h>

#include "MQTT/Client.hpp"
#include "MQTT/Message.hpp"
#include "TopicCtrl.hpp"
#include "GUI/Events/Edit.hpp"
#include "GUI/Events/TopicCtrl.hpp"

namespace Rapatas::Transmitron::GUI::Widgets
{

class Edit :
  public wxPanel
{
public:

  explicit Edit(
    wxWindow* parent,
    wxWindowID id,
    int optionsHeight,
    bool darkMode
  );

  void format();

  MQTT::Message getMessage() const;
  std::string getPayload() const;
  bool getReadOnly() const;
  std::string getTopic() const;
  MQTT::QoS getQos() const;
  bool getRetained() const;

  void setReadOnly(bool readonly);
  void clear();

  void setMessage(const MQTT::Message &message);
  void setPayload(const std::string &text);
  void setRetained(bool retained);
  void setTopic(const std::string &topic);
  void setQos(MQTT::QoS qos);
  void setTimestamp(const std::chrono::system_clock::time_point &timestamp);
  void setInfoLine(const std::string &info);
  void addKnownTopics(
    const wxObjectDataPtr<Models::KnownTopics> &knownTopicsModel
  );

private:

  enum class Theme : uint8_t
  {
    Light,
    Dark,
  };

  enum class Style : uint8_t
  {
    Comment,
    Editor,
    Error,
    Key,
    Keyword,
    Normal,
    Number,
    Special,
    String,
    Uri,
  };

  using ThemeStyles = std::map<Style, std::pair<uint32_t, uint32_t>>;

  std::shared_ptr<spdlog::logger> mLogger;
  const std::map<Theme, ThemeStyles> mStyles;

  Theme mTheme;
  wxFont mFont;

  bool mReadOnly = false;
  const int mOptionsHeight;

  wxBoxSizer *mTop = nullptr;
  wxBoxSizer *mVsizer = nullptr;
  wxBoxSizer *mBottom = nullptr;

  TopicCtrl *mTopic = nullptr;

  bool mRetained;
  wxStaticBitmap *mRetainedTrue = nullptr;
  wxStaticBitmap *mRetainedFalse = nullptr;

  MQTT::QoS mQoS;
  wxStaticBitmap *mQos0 = nullptr;
  wxStaticBitmap *mQos1 = nullptr;
  wxStaticBitmap *mQos2 = nullptr;

  wxBitmapButton *mPublish = nullptr;

  wxStyledTextCtrl *mText = nullptr;
  std::string mPayload;

  wxStaticText *mInfoLine = nullptr;

  wxButton *mSaveMessage = nullptr;
  wxComboBox *mFormatSelect = nullptr;

  enum class Format
  {
    Auto,
    Text,
    Json,
    Xml,
    Binary,
  };
  Format mCurrentFormat = Format::Auto;

  const std::map<std::string, Format> mFormats{
    {"Auto", Format::Auto},
    {"Text", Format::Text},
    {"Json", Format::Json},
    {"Xml", Format::Xml},
    {"Binary", Format::Binary},
  };

  void onQosClicked(wxMouseEvent &event);
  void onRetainedClicked(wxMouseEvent &event);

  void setupScintilla();
  void setStyle(Format format);
  void onFormatSelected(wxCommandEvent &event);
  void onTopicCtrlReturn(Events::TopicCtrl &event);

  std::string formatTry(
    const std::string &text,
    Format format
  );

  static Format formatGuess(const std::string &text);
  static std::map<Theme, ThemeStyles> initThemes();
};

} // namespace Rapatas::Transmitron::GUI::Widgets

