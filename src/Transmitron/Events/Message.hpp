#ifndef TRANSMITRON_EVENTS_MESSAGEEVENT_HPP
#define TRANSMITRON_EVENTS_MESSAGEEVENT_HPP

#include <wx/event.h>
#include <wx/dataview.h>

namespace Transmitron::Events
{

class Message;
wxDECLARE_EVENT(MESSAGE_RECEIVED, Message);

class Message : public wxCommandEvent
{
public:
  Message(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Message(const Message& event) :
    wxCommandEvent(event)
  {
    this->setMessage(event.getMessage());
  }

  wxEvent* Clone() const { return new Message(*this); }
  wxDataViewItem getMessage() const { return mMessage; }
  void setMessage(const wxDataViewItem &message) { mMessage = message; }

private:
  wxDataViewItem mMessage;
};

typedef void (wxEvtHandler::*MessageFunction)(Message &);
#define \
  MessageHandler(func) \
  wxEVENT_HANDLER_CAST(MessageFunction, func)

}

#endif // TRANSMITRON_EVENTS_MESSAGEEVENT_HPP
