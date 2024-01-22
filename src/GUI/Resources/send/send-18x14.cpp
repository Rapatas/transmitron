#include "send-18x14.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2cSend18x14()
{
  constexpr size_t Length = 321;
  static wxMemoryInputStream mistream("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\16\10\4\0\0\0\210\323\204<\0\0\0\2bKGD\0\377\207\217\314\277\0\0\0\11pHYs\0\0\13\23\0\0\13\23\1\0\232\234\30\0\0\0\7tIME\7\345\2\15\7/\31q\317-\4\0\0\0\322IDAT(\317u\321\261.\4Q\24\6\340oY;\211Y\211\214\331b%\204R\357\0114\364\22\225\25\321\350\24\274\202\27\21\255B\241Sx\0\275l\224\250HV\245\220\25G1\273v2;{\222\333}\271\347\277\377e_\10a\340\301\236\31\223\373\34\261\20\372v$\323\250\345\272\204Bxt\254[e=\303\12\13\317.\245e\264\341e\12\205\360\345Ls\302n\375\324\262\360dW\253\0016mY\220H\245\332\243\223Z\325\265f\321Mq\335\267\206\266L.\227\313\254\310t4\315\231G\24\353\356f,\373\365\352\244 \353\336jI\337\205\345q\354\303\232\330\357\216d\223\227%\2252\207>\234W\253\354\30\2242\334;-w3\236\203\177re\333R\335\367\376\1\242~x\273v$?S\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(mistream, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}