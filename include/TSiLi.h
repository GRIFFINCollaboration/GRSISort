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
   enum class ESiLiBits {
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

   Short_t           GetMultiplicity() const override { return fSiLiHits.size(); }
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   TSiLiHit* GetSiLiHit(const Int_t& i = 0);
   
   TSiLiHit* GetAddbackHit(const Int_t& i = 0);
   TSiLiHit* GetRejectHit(const Int_t& i = 0);
   Int_t GetAddbackMultiplicity();
   Int_t GetRejectMultiplicity();

   void ResetAddback()
   {
      fSiLiBits.SetBit(ESiLiBits::kAddbackSet, false);
      fAddbackHits.clear();
   }

   void UseFitCharge()
   {
      for(auto& fSiLiHit : fSiLiHits) {
         fSiLiHit.UseFitCharge();
      }
   }
   
   void CoincidenceTime(double time)
   {   
      fSiLiCoincidenceTime = time;
      ResetAddback();
   } 
   
   void RejectCrosstalk(bool reject=true)
   {
      fRejectPossibleCrosstalk=reject;
      ResetAddback();
   }
   

   static TVector3 GetPosition(int ring, int sector, bool smear = false);

   static std::vector<TGraph> UpstreamShapes();

   static double sili_noise_fac;        // Sets the level of integration to remove noise during waveform fitting
   static double sili_default_decay;    // Sets the waveform fit decay parameter
   static double sili_default_rise;     // Sets the waveform fit rise parameter
   static double sili_default_baseline; // Sets the waveform fit rise parameter

	static Int_t GetRing(Int_t seg) {  return seg/12; }
	static Int_t GetSector(Int_t seg) {  return seg%12; }
	static Int_t GetPreamp(Int_t seg) {  return  ((GetSector(seg)/3)*2)+(((GetSector(seg)%3)+GetRing(seg))%2); }
	static Int_t GetPin(Int_t seg) {//stupid chequerboard pattern that inverts
		int ring=GetRing(seg);
		int sec=GetSector(seg)%3;
		int inv=(GetSector(seg)/3)%2;
		int ret=10*(2-sec);
		if((sec==1)^!(inv))ret+=9-ring;
		else ret+=ring;
		return  (ret/2)+1;
	}
	static bool MagnetShadow(Int_t seg) { return ((seg%3)==1); }
	
   static double GetSegmentArea(Int_t seg);

   bool fAddbackCriterion(TSiLiHit*, TSiLiHit*);
   bool fRejectCriterion(TSiLiHit*, TSiLiHit*);
   bool fCoincidenceTime(TSiLiHit*, TSiLiHit*);

// This value defines what scheme is used when fitting sili waveforms
// 0 quick linear eq. method, requires good baseline
// 1 use slow TF1 fit if quick linear eq. method fails
// 2 use slow TF1 method exclusively
// 3 use slow TF1 with experimental oscillation
   static int FitSiLiShape;     //!<!
   static double BaseFreq;     //!<!
   static std::string fPreAmpName[8];     //!<!
	
private:
   std::vector<TSiLiHit> fSiLiHits;
   std::vector<TSiLiHit> fAddbackHits;     //!<!
   std::vector<unsigned int> fRejectHits;     //!<!

   TTransientBits<UChar_t> fSiLiBits;

   void AddCluster(std::vector<unsigned>&,bool=false);

   /// for geometery
   static int    fRingNumber;     //!<!
   static int    fSectorNumber;   //!<!
   static double fOffsetPhi;      //!<!
   static double fOuterDiameter;  //!<!
   static double fInnerDiameter;  //!<!
   static double fTargetDistance; //!<!

public:
   static double  fSiLiCoincidenceTime; //!<!
   static bool  fRejectPossibleCrosstalk; //!<!
   
   /// \cond CLASSIMP
   ClassDefOverride(TSiLi, 6);
   /// \endcond
};
/*! @} */
#endif
