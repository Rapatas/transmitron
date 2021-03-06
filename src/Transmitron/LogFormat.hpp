#ifndef TRANSMITRON_LOGFORMAT_HPP
#define TRANSMITRON_LOGFORMAT_HPP

#include <wx/log.h>

namespace Transmitron
{

class LogFormat :
  public wxLogFormatter
{
  wxString Format(
    wxLogLevel /* level */,
    const wxString& msg,
    const wxLogRecordInfo& info
  ) const override {
    constexpr uint8_t MinColorChannel = 100;
    const uint8_t color =
      ((info.threadId % (256U - MinColorChannel)) + MinColorChannel) & 0xFF;
    return wxString::Format(
      "[\033[38;5;%um%#x\033[0m] [%s] : %s",
      color,
      (unsigned)info.threadId,
      info.component,
      msg.c_str().operator const char *()
    );
  }
};

}

#endif // TRANSMITRON_LOGFORMAT_HPP
