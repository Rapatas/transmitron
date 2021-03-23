#ifndef TRANSMITRON_WIDGETS_EDIT_HPP
#define TRANSMITRON_WIDGETS_EDIT_HPP

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
    size_t optionsHeight
  );
  virtual ~Edit() = default;

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

private:

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
