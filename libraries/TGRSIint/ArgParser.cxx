#include "ArgParser.h"

std::ostream& operator<<(std::ostream& out, const ArgParser& val)
{
   val.print(out);
   return out;
}
