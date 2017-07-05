#ifndef TFIPPS_H
#define TFIPPS_H

/** \addtogroup Detectors
 *  @{
 */

#include <utility>
#include <vector>
#include <cstdio>
#include <functional>
//#include <tuple>

#include "TBits.h"
#include "TVector3.h"

#include "Globals.h"
#include "TFippsHit.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"

class TFipps : public TGRSIDetector {
public:
   enum EFippsBits {
      kIsAddbackSet   = 1<<0,
      kIsCrossTalkSet = 1<<1,
      kBit2           = 1<<2,
      kBit3           = 1<<3,
      kBit4           = 1<<4,
      kBit5           = 1<<5,
      kBit6           = 1<<6,
      kBit7           = 1<<7
   };

   TFipps();
   TFipps(const TFipps&);
   ~TFipps() override;

public:
   TFippsHit* GetFippsHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
   Int_t GetMultiplicity() const;

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double dist = 110.0); //!<!
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void ClearTransients() override
   {
      fFippsBits = 0;
      for(const auto& hit : fFippsHits) {
         hit.ClearTransients();
      }
   }
   void ResetFlags() const;

   TFipps& operator=(const TFipps&); //!<!

#if !defined(__CINT__) && !defined(__CLING__)
   void SetAddbackCriterion(std::function<bool(TFippsHit&, TFippsHit&)> criterion)
   {
      fAddbackCriterion = std::move(criterion);
   }
   std::function<bool(TFippsHit&, TFippsHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
#endif

   Int_t      GetAddbackMultiplicity();
   TFippsHit* GetAddbackHit(const int& i);
   bool     IsAddbackSet() const;
   void     ResetAddback();
   UShort_t GetNAddbackFrags(const size_t& idx);

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(TFippsHit&, TFippsHit&)> fAddbackCriterion;
#endif
   std::vector<TFippsHit> fFippsHits; //  The set of crystal hits

   // static bool fSetBGOHits;                //!<!  Flag that determines if BGOHits are being measured

   mutable TTransientBits<UChar_t> fFippsBits; // Transient member flags

   mutable std::vector<TFippsHit> fAddbackHits;  //!<! Used to create addback hits on the fly
   mutable std::vector<UShort_t>  fAddbackFrags; //!<! Number of crystals involved in creating in the addback hit

public:
   // static bool SetBGOHits()       { return fSetBGOHits;   }  //!<!

private:
   static TVector3 gCloverPosition[17];                    //!<! Position of each HPGe Clover
   void            ClearStatus() const { fFippsBits = 0; } //!<!
   void SetBitNumber(enum EFippsBits bit, Bool_t set) const;
   Bool_t TestBitNumber(enum EFippsBits bit) const { return fFippsBits.TestBit(bit); }

   // Cross-Talk stuff
public:
   static const Double_t gStrongCT[2];           //!<!
   static const Double_t gWeakCT[2];             //!<!
   static const Double_t gCrossTalkPar[2][4][4]; //!<!
   static Double_t CTCorrectedEnergy(const TFippsHit* const hit_to_correct, const TFippsHit* const other_hit,
                                     Bool_t time_constraint = true);
   Bool_t IsCrossTalkSet() const;
   void   FixCrossTalk();

private:
   // This is where the general untouchable functions live.
   std::vector<TFippsHit>* GetHitVector();         //!<!
   std::vector<TFippsHit>* GetAddbackVector();     //!<!
   std::vector<UShort_t>*  GetAddbackFragVector(); //!<!
   void SetAddback(bool flag = true) const;
   void SetCrossTalk(bool flag = true) const;

public:
   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TFipps, 5) // Fipps Physics structure
   /// \endcond
};
/*! @} */
#endif
