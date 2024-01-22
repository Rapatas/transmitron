#include "messages-18x18.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2cMessages18x18()
{
  constexpr size_t Length = 274;
  static wxMemoryInputStream mistream("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\22\10\6\0\0\0V\316\216W\0\0\0\6bKGD\0\377\0\377\0\377\240\275\247\223\0\0\0\11pHYs\0\0\3\357\0\0\3\357\1\347?\364\24\0\0\0\7tIME\7\345\2\15\7)\6\252\235\207w\0\0\0\237IDAT8\313\355\320\261\11\302`\20\206\341G\11\16`\260\261q\4\7p\32\327q\211\214\21\320\306B\253l\220\322V\254\15\242h\23!\376$!1\205\215/|\315\35\367rw|\222\341\3311Yup\24\210bLt\343\206K]\343\200\2\327\216)\312\31\20UD\33\314\312\265\2730\3029\24\255\260\324\237yy\336\361]\330\365xr\230-\214K\321\335\367<\252\242\220\4\213\206$u\3Q\203h\217SKo\35\26\2336\212[N\231\266\335\231\16xv\332\266Qo\376\242\37\210\362\1\216\34^\356\30F\340T\272|#\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(mistream, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}
