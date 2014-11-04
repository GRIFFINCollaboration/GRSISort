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

 private:
   TFile *effFile;

  ClassDef(TECal,1);

};
