#ifndef TRANSMITRON_LOGFORMAT_HPP
#define TRANSMITRON_LOGFORMAT_HPP

#include <wx/log.h>

namespace Transmitron
{

class LogFormat :
  public wxLogFormatter
{
  virtual wxString Format(
    wxLogLevel level,
    const wxString& msg,
    const wxLogRecordInfo& info
  ) const {
    uint8_t minColor = 100;
    uint8_t color = (info.threadId % (256 - minColor)) + minColor;
    return wxString::Format(
      "[\e[38;5;%um%#x\e[0m] [%s] : %s",
      color,
      (unsigned)info.threadId,
      info.component,
      msg.c_str().operator const char *()
    );
  }
};

}

#endif // TRANSMITRON_LOGFORMAT_HPP
