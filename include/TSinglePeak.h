#ifndef TSINGLEPEAK_H
#define TSINGLEPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <string>
#include <algorithm>
#include <vector>
#include <cstdarg>

#include "TObject.h"
#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TGraph.h"
#include "TMath.h"
#include "TVirtualFitter.h"
#include "TPeakFitter.h"

/////////////////////////////////////////////////////////////////
///
/// \class TSinglePeak
///
///  This class is used to fit things that resemble "peaks" in data
///
/////////////////////////////////////////////////////////////////
class TPeakFitter;

class TSinglePeak : public TObject {
public:
   friend class TPeakFitter;
   // ctors and dtors
   ~TSinglePeak() override{};
   TSinglePeak();

   virtual void InitParNames() {}
   virtual void InitializeParameters(TH1* hist = nullptr) {}
   bool IsBackgroundParameter(const Int_t& par) const;
   bool IsPeakParameter(const Int_t& par) const;
   void SetListOfBGPar(std::vector<bool> list_of_bg_par) { fListOfBGPars = list_of_bg_par; }
   Int_t GetNParameters() const;

   void SetArea(const Double_t& area) { fArea = area; }
   void SetAreaErr(const Double_t& area_err) { fAreaErr = area_err; }

   Double_t Area() const { return fArea; }
   Double_t AreaErr() const { return fAreaErr; }

   virtual Double_t Centroid() const = 0;
   virtual Double_t CentroidErr() const = 0;

   virtual void Print(Option_t * opt = "" ) const override;
   virtual void Draw(Option_t * opt = "") override { fFitFunction->Draw(opt);}
   virtual void DrawBackground(Option_t * opt = "") { if(fGlobalBackground) fGlobalBackground->Draw("same");}

   TF1* GetFitFunction() { return fFitFunction; }
   void SetGlobalBackground(TF1* bg) { fGlobalBackground = bg; }

protected:
   virtual Double_t FitFunction(Double_t*, Double_t *) {return 0.0;}

protected:
   TF1* fFitFunction{nullptr};
   std::vector<bool> fListOfBGPars;
   Double_t fArea{-0.1};
   Double_t fAreaErr{0.0};

private:
   TF1* fGlobalBackground{nullptr};

public:
   /// \cond CLASSIMP
   ClassDefOverride(TSinglePeak, 1);
   /// \endcond
};
/*! @} */
#endif
