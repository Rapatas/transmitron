#include "not-pinned-18x18.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2c_not_pinned_18x18()
{
  constexpr size_t Length = 400;
  static wxMemoryInputStream sm("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\22\10\6\0\0\0V\316\216W\0\0\0\6bKGD\0\377\0\377\0\377\240\275\247\223\0\0\0\11pHYs\0\0\13\23\0\0\13\23\1\0\232\234\30\0\0\0\7tIME\7\345\1\21\11\4$\10&#\201\0\0\1\35IDAT8\313\245\322O+Dq\24\306\361\317\230\301\354\330\261\261\264\263\306B\221d\241\31\312\202\374y\5^\206\215\225w\300;P\32I\222\5\26\344OB\310N\241(+f+l\216\232n\227\2311\317\346\376\3569\347\367\355\271\317=Y\265\253\31\273x\307\235\0064\206W|a<\331l\252\3\324\2135\24P\302\344\177\35]c%\316\305pV\254\7\320\212\23\234\"\213L\324\247\2V\263\263c\34\4$\251\321\237\3142\211F\27\372\321\215\27,\304`_<\277b.\217\245x\237\305C%d\22e\334b\31\373\330Kq\222\307e\314-b\4\271\312\201M\274\241%q1\223\200\354\340\350\257,\262\330\3023\332R\372y\\\3410\316UU\302#:\22\365\215\370{\251J[\310\11<\341,\341,\207\325z@\5\364\340\0367\350\214\372y\212\313_5\203O\14F\310\333\221Y6\366\251\246\345+\340\3\363\211\372zdVF{\255N\206~\371\374=LW\203\14\207\2239\15\352\2\3\215B\276\1\214\10<\226\272\32\311#\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(sm, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}
