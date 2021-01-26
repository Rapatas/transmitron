#ifndef EDIT_H
#define EDIT_H

#include <wx/stc/stc.h>
#include <wx/wx.h>

#include "MQTT/Client.hpp"

class Edit :
  public wxPanel
{
public:

  explicit Edit(wxWindow* parent = nullptr);
  virtual ~Edit();

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

  bool mReadOnly = false;

  wxBoxSizer *mTop;
  wxBoxSizer *mVsizer;
  wxBoxSizer *mBottom;

  wxTextCtrl *mTopic;

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

#endif // EDIT_H
