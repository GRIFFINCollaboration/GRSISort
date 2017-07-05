#ifndef TSILI_H
#define TSILI_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <iostream>
#include <TRandom2.h>

#include "TGRSIDetector.h"
#include "TSiLiHit.h"
#include "TGRSIRunInfo.h"

class TSiLi : public TGRSIDetector {
public:
   enum ESiLiBits {
      kAddbackSet = BIT(0),
      kSiLiBit1   = BIT(1),
      kSiLiBit2   = BIT(2),
      kSiLiBit3   = BIT(3),
      kSiLiBit4   = BIT(4),
      kSiLiBit5   = BIT(5),
      kSiLiBit6   = BIT(6),
      kSiLiBit7   = BIT(7)
   };

public:
   TSiLi();
   TSiLi(const TSiLi&);
   ~TSiLi() override;

#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   void ClearTransients() override
   {
      for(const auto& hit : fSiLiHits) {
         hit.ClearTransients();
      }
   }

   TSiLi& operator=(const TSiLi&); //

   void Copy(TObject&) const override;
   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   Short_t           GetMultiplicity() const { return fSiLiHits.size(); }
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
   TSiLiHit* GetSiLiHit(const Int_t& i = 0);

   TSiLiHit* GetAddbackHit(const Int_t& i = 0);
   Int_t GetAddbackMultiplicity();

   void ResetAddback()
   {
      fSiLiBits.SetBit(kAddbackSet, false);
      fAddbackHits.clear();
   }

   void UseFitCharge()
   {
      for(auto& fSiLiHit : fSiLiHits) {
         fSiLiHit.UseFitCharge();
      }
   }

   static TVector3 GetPosition(int ring, int sector, bool smear = false);

   static std::vector<TGraph> UpstreamShapes();

   static double sili_noise_fac;        // Sets the level of integration to remove noise during waveform fitting
   static double sili_default_decay;    // Sets the waveform fit decay parameter
   static double sili_default_rise;     // Sets the waveform fit rise parameter
   static double sili_default_baseline; // Sets the waveform fit rise parameter
   static Int_t GetRing(Int_t seg) { return seg / 12; }
   static Int_t GetSector(Int_t seg) { return seg % 12; }
   static Int_t GetPreamp(Int_t seg)
   {
      return ((GetSector(seg) / 3) * 2) + (((GetSector(seg) % 3) + GetRing(seg)) % 2);
   }

   static double GetSegmentArea(Int_t seg);

   bool fAddbackCriterion(TSiLiHit*, TSiLiHit*);

   // This value defines what scheme is used when fitting sili waveforms
   // 0 quick linear eq. method, requires good baseline
   // 1 use slow TF1 fit if quick linear eq. method fails
   // 2 use slow TF1 method exclusively
   static int FitSiLiShape;

   static double SiLiBaseLine[120];
   static double SiLiRiseTime[120];
   static double SiLiDecayTime[120];

private:
   std::vector<TSiLiHit> fSiLiHits;
   std::vector<TSiLiHit> fAddbackHits;

   TTransientBits<UChar_t> fSiLiBits;

   void SortCluster(std::vector<unsigned>&);

   /// for geometery
   static int    fRingNumber;     //!<!
   static int    fSectorNumber;   //!<!
   static double fOffsetPhi;      //!<!
   static double fOuterDiameter;  //!<!
   static double fInnerDiameter;  //!<!
   static double fTargetDistance; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TSiLi, 5);
   /// \endcond
};
/*! @} */
#endif
