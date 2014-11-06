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
#include <utility>

using namespace TGRSIFunctions;

class TGRSIFit : public TObject {
 public: 
   TGRSIFit(){};
   ~TGRSIFit(){};

 public:   
   Bool_t FitPhotoPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, Bool_t verbosity = false); 
 //  void FitPeak(Int_t limit1, Int_t limit2, std::initializer_list<double> centroid);
   //void FitPeak(Int_t limit1, Int_t limit2, ...);

 private:   
   static Double_t fitFunction(Double_t *dim, Double_t *par);
   std::vector<Double_t> centroids;

//TEMPLATES
 public:
   int FitPeak(Int_t limit1, Int_t limit2, Int_t centroid){FitPeak(limit1,limit2,(double)(centroid));}
   int FitPeak(Int_t limit1, Int_t limit2, Double_t centroid); // termination version
   template<int, int, typename First, typename... Rest>
      int FitPeak(const int& limit1, const int& limit2, const First& firstcent, const Rest&... rest)
      {
         centroids.push_back(firstcent);
         FitPeak(limit1,limit2,rest...); // note: arg1 does not appear here!
      }


  ClassDef(TGRSIFit,1);

};
