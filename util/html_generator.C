#include "THtml.h"

void html_generator(){
   THtml h;
   h.SetInputDir("$(GRSISYS)");
   h.MakeAll();
}
