#ifndef TRANSMITRON_EVENTS_CONNECTION_HPP
#define TRANSMITRON_EVENTS_CONNECTION_HPP

#include <wx/event.h>
#include "Transmitron/Types/Connection.hpp"

namespace Transmitron::Events
{

class Connection;
wxDECLARE_EVENT(CONNECTION, Connection);
wxDECLARE_EVENT(CONNECTED, Connection);
wxDECLARE_EVENT(DISCONNECTED, Connection);

class Connection : public wxCommandEvent
{
public:
  Connection(wxEventType commandType = CONNECTION, int id = 0) :
    wxCommandEvent(commandType, id)
  {}

  Connection(const Connection& event) :
    wxCommandEvent(event)
  {
    this->setConnection(event.getConnection());
  }

  wxEvent* Clone() const { return new Connection(*this); }
  wxDataViewItem getConnection() const { return mConnection; }
  void setConnection(wxDataViewItem connection) { mConnection = connection; }

private:
  wxDataViewItem mConnection;
};

typedef void (wxEvtHandler::*ConnectionEventFunction)(Connection &);
#define \
  ConnectionEventHandler(func) \
  wxEVENT_HANDLER_CAST(ConnectionEventFunction, func)

}

#endif // TRANSMITRON_EVENTS_CONNECTION_HPP
