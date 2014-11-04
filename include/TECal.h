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
#include "TROOT.h"
#include "TFile.h"
#include "TChannel.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
//#include "../include/TNucleus.h"

#include "../include/TGRSITransition.h"


class TECal : public TObject {
 public: 
   TECal();
   TECal(const char *);
   ~TECal(); 

 public:
   void CalibrateEfficiency();
   void CalibrateEnergy();
   Bool_t FitEnergyCal();
   void AddEnergyGraph(TGraphErrors *graph, Int_t channum);
   void AddEnergyGraph(Int_t channum, TGraphErrors *graph);

 private:
   TFile *effFile;

   static std::map<Int_t,TGraphErrors*> fmgenergy;         //Map of channel number to energy multigraph
   static std::map<Int_t,TGraphErrors*> fmgefficiency;     //Map of channel number to efficiency multigraph

  ClassDef(TECal,1);

};
