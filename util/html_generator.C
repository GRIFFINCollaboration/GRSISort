#include "THtml.h"

void html_generator(){

   //Order matters!!!!
   gSystem->Load("libTreePlayer");
   gSystem->Load("$GRSISYS/libraries/libMidasFormat.so");
   gSystem->Load("$GRSISYS/libraries/libGRSIFormat.so");
 //  gSystem->Load("$GRSISYS/libraries/libBuilder.so");
 //  gSystem->Load("$GRSISYS/libraries/libGRSIRootIO.so");
   gSystem->Load("$GRSISYS/libraries/libDataParser.so");
   gSystem->Load("$GRSISYS/libraries/libNucleus.so");
   gSystem->Load("$GRSISYS/libraries/libKinematics.so");
   gSystem->Load("$GRSISYS/libraries/libSharc.so");
   gSystem->Load("$GRSISYS/libraries/libGRSIint.so");
   gSystem->Load("libProof");

   THtml html;
   html.SetProductName("GRSISort");
   html.SetInputDir("$(GRSISYS)/libraries/");
   html.SetIncludePath("$(GRSISYS)/include/");
   html.AddMacroPath("$(GRSISYS)/util");
   html.SetOutputDir("$(GRSISYS)/htmldoc");
   html.SetSourceDir("$(GRSISYS)/libraries/*/*:$(GRSISYS)/include/");
   html.MakeAll();
}
