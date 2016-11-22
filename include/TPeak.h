#ifndef TPEAK_H
#define TPEAK_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include <string>
#include <algorithm>

#include "TF1.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TGraph.h"

#include "TGRSIFunctions.h"
#include "TGRSIFit.h"

using namespace TGRSIFunctions;

/////////////////////////////////////////////////////////////////
///
/// \class TPeak
///
/// This Class is used to represent fitted data that is
/// Gaussian like in nature (ie centroid and area).
///
/////////////////////////////////////////////////////////////////

class TPeak : public TGRSIFit {
   friend class TMultiPeak;
 public: 
   //ctors and dtors
   virtual ~TPeak();
   TPeak(const TPeak& copy);
   TPeak(Double_t cent, Double_t xlow, Double_t xhigh);
   TPeak(Double_t cent, Double_t xlow, Double_t xhigh, TF1* background);
   TPeak(); //I might make it so if you call this ctor, the TPeak yells at you since it's a fairly useless call anyway
   
 protected:
   void InitNames();

 public:
   virtual void Copy(TObject& obj) const;
   void SetCentroid(Double_t cent)  { SetParameter("centroid",cent); }

   Bool_t Fit(TH1* fithist, Option_t* opt = ""); //Might switch this to TFitResultPtr
  // Bool_t Fit(TH1* fithist = 0);

   Double_t GetCentroid() const     { return GetParameter("centroid"); }
   Double_t GetCentroidErr() const  { return GetParError(GetParNumber("centroid")); }
   Double_t GetArea() const         { return fArea; }
   Double_t GetAreaErr() const      { return fDArea; }
   Double_t GetFWHM() const         { return GetParameter("sigma")*2.3548;}
   Double_t GetFWHMErr() const      { return GetParError(GetParNumber("sigma"))*2.3548;}
   Double_t GetIntegralArea(); 
   Double_t GetIntegralArea(Double_t int_low, Double_t int_high); 
   Double_t GetIntegralAreaErr(); 
   Double_t GetIntegralAreaErr(Double_t int_low, Double_t int_high); 
/*
   Double_t Fit(Option_t* opt = "");
   Double_t Fit(TH1* hist, Option_t* opt = "");
   Double_t Fit(const char* histname, Option_t* opt);
*/
   const TF1* GetFitFunction() const      { return static_cast<const TF1*>(this); } //I might move the fit functions to TGRSIFit, it's just a little tricky to initilize the function


 protected:  
   void SetArea(Double_t a) { fArea = a; }
   void SetAreaErr(Double_t d_a) { fDArea = d_a; }
   void SetArea(Double_t a, Double_t dA) { SetArea(a); SetAreaErr(dA);}
   void SetChi2(Double_t chi2)   { fChi2 = chi2; }
   void SetNdf(Double_t Ndf)     { fNdf  = Ndf; } 

 public:
   Bool_t InitParams(TH1* fithist = 0);
   TF1* Background() const { return fBackground; } 
   void DrawBackground(Option_t* opt = "SAME") const; // *MENU*
   void DrawResiduals(); // *MENU*
   void CheckArea();
   void CheckArea(Double_t int_low, Double_t int_high);

   static void SetLogLikelihoodFlag(Bool_t flag = true) { fLogLikelihoodFlag = flag; }
   static Bool_t GetLogLikelihoodFlag() { return fLogLikelihoodFlag; }

   static Bool_t CompareEnergy(const TPeak& lhs, const TPeak& rhs)  { return lhs.GetCentroid() < rhs.GetCentroid(); }
   static Bool_t CompareArea(const TPeak& lhs, const TPeak& rhs)    { return lhs.GetArea() < rhs.GetArea(); }
   static Bool_t CompareEnergy(const TPeak* lhs, const TPeak* rhs)  { return lhs->GetCentroid() < rhs->GetCentroid(); }
   static Bool_t CompareArea(const TPeak* lhs, const TPeak* rhs)    { return lhs->GetArea() < rhs->GetArea(); }

 public:
   virtual void Print(Option_t* opt = "") const;
   const char*  PrintString(Option_t* opt = "") const;
   virtual void Clear(Option_t* opt = "");

 private: 
   //Centroid will eventually be read from parameters
   Double_t fArea;
   Double_t fDArea; 
   Double_t fChi2; 
   Double_t fNdf;
   Bool_t fOwnBgFlag;

   static bool fLogLikelihoodFlag; //!<!

   TF1* fBackground;
   TGraph* fResiduals;

/// \cond CLASSIMP
  ClassDef(TPeak,2);
/// \endcond

};
/*! @} */
#endif
