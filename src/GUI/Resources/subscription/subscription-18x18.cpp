#include "subscription-18x18.hpp"
#include <wx/bitmap.h>
#include <wx/image.h>
#include <wx/mstream.h>

const wxBitmap *bin2cSubscription18x18()
{
  constexpr size_t Length = 379;
  static wxMemoryInputStream mistream("\211PNG\15\12\32\12\0\0\0\15IHDR\0\0\0\22\0\0\0\22\10\6\0\0\0V\316\216W\0\0\0\6bKGD\0\377\0\377\0\377\240\275\247\223\0\0\0\11pHYs\0\0\3\261\0\0\3\261\1\365\203\355I\0\0\0\7tIME\7\345\2\15\10+0\\M7Q\0\0\1\10IDAT8\313\235\324\275J\3A\24\305\361\237k\32\5\3\261\223\264\1\305R+A_!`\245\205\225\235\202\251\323\344U,\203V\276\205/\240o ~\244\22I\23!\32\233\273aX\223M6\7\16;w\347\334\377\356\16;C\271\232x\0137\313\202k\205\272\201!\306Q?\3401rG8\215\3735l\341s\36t\202\343\250[\30\4t\34\343V\314\235D\366\2372\274\240\23u?\202\263\334\217\314M\364d)h\37\243\30\367J \271{\221\35E\357T\367\270\212\361dI\3035\356R\320\0176p^\1t\206\315\350\235~_\206_\354Z^{\1\311RP\35\337\305\205[\240,z\352)h\30\327\355\12\240F\332[|\203\203\12\240\303\262\311A\205\305\376\230\7iW\200\344n\27!\265\330\6UA\343\350\235\352i\5H\356g\11\355b\3211Q\242\327Y\307H\252\35\334\346\177.\326q\211\367U\236\326\305W\270[\26\374\3\273t\177h\223\2#w\0\0\0\0IEND\256B`\202", Length);
  static const wxBitmap *bitmap = new wxBitmap(wxImage(mistream, wxBITMAP_TYPE_ANY), -1);
  return bitmap;
}