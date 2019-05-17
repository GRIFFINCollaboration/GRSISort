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

      std::string inclpath = "$(GRSISYS):$(GRSISYS)/include";
      std::stringstream totpath;
      totpath<<inclpath<<fpath.str();
      std::string incldirs = totpath.str(); 
      GetHtml()->SetSourceDir(incldirs.c_str());
		std::cout<<"set source dir to "<<incldirs.c_str()<<std::endl;
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

   void AddLibraryPath(std::string newpath)
   {   
       fpath<<":$(GRSISYS)/"<<newpath;
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
   html.AddSourcePath("TAnalysis");
   html.AddSourcePath("TDataParser");
   html.AddSourcePath("TFormat"); 
   html.AddSourcePath("TGRSIint");
   html.AddSourcePath("TGRSIProof");
   html.AddSourcePath("TGUI");
   html.AddSourcePath("THistogramming");
   html.AddSourcePath("TLoops");
   html.AddSourcePath("TRawFile");
   html.AddSourcePath("TAnalysis/SourceData");
   html.AddSourcePath("TAnalysis/SRIMData");
   html.AddSourcePath("TAnalysis/TBetaDecay");
   html.AddSourcePath("TAnalysis/TBgo");
   html.AddSourcePath("TAnalysis/TCal");
   html.AddSourcePath("TAnalysis/TDetector");
   html.AddSourcePath("TAnalysis/TGRSIFit");
   html.AddSourcePath("TAnalysis/TKinematics");
   html.AddSourcePath("TAnalysis/TNucleus");
   html.AddSourcePath("TAnalysis/TPeakFitting");
   html.AddSourcePath("TAnalysis/TPulseAnalyzer");
   html.AddSourcePath("TAnalysis/TReaction");
   html.AddSourcePath("TAnalysis/TSRIM");
   html.AddSourcePath("TAnalysis/TSuppressed");

	html.AddLibraryPath("GRSIData");
	html.AddLibraryPath("GRSIData/include");
	html.AddLibraryPath("GRSIData/libraries");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis");
	html.AddLibraryPath("GRSIData/libraries/TGRSIDataParser");
	html.AddLibraryPath("GRSIData/libraries/TGRSIFormat");
	html.AddLibraryPath("GRSIData/libraries/TMidas");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TAngularCorrelation");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TCSM");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TDescant");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TEmma");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TGenericDetector");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TGriffin");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TGRSIDetector");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TLaBr");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TPaces");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TRF");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TS3");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TSceptar");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TSharc");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TSiLi");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TTAC");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TTigress");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TTip");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TTriFoil");
	html.AddLibraryPath("GRSIData/libraries/TGRSIAnalysis/TZeroDegree");

	html.AddLibraryPath("ILLData");
	html.AddLibraryPath("ILLData/include");
	html.AddLibraryPath("ILLData/libraries");
	html.AddLibraryPath("ILLData/libraries/TILLAnalysis");
	html.AddLibraryPath("ILLData/libraries/TILLDataParser");
	html.AddLibraryPath("ILLData/libraries/TILLFormat");
	html.AddLibraryPath("ILLData/libraries/TLst");
	html.AddLibraryPath("ILLData/libraries/TILLAnalysis/TFipps");
	html.AddLibraryPath("ILLData/libraries/TILLAnalysis/TILLDetector");

	html.AddLibraryPath("iThembaData");
	html.AddLibraryPath("iThembaData/include");
	html.AddLibraryPath("iThembaData/libraries");
	html.AddLibraryPath("iThembaData/libraries/TTdr");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis");
	html.AddLibraryPath("iThembaData/libraries/TTdrDataParser");
	html.AddLibraryPath("iThembaData/libraries/TTdrFormat");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis/TTdrClover");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis/TTdrDetector");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis/TTdrPlastic");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis/TTdrSiLi");
	html.AddLibraryPath("iThembaData/libraries/TTdrAnalysis/TTdrTigress");

   html.RunAll();
}
