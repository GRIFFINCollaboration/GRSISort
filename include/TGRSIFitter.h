#include "TGRSIFunctions.h"
#include "TObject.h"
#include "TH1.h"
#include "TF1.h"
#include "TList.h"
#include "TFitResult.h"
#include "TFitResultPtr.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"
#include "TMatrixTSym.h"
#include "TMath.h"
#include "TCanvas.h"
#include <map>
#include <vector>
#include "TROOT.h"

using namespace TGRSIFunctions;

class TGRSIFitter : public TObject {
 public: 
   TGRSIFitter(){};
   ~TGRSIFitter(){};

 public:   
   Bool_t FitPhotoPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, Bool_t verbosity = false);

 private:   
  static Double_t fitFunction(Double_t *dim, Double_t *par);

  ClassDef(TGRSIFitter,1);

};
