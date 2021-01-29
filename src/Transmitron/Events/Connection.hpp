#ifndef TRANSMITRON_EVENTS_CONNECTION_HPP
#define TRANSMITRON_EVENTS_CONNECTION_HPP

#include <wx/event.h>
#include "Transmitron/Types/Connection.hpp"

namespace Transmitron::Events
{

class Connection;
wxDECLARE_EVENT(CONNECTION, Connection);

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
  Types::Connection getConnection() const { return mConnection; }
  void setConnection(const Types::Connection &connection) { mConnection = connection; }

private:
  Types::Connection mConnection;
};

typedef void (wxEvtHandler::*ConnectionEventFunction)(Connection &);
#define \
  ConnectionEventHandler(func) \
  wxEVENT_HANDLER_CAST(ConnectionEventFunction, func)

}

#endif // TRANSMITRON_EVENTS_CONNECTION_HPP
