#ifndef TSILIHIT_H
#define TSILIHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>
#include <TGraph.h>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
public:
   enum class ESiLiHitBits { kUseFitCharge = BIT(0), kSiLiHitBit1 = BIT(1) };

   TSiLiHit();
   TSiLiHit(const TFragment&);
   ~TSiLiHit() override;
   TSiLiHit(const TSiLiHit&);

   void Copy(TObject&, bool = false) const override; //!
   void Clear(Option_t* opt = "") override;
   void Print(Option_t* opt = "") const override;

   Int_t    GetRing() const;
   Int_t    GetSector() const;
   Int_t    GetPreamp() const;
   Int_t    GetPin() const;
   bool     MagnetShadow() const;
   Double_t GetTimeFit() const {// In 10ns tstamp units
      TChannel* channel = GetChannel();
      if(channel != nullptr) return fTimeFit+channel->GetTZero(GetEnergy());
      return fTimeFit;
   }
   
   Double_t GetSig2Noise() const { return fSig2Noise; }
   Double_t GetSmirnov() const { return fSmirnov; }

   Int_t    GetTimeStampLow() { return GetTimeStamp() & 0x0fffffff; }
   Double_t GetTimeFitns() const
   {
      return (GetTimeStamp()+GetTimeFit())*10.;
   }   
   Double_t GetTimeFitCfd() const
   {
	   double fitt=GetTimeFit();
      if(fitt != 0 && fitt < 1000 && fitt > -1000) {
         long ts = GetTimeStamp()<<4 &
                   0x07ffffff; // bit shift by 4 (x16) then knock off the highest bit which is absent from cfd
         return ts + fitt * 16;
      }
      return 0;
   }

   void SetTimeFit(double t0) { fTimeFit = t0; }

   void             SetWavefit(const TFragment&);
   static TChannel* GetSiLiHitChannel(int segment);
   static TPulseAnalyzer* FitFrag(const TFragment& frag, int ShapeFit, int segment);
   static TPulseAnalyzer* FitFrag(const TFragment& frag, int ShapeFit = 0, TChannel* = nullptr);
   static int FitPulseAnalyzer(TPulseAnalyzer* pulse, int ShapeFit, int segment);
   static int FitPulseAnalyzer(TPulseAnalyzer* pulse, int ShapeFit = 0, TChannel* = nullptr);
   TVector3 GetPosition(Double_t dist, bool) const; //!
   TVector3 GetPosition(bool) const;                //!

	TVector3 GetPosition(Double_t dist) const override { return GetPosition(dist, false); }
	TVector3 GetPosition() const override { return GetPosition(false); }

   void SumHit(TSiLiHit*);

   void UseFitCharge(bool set = true)
   {
      SetHitBit(EBitFlag::kIsEnergySet, false);
      fSiLiHitBits.SetBit(ESiLiHitBits::kUseFitCharge, set);
   }

   double GetWaveformEnergy() const { return GetFitEnergy(); }
   double GetFitEnergy() const;
   double GetFitCharge() const { return fFitCharge; }
   double GetEnergy(Option_t* opt = nullptr) const override;

   // Not strictly "doppler" but consistent
   inline double GetDoppler(double beta, TVector3* vec = nullptr,double E=0)
   {
      if(vec == nullptr) {
         vec = GetBeamDirection();
      }
      TVector3 pos = GetPosition();
      pos.SetTheta(130. * TMath::Pi() / 180.);
      double costhe = TMath::Cos(pos.Angle(*vec));
      if(E>0) E= this->GetEnergy();
      double gamma  = 1 / (sqrt(1 - pow(beta, 2)));

      return ((E + 511 - beta * costhe * sqrt(E * (E + 1022))) * gamma) - 511;
      ;
   }

   unsigned int GetAddbackSize()
   {
      if(fAddBackSegments.size() == fAddBackEnergy.size()) {
         return fAddBackEnergy.size();
      }
      return 0;
   }

   double GetAddbackEnergy(unsigned int i)
   {
      if(i < GetAddbackSize()) {
         return fAddBackEnergy[i];
      }
      return 0;
   }
   short GetAddbackSegment(unsigned int i)
   {
      if(i < GetAddbackSize()) {
         return fAddBackSegments[i];
      }
      return 0;
   }

private:
   Double_t GetDefaultDistance() const { return 0.0; }

   std::vector<short>      fAddBackSegments;   //!<!
   std::vector<double>     fAddBackEnergy;     //!<!
   // probably not needed after development finished
   TTransientBits<UChar_t> fSiLiHitBits;

   Double_t fTimeFit{0.};
   Double_t fSig2Noise{0.};
   Double_t fSmirnov{0.};
   Double_t fFitCharge{0.};
   Double_t fFitBase{0.};

   /// \cond CLASSIMP
   ClassDefOverride(TSiLiHit, 10);
   /// \endcond
};
/*! @} */
#endif
