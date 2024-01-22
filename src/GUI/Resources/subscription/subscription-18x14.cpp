#include "subscription-18x14.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2cSubscription18x14()
{
  constexpr size_t Length = 305;
  static wxMemoryInputStream mistream("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\16\10\6\0\0\0\"\332L\267\0\0\0\6bKGD\0\377\0\377\0\377\240\275\247\223\0\0\0\11pHYs\0\0\3\261\0\0\3\261\1\365\203\355I\0\0\0\7tIME\7\345\2\15\10,\5E\277e\265\0\0\0\276IDAT(\317\235\3221N\2A\24\306\361\37\13\215\24$\320\31\16\240\241\304{\320jae\247'\240\341*\326z\36\270\2\1Kc\203\311\352R\370\226LH\\v\366K\276\314\233\314\373\376o2\31\3764\303!\352\25\252\13^E\357!\262'\275\3439\352\252\245\341\5o)\350\7Wx\310\0\335c\30YE\200\12\374\342F{\335\6\244HA#|'\3736*\"3JA_\261N2@\3434{~\203y\6\350\256\351\360#\343\261\367\377A\26\31\220\332\213s\310\0e\7P\31\331\223\326\35 \2657\22\332#\246\272i\13\275\206\206k\274\326?\27}<a\327e\332\22\237\341eS\343\21V\35g\360}B\217\352\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(mistream, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}