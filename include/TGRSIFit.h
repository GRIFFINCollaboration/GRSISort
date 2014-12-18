#ifndef __TGRSIFIT_H
#define __TGRSIFIT_H

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
#include "TNamed.h"
#include <map>
#include <vector>
#include "TROOT.h"
#include <utility>

using namespace TGRSIFunctions;

//typedef std::tuple <int, double, double> CPIs;

class TGRSIFit : public TNamed {
  protected: 
   TGRSIFit(){};
   virtual ~TGRSIFit(){};

 public:
 //  void FitPeak(Int_t limit1, Int_t limit2, std::initializer_list<double> centroid);
   //void FitPeak(Int_t limit1, Int_t limit2, ...);
//TFitResultPtr FitPhotoPeak(Double_t *par, TH1 *h, Float_t &area, Float_t &darea, Double_t *energy, Bool_t verbosity);

   virtual Double_t Fit(Option_t *opt = "") = 0;
   virtual Bool_t SetHist(void* hist) = 0;

//   int FitPeak(Int_t limit1, Int_t limit2, Int_t cent){FitPeak(limit1,limit2,(double)(cent));}
//   int FitPeak(Int_t limit1, Int_t limit2, Double_t cent); // termination version
/*   template<int, int, typename First, typename... Rest>
      int FitPeak(const int& limit1, const int& limit2, const First& firstcent, const Rest&... rest)
      {
 //        centroid.push_back(std::make_pair(firstcent,0));
         FitPeak(limit1,limit2,rest...); // note: arg1 does not appear here!
      }
*/   
   ClassDef(TGRSIFit,0);
};

#endif
