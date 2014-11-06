#include <TObject.h>
#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
#include <vector>
#include <utility>
#include "TROOT.h"
#include "TFile.h"
#include "TChannel.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TNucleus.h"
//#include "../include/TNucleus.h"

#include "../include/TGRSITransition.h"


class TECal : public TObject {
 public: 
   TECal();
   TECal(const char *filename);
   ~TECal(); 

   void OpenFile(const char * filename);
 public:
   void CalibrateEfficiency();
   void CalibrateEnergy();
   Bool_t FitEnergyCal();
   void AddEnergyGraph(TGraphErrors *graph, Int_t channum, const char *nucname);
   void AddEnergyGraph(Int_t channum, const char *nucname, TGraphErrors *graph);
   void AddEfficiencyGraph(Int_t channum, const char * nucname, TGraphErrors *graph);
   void AutoFitSource();

   Bool_t Write();

 private:
   TFile *effFile = NULL;
   std::map<Int_t,std::map<std::string,TGraphErrors*>> fenergyMap;
   std::map<Int_t,std::map<std::string,TGraphErrors*>> fefficiencyMap;

  ClassDef(TECal,1);

};
