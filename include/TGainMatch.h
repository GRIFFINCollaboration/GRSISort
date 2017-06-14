#ifndef TGAINMATCH_H__
#define TGAINMATCH_H__

/** \addtogroup Calibration
 *  @{
 */

#include "TCal.h"
#include "TCalManager.h"
#include "TPeak.h"
#include "TSpectrum.h"
#include "TH2.h"
#include "TF1.h"

class TGainMatch : public TCal {
public:
   TGainMatch() : fHist(0), fCoarseRange(gDefaultCoarseRange) {}
   TGainMatch(const char* name, const char* title) : TCal(name, title), fCoarseRange(gDefaultCoarseRange) { Clear(); }
   virtual ~TGainMatch() {}

   TGainMatch(const TGainMatch& copy);

public:
   void Copy(TObject& obj) const;

   void CalculateGain(Double_t cent1, Double_t cent2, Double_t eng1, Double_t eng2);

   static Bool_t CoarseMatchAll(TCalManager* cm, TH2* mat, Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2* mat, Double_t energy1, Double_t energy2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2* mat1, Double_t energy1, TH2* mat2, Double_t energy2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2* mat1, TPeak* peak1, TH2* hist2, TPeak* peak2);
   static Bool_t FineMatchFastAll(TCalManager* cm, TH2* mat, TPeak* peak1, TPeak* peak2);

   static Bool_t FineMatchAll(TCalManager* cm, TH2* chargeMat, TH2* engMat, Int_t testchan, Double_t energy1,
                              Double_t energy2, Int_t low_range = 100, Int_t high_range = 600);

   static Bool_t AlignAll(TCalManager* cm, TH1* hist, TH2* mat, Int_t low_range = 100, Int_t high_range = 600);

   Bool_t CoarseMatch(TH1* hist, Int_t channelNum = 9999, Double_t energy1 = 1173.228, Double_t energy2 = 1332.492);
   Bool_t FineMatchFast(TH1* hist1, TPeak* peak1, TH1* hist2, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1* hist, TPeak* peak1, TPeak* peak2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1* hist1, Double_t energy1, Double_t energy2, Int_t channelNum = 9999);
   Bool_t FineMatchFast(TH1* hist1, Double_t energy1, TH1* hist2, Double_t energy2, Int_t channelNum = 9999);

   Bool_t FineMatch(TH1* energyHist, TH1* testHist, TH1* chargeHist, Double_t energy1, Double_t energy2,
                    Int_t low_range = 100, Int_t high_range = 600, Int_t channelNum = 9999);

   Bool_t Align(TH1* testhist, TH1* hist, Int_t low_range = 100, Int_t high_range = 600);

   void Clear(Option_t* opt = "");
   void Print(Option_t* opt = "") const;

   Bool_t IsGroupable() const { return false; }
   void   WriteToChannel() const;

   void SetNucleus(TNucleus*, Option_t* = "") { Warning("SetNucleus", "Is not used in TGainMatching"); }
   TNucleus* GetNucleus() const
   {
      Warning("GetNucleus", "Is not used in TGainMatching");
      return 0;
   }

   void SetHist(TH1*) { Warning("SetHist", "Is not used in TGainMatching"); }
   TH1*              GetHist() const
   {
      Warning("GetHist", "Is not used in TGainMatching");
      return 0;
   }

   void SetCoarseRange(Double_t coarseRange) { fCoarseRange = coarseRange; }
   Double_t                     GetCoarseRange() const { return fCoarseRange; }
   static void SetDefaultCoarseRange(Double_t coarseRange)
   {
      printf("All new TGainMatch objects will have their range set to %lf\n", coarseRange);
      gDefaultCoarseRange = coarseRange;
   }
   static Double_t GetDefaultCoarseRange() { return gDefaultCoarseRange; }

private:
   Bool_t   fCoarseMatch;
   Bool_t   fAligned;
   TH1*     fHist;
   Double_t fAlignCoeffs[2];
   Double_t fGainCoeffs[2];
   Double_t fCoarseRange;
   Double_t HistCompare(Double_t* x, Double_t* par);

   static Double_t gDefaultCoarseRange;

   /// \cond CLASSIMP
   ClassDef(TGainMatch, 1);
   /// \endcond
};
/*! @} */
#endif
