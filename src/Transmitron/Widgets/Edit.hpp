#ifndef TRANSMITRON_WIDGETS_EDIT_HPP
#define TRANSMITRON_WIDGETS_EDIT_HPP

#include <chrono>
#include <wx/stattext.h>
#include <wx/stc/stc.h>
#include <wx/wx.h>

#include "TopicCtrl.hpp"
#include "MQTT/Client.hpp"
#include "MQTT/Message.hpp"
#include "Transmitron/Events/Edit.hpp"

namespace Transmitron::Widgets
{

class Edit :
  public wxPanel
{
public:

  explicit Edit(
    wxWindow* parent,
    wxWindowID id,
    size_t optionsHeight,
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

  static constexpr uint32_t NoColor = 0x10000000;
  static constexpr uint32_t BrightnessBump = (80 << 0) | (80 << 8) | (80 << 16); // NOLINT

  static constexpr uint32_t Black  = (30  << 0) | (30  << 8) | (30  << 16); // NOLINT
  static constexpr uint32_t White  = (250 << 0) | (250 << 8) | (250 << 16); // NOLINT
  static constexpr uint32_t Red    = (180 << 0) | (0   << 8) | (0   << 16); // NOLINT
  static constexpr uint32_t Orange = (150 << 0) | (120 << 8) | (0   << 16); // NOLINT
  static constexpr uint32_t Green  = (0   << 0) | (150 << 8) | (0   << 16); // NOLINT
  static constexpr uint32_t Pink   = (200 << 0) | (0   << 8) | (150 << 16); // NOLINT
  static constexpr uint32_t Cyan   = (0   << 0) | (120 << 8) | (150 << 16); // NOLINT

  const std::map<Theme, std::map<Style, std::pair<uint32_t, uint32_t>>> mStyles {
    {Theme::Light, {
      {Style::Comment, {Black,  Black}},
      {Style::Editor,  {Black,  White}},
      {Style::Error,   {Black,  Red}},
      {Style::Normal,  {Black,  White}},
      {Style::Key,     {Green,  White}},
      {Style::Keyword, {Cyan,   White}},
      {Style::Number,  {Orange, White}},
      {Style::String,  {Orange, White}},
      {Style::Uri,     {Cyan,   White}},
      {Style::Special, {Pink,   White}},
    }},
    {Theme::Dark, {
      {Style::Comment, {White, Black}},
      {Style::Editor,  {White, Black}},
      {Style::Error,   {Black, Red}},
      {Style::Normal,  {White, Black}},
      {Style::Key,     {Green  | BrightnessBump, Black}},
      {Style::Keyword, {Cyan   | BrightnessBump, Black}},
      {Style::Number,  {Orange | BrightnessBump, Black}},
      {Style::String,  {Orange | BrightnessBump, Black}},
      {Style::Uri,     {Cyan   | BrightnessBump, Black}},
      {Style::Special, {Pink   | BrightnessBump, Black}},
    }}
  };

  Theme mTheme;
  wxFont mFont;

  bool mReadOnly = false;
  size_t mOptionsHeight;

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

  wxStaticText *mTimestamp = nullptr;

  wxButton *mSaveSnippet = nullptr;
  wxComboBox *mFormatSelect = nullptr;

  enum class Format
  {
    Auto,
    Text,
    Json,
    Xml
  };
  Format mCurrentFormat = Format::Auto;

  const std::map<std::string, Format> mFormats{
    {"Auto", Format::Auto},
    {"Text", Format::Text},
    {"Json", Format::Json},
    {"Xml", Format::Xml},
  };

  void onQosClicked(wxMouseEvent &e);
  void onRetainedClicked(wxMouseEvent &e);

  void setupScintilla();
  void setStyle(Format format);
  void onFormatSelected(wxCommandEvent &e);
  void onTopicKeyDown(wxKeyEvent &e);

  std::string formatTry(
    const std::string &text,
    Format format
  );

  static Format formatGuess(const std::string &text);
};

}

#endif // TRANSMITRON_WIDGETS_EDIT_HPP
