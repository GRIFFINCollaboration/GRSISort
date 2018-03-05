#ifndef TS3HIT_H
#define TS3HIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TMath.h"
#include "TPulseAnalyzer.h"
#include "TGRSIDetectorHit.h"

class TS3Hit : public TGRSIDetectorHit {
public:
   TS3Hit();
   TS3Hit(const TFragment&);
   ~TS3Hit() override;
   TS3Hit(const TS3Hit&);

   Short_t GetRing() const { return fRing; }
   Short_t GetSector() const { return fSector; }
   Bool_t  GetIsDownstream() const { return fIsDownstream; }
   Int_t   GetArrayPosition() const { return GetChannel()->GetMnemonic()->ArrayPosition(); }
   std::string GetDistanceStr() const { return GetChannel()->GetMnemonic()->ArraySubPositionString(); }
   

   Double_t fTimeFit{0.};
   Double_t fSig2Noise{0.};

public:
   void Copy(TObject&) const override; //!
   void Print(Option_t* opt = "") const override;
   void Clear(Option_t* opt = "") override;

   inline Double_t GetFitTime() { return fTimeFit; }         //!<!
   inline Double_t GetSignalToNoise() { return fSig2Noise; } //!<!

   void SetRingNumber(Short_t rn) { fRing = rn; }
   void SetSectorNumber(Short_t sn) { fSector = sn; }
   void SetIsDownstream(Bool_t dwnstrm) { fIsDownstream = dwnstrm; }

   void SetRingNumber(TFragment& frag) { fRing = frag.GetSegment(); }
   void SetSectorNumber(TFragment& frag) { fSector = frag.GetSegment(); }
   void SetSectorNumber(int n) { fSector = n; }
   void SetRingNumber(int n) { fRing = n; }

   void SetWavefit(const TFragment&);
   void SetTimeFit(Double_t time) { fTimeFit = time; }
   void SetSig2Noise(Double_t sig2noise) { fSig2Noise = sig2noise; }

   Bool_t SectorsDownstream() const;
   
   Double_t GetPhi(double offset = 0) { return GetPosition(offset).Phi(); }

   Double_t GetTheta(double offset = 0, TVector3* vec = nullptr)
   {
      if(vec == nullptr) {
         vec = new TVector3();
         vec->SetXYZ(0, 0, 1);
      }
      return GetPosition(offset).Angle(*vec);
   }
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Woverloaded-virtual"
   TVector3 GetPosition(Double_t phioffset, Double_t dist, bool smear = false) const; //!
   TVector3 GetPosition(Double_t phioffset, bool smear = false) const;                //!
   TVector3 GetPosition(bool smear = false) const;                                    //!
#pragma clang diagnostic pop

private:
   Double_t GetDefaultPhiOffset() const; //!
   Double_t GetDefaultDistance() const;  //!

   Bool_t  fIsDownstream{false}; // Downstream check
   Short_t fRing{0};         // front
   Short_t fSector{0};       // back

   /// \cond CLASSIMP
   ClassDefOverride(TS3Hit, 4);
   /// \endcond
};
/*! @} */
#endif
