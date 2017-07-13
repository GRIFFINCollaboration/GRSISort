/*
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
   html.SetInputDir("$(GRSISYS)");
 //  html.SetIncludePath("$(GRSISYS)/include/");
   html.AddMacroPath("$(GRSISYS)/util");
   html.SetOutputDir("$(GRSISYS)/htmldoc");
 //  html.SetSourceDir("$(GRSISYS)/libraries/TGint:$(GRSISYS)/include/:$(GRSISYS)/src");
   html.MakeAll();
}
*/



#include "THtml.h"
#include <string>
#include <sstream>

class THtmlCreator: public TObject {
public:
   THtmlCreator(): fHtml(0)
   { 
      printf("This class generates HTML documentation\n"); 
   }
   ~THtmlCreator() { if (fHtml) delete fHtml; }

   // inline methods can have their documentation in front 
   // of the declaration. DontDoMuch is so short - where
   // else would one put it?
   void DontDoMuch() {}

   void Convert() 
   {
      // Create a "beautified" version of this source file.
      // It will be called htmldoc/htmlex.C.html.
      GetHtml()->SetMacroPath("$(ROOTSYS)/macros/");
      GetHtml()->SetSourceDir("$(GRSISYS)/util:$(GRSISYS)/scripts");
      GetHtml()->Convert("html_generator.C", "Generates HTML Documentation", "./htmldoc/", "./");
      GetHtml()->Convert("DroppedData.C","Calculates Dropped Events","./htmldoc/","./");
   }

   void MakeDocForAllClasses() 
   {
      // Creates the documentation pages for all classes that have
      // been loaded, and that are accessible from "./".
      // If evenForROOT is set, we'll try to document ROOT's classes, 
      // too - you will end up with a copy of ROOT's class reference.
      // The documentation will end up in the subdirectory htmldoc/.

      std::string inclpath = "$(GRSISYS)/grsisort/";
      std::stringstream totpath;
      totpath<<inclpath<<fpath.str();
      std::string incldirs = totpath.str(); 
      GetHtml()->SetSourceDir(incldirs.c_str());
      GetHtml()->SetOutputDir("$GRSISYS/htmldoc");
      GetHtml()->MakeAll();
   }

   void RunAll() {
      // Do everything we can.
      MakeDocForAllClasses();
      Convert();
   }

   void SetProductName(const char* name)
   {
      //Sets Program name in HTML documentation
      GetHtml()->SetProductName(name);
   }

   void SetEtcDir(TString name)
   {
      gSystem->PrependPathName(getenv("GRSISYS"), name);
      GetHtml()->SetEtcDir(name);
      std::cout<<"ETC "<<GetHtml()->GetEtcDir()<<std::endl;
   }

   void AddSourcePath(std::string newpath)
   {   
       fpath<<":$(GRSISYS)/libraries/"<<newpath;
   }

   void AddRootSourcePath(){
      fpath<<":$(ROOTSYS)/";
   }

protected:
   THtml* GetHtml() 
   {
      // Return out THtml object, and create it if it doesn't exist.
      if (!fHtml) fHtml = new THtml();
      return fHtml;
   }

private:
   THtml* fHtml; // our local THtml instance.
   std::stringstream fpath;
   ClassDef(THtmlCreator, 0); // The Html Creation Class. 
};

void html_generator() {

   gErrorIgnoreLevel=kError;
   
   gSystem->ListLibraries();

   THtmlCreator html;
   html.SetProductName("GRSISort");
   html.AddRootSourcePath();
   html.SetEtcDir("etc/html");
//We must do this because of our naming convention of GRSISort directories
   html.AddSourcePath("GROOT");
   html.AddSourcePath("TGRSIFormat"); 
   html.AddSourcePath("TDataParser");
   html.AddSourcePath("TGint");
   html.AddSourcePath("TGRSILoop");
   html.AddSourcePath("TGRSIAnalysis");
   html.AddSourcePath("TMidas");
   html.AddSourcePath("TGRSIint");
   html.AddSourcePath("TGRSIRootIO");
   html.AddSourcePath("TGRSIAnalysis/TAnalysisTreeBuilder");
   html.AddSourcePath("TGRSIAnalysis/TKinematics");
   html.AddSourcePath("TGRSIAnalysis/TNucleus");
   html.AddSourcePath("TGRSIAnalysis/TSharc");
   html.AddSourcePath("TGRSIAnalysis/TCSM");
   html.AddSourcePath("TGRSIAnalysis/TGriffin");
   html.AddSourcePath("TGRSIAnalysis/TGRSIFit");
   html.AddSourcePath("TGRSIAnalysis/TPaces");
   html.AddSourcePath("TGRSIAnalysis/TSceptar");
   html.AddSourcePath("TGRSIAnalysis/TSharc");
   html.AddSourcePath("TGRSIAnalysis/TSRIM");
   html.AddSourcePath("TGRSIAnalysis/TGRSIDetector");
   html.AddSourcePath("TGRSIAnalysis/TTigress");
   html.AddSourcePath("TGRSIAnalysis/TBetaDecay");
   html.AddSourcePath("TGRSIAnalysis/TCal");

   html.RunAll();
}
