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

class TGRSIFitter : public TObject {
 public: 
   TGRSIFitter(){};
   ~TGRSIFitter(){};

 public:   
   Bool_t FitPhotoPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, Bool_t verbosity = false); 
 //  void FitPeak(Int_t limit1, Int_t limit2, std::initializer_list<double> centroid);
   //void FitPeak(Int_t limit1, Int_t limit2, ...);


   //This might be introduced if we need a true variadic version
   int func(Int_t limit1, Int_t limit2, Int_t centroid){}
   int func(Int_t limit1, Int_t limit2, Double_t centroid) {} // termination version
   template<int, int, typename First, typename... Rest>
      int func(const int& limit1, const int& limit2, const First& firstcent, const Rest&... rest)
      {
         static std::vector<int> vlist;
            // process( arg1 );
                 func(limit1,limit2,rest...); // note: arg1 does not appear here!
      }

   //Provide a vector filling function

 private:   
  static Double_t fitFunction(Double_t *dim, Double_t *par);

  ClassDef(TGRSIFitter,1);

};
