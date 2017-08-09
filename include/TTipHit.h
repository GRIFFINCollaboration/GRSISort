#ifndef TTIPHIT_H
#define TTIPHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TGRSIDetectorHit.h"

class TTipHit : public TGRSIDetectorHit {
public:
   TTipHit();
   TTipHit(const TFragment&);
   ~TTipHit() override;
   TTipHit(const TTipHit&);

private:
   Int_t    fFilter{0}; //
   Double_t fPID{0.};    //
   Int_t    fChiSq{0};

   // Double_t fFastAmplitude;
   // Double_t fSlowAmplitude;
   // Double_t fGammaAmplitude;

   bool csi_flag{false};

   Int_t fTipChannel{0};

   Double_t fTimeFit{0.};
   Double_t fSig2Noise{0.};

public:
   /////////////////////////    /////////////////////////////////////
   inline void SetFilterPattern(const int& x) { fFilter = x; } //!<!
   inline void SetPID(Double_t x) { fPID = x; }                //!<!
   inline void SetTipChannel(const int x) { fTipChannel = x; } //!<!

   inline Int_t    GetFiterPatter() { return fFilter; }      //!<!
   inline Double_t GetPID() { return fPID; }                 //!<!
   inline Int_t    GetFitChiSq() { return fChiSq; }          //!<!
   inline Double_t GetFitTime() { return fTimeFit; }         //!<!
   inline Double_t GetSignalToNoise() { return fSig2Noise; } //!<!
   inline Int_t    GetTipChannel() { return fTipChannel; }   //!<!

   inline bool IsCsI() { return csi_flag; }                    //!<!
   inline void SetCsI(bool flag = "true") { csi_flag = flag; } //!<!
   inline void SetFitChiSq(int chisq) { fChiSq = chisq; }      //!<!

   bool InFilter(Int_t); //!<!

   // void SetVariables(const TFragment &frag) { SetAddress(frag.ChannelAddress);
   //									 SetCfd(frag.GetCfd());
   //                                   SetCharge(frag.GetCharge());
   //                                     SetTimeStamp(frag.GetTimeStamp()); }

   void SetUpNumbering(TChannel*)
   {
      TChannel* channel = GetChannel();
      if(!channel) {
         Error("SetDetector", "No TChannel exists for address %u", GetAddress());
         return;
      }
      Int_t tmp = atoi(channel->GetMnemonic()->ArraySubPositionString().c_str());
      SetTipChannel(10 * channel->GetMnemonic()->ArrayPosition() + tmp);
      if(channel->GetMnemonic()->SubSystemString().compare(0, 1, "W") == 0) {
         SetCsI();
      }
   }

   void SetWavefit(const TFragment&);

   void SetPID(const TFragment&);

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!
   void Copy(TObject&) const override;            //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTipHit, 1);
   /// \endcond
};
/*! @} */
#endif
