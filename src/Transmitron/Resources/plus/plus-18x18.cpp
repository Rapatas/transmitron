#include "plus-18x18.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2cPlus18x18()
{
  constexpr  size_t Length = 166;
  static wxMemoryInputStream sm("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\22\10\6\0\0\0V\316\216W\0\0\0\6bKGD\0\377\0\377\0\377\240\275\247\223\0\0\0\11pHYs\0\0\15\327\0\0\15\327\1B(\233x\0\0\0\7tIME\7\345\1\30\0178\32\3031t\355\0\0\0003IDAT8\313c\374\377\377?\3.\300\310\310\210\"\371\377\377\177F\\j\231\30\250\4F\15\242\243A\214\14\14\14\377G\303\10\177\30\215f\221\221l\20\0\355+\17\33\24\274Ap\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(sm, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}
