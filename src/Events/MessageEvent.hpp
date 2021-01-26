#ifndef MESSAGEEVENT_H
#define MESSAGEEVENT_H

#include <wx/event.h>
#include <wx/dataview.h>

class MessageEvent;
wxDECLARE_EVENT(EVT_MESSAGE_RECEIVED, MessageEvent);

class MessageEvent : public wxCommandEvent
{
public:
  MessageEvent(wxEventType commandType, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  MessageEvent(const MessageEvent& event) :
    wxCommandEvent(event)
  {
    this->setMessage(event.getMessage());
  }

  wxEvent* Clone() const { return new MessageEvent(*this); }

  wxDataViewItem getMessage() const { return mMessage; }
  void setMessage(const wxDataViewItem &message) { mMessage = message; }

private:
  wxDataViewItem mMessage;
};

typedef void (wxEvtHandler::*MessageEventFunction)(MessageEvent &);
#define \
  MessageEventHandler(func) \
  wxEVENT_HANDLER_CAST(MessageEventFunction, func)

#endif // MESSAGEEVENT_H
