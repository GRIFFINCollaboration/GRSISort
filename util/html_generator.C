#include "THtml.h"

void html_generator(){

   gSystem->Load("libTreePlayer");
   gSystem->Load("$GRSISYS/libraries/libMidasFormat.so");
   gSystem->Load("$GRSISYS/libraries/libGRSIFormat.so");
 //  gSystem->Load("$GRSISYS/libraries/libBuilder.so");
 //  gSystem->Load("$GRSISYS/libraries/libGRSILoop.so");
 //  gSystem->Load("$GRSISYS/libraries/libTGRSIint.so");
   gSystem->Load("$GRSISYS/libraries/libDataParser.so");
 //  gSystem->Load("$GRSISYS/libraries/libGRSIRootIO.so");
   gSystem->Load("$GRSISYS/libraries/libNucleus.so");
   gSystem->Load("$GRSISYS/libraries/libKinematics.so");
   gSystem->Load("$GRSISYS/libraries/libSharc.so");


   THtml h;
   h.SetInputDir("$(GRSISYS)");
   h.SetOutputDir("$(GRSISYS)/htmldoc");
   h.MakeClass("TNucleus");
   h.MakeAll();
}
