#include "THtml.h"

void html_generator(){
   gSystem->Load("/opt/root/lib/libXMLIO.so");
   gSystem->Load("/opt/root/lib/libXMLParser.so");

   gSystem->Load("libTreePlayer");
   gSystem->Load("$(GRSISYS)/libraries/libMidasFormat.so");
   gSystem->Load("$(GRSISYS)libraries/libGRSIFormat.so");


   THtml h;
   h.SetInputDir("$(GRSISYS)");
   h.SetOutputDir("$(GRSISYS)/htmldoc");
   h.MakeAll();
}
