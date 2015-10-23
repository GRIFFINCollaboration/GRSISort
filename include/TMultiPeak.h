#ifndef TMULTIPEAK_H
#define TMULTIPEAK_H

#include "TGRSIFunctions.h"
#include "TGRSIFit.h"
#include "TF1.h"
#include "TPeak.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include <string>
#include <algorithm>
#include <vector>
#include <stdarg.h>
#include "TGraph.h"


////////////////////////////////////////////////////////////////
//                                                            //
// TMultiPeak                                                 //
//                                                            //
// This Class is used to represent fitted data that is        //
// Gaussian like in nature (ie centroid and area).            //
//                                                            //
////////////////////////////////////////////////////////////////

class TMultiPeak : public TGRSIFit {
 public: 
   //ctors and dtors
   virtual ~TMultiPeak();
   //TMultiPeak(int n, ...);
  // TMultiPeak(double xlow, double xhigh, int n, ...);
   TMultiPeak(Double_t xlow, Double_t xhigh, const std::vector<Double_t> &centroids, Option_t *type = "gsc");
   TMultiPeak(const TMultiPeak &copy);
   TMultiPeak(); //I might make it so if you call this ctor, the TPeak yells at you since it's a fairly useless call anyway
   
 protected:
   void InitNames();

 public:
   Bool_t Fit(TH1* fithist,Option_t *opt = "");
   bool InitParams(TH1* hist);
   void SortPeaks(Bool_t (*SortFunction)(const TPeak* ,const TPeak* ) = TPeak::CompareEnergy);
   TPeak* GetPeak(UInt_t idx);
   void DrawPeaks() const;
   TF1* Background() const { return fBackground; }

   static void SetLogLikelihoodFlag(bool flag){ fLogLikelihoodFlag = flag; }
   static bool GetLogLikelihoodFlag() { return fLogLikelihoodFlag; }
  
   virtual void Copy(TObject &obj) const;
   virtual void Print(Option_t *opt = "") const;
   virtual void Clear(Option_t *opt = "");
   virtual const char* PrintString(Option_t *opt = "") const;

 private:
   static bool fLogLikelihoodFlag; //!
   std::vector<TPeak*> fPeakVec;
   TF1* fBackground;

   static Double_t MultiPhotoPeakBG(Double_t *dim, Double_t *par); 
   static Double_t MultiStepBG(Double_t *dim, Double_t *par); 
   static Double_t SinglePeakBG(Double_t *dim, Double_t *par); 

  ClassDef(TMultiPeak,2);

};

#endif
