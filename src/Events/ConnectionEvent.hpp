#ifndef CONNECTIONEVENT_H
#define CONNECTIONEVENT_H

#include "Connection.hpp"
#include <wx/event.h>

class ConnectionEvent;
wxDECLARE_EVENT(EVT_CONNECTION, ConnectionEvent);

class ConnectionEvent : public wxCommandEvent
{
public:
  ConnectionEvent(wxEventType commandType = EVT_CONNECTION, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  ConnectionEvent(const ConnectionEvent& event) :
    wxCommandEvent(event)
  {
    this->setConnection(event.getConnection());
  }

  wxEvent* Clone() const { return new ConnectionEvent(*this); }

  Connection getConnection() const { return mConnection; }
  void setConnection(const Connection &connection) { mConnection = connection; }

private:
  Connection mConnection;
};

typedef void (wxEvtHandler::*ConnectionEventFunction)(ConnectionEvent &);
#define \
  ConnectionEventHandler(func) \
  wxEVENT_HANDLER_CAST(ConnectionEventFunction, func)

#endif // CONNECTIONEVENT_H
