#ifndef TRANSMITRON_WIDGETS_EDIT_HPP
#define TRANSMITRON_WIDGETS_EDIT_HPP

#include <wx/stc/stc.h>
#include <wx/wx.h>

#include "TopicCtrl.hpp"
#include "MQTT/Client.hpp"

namespace Transmitron::Widgets
{

class Edit :
  public wxPanel
{
public:

  explicit Edit(
    wxWindow* parent,
    wxWindowID id
  );
  virtual ~Edit() = default;

  void format();

  std::string getPayload() const;
  bool getReadOnly() const;
  std::string getTopic() const;
  MQTT::QoS getQos() const;
  bool getRetained() const;

  void setReadOnly(bool readonly);
  void clear();

  void setPayload(const std::string &text);
  void setRetained(bool retained);
  void setTopic(const std::string &topic);
  void setQos(MQTT::QoS qos);

private:

  wxFont mFont;

  bool mReadOnly = false;

  wxBoxSizer *mTop;
  wxBoxSizer *mVsizer;
  wxBoxSizer *mBottom;

  TopicCtrl *mTopic;

  bool mRetained;
  wxStaticBitmap *mRetainedTrue;
  wxStaticBitmap *mRetainedFalse;

  MQTT::QoS mQoS;
  wxStaticBitmap *mQos0;
  wxStaticBitmap *mQos1;
  wxStaticBitmap *mQos2;

  wxBitmapButton *mPublish;

  wxStyledTextCtrl *mText;

  wxButton *mFormat;
  wxComboBox *mFormatSelect;

  void onQosClicked(wxMouseEvent &e);
  void onRetainedClicked(wxMouseEvent &e);

  void setupScintilla();
  void onFormatSelected(wxCommandEvent& event);

  static std::string formatTry(
    const std::string &text,
    const wxString &format
  );
};

}

#endif // TRANSMITRON_WIDGETS_EDIT_HPP
