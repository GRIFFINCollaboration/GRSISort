#ifndef TTDRTIGRESS_H
#define TTDRTIGRESS_H

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
#include "TTdrTigressHit.h"
#include "TGRSIDetector.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TTdrTigress : public TGRSIDetector {
public:
   enum class ETdrTigressBits {
      kIsAddbackSet    = 1<<0,
      kIsCrossTalkSet  = 1<<1,
      kBit2            = 1<<2,
      kBit3            = 1<<3,
      kBit4            = 1<<4,
      kBit5            = 1<<5,
      kBit6            = 1<<6,
      kBit7            = 1<<7
   };

   TTdrTigress();
   TTdrTigress(const TTdrTigress&);
   ~TTdrTigress() override;

public:
   TTdrTigressHit* GetTdrTigressHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   Short_t   GetMultiplicity() const override;

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double dist = 110.0); //!<!
   static const char* GetColorFromNumber(Int_t number);
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void ClearTransients() override
   {
      fTdrTigressBits = 0;
      for(const auto& hit : fTdrTigressHits) {
         hit.ClearTransients();
      }
   }
   void ResetFlags() const;

   TTdrTigress& operator=(const TTdrTigress&); //!<!

#if !defined(__CINT__) && !defined(__CLING__)
   void SetAddbackCriterion(std::function<bool(TTdrTigressHit&, TTdrTigressHit&)> criterion)
   {
      fAddbackCriterion = std::move(criterion);
   }
   std::function<bool(TTdrTigressHit&, TTdrTigressHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
#endif

   Int_t        GetAddbackMultiplicity();
   TTdrTigressHit* GetAddbackHit(const int& i);
   bool IsAddbackSet() const;
   void     ResetAddback();
   UShort_t GetNAddbackFrags(const size_t& idx);

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(TTdrTigressHit&, TTdrTigressHit&)> fAddbackCriterion;
#endif
   std::vector<TTdrTigressHit> fTdrTigressHits;  //  The set of crystal hits

   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

   long                            fCycleStart; //!<!  The start of the cycle
   mutable TTransientBits<UChar_t> fTdrTigressBits;  // Transient member flags

   mutable std::vector<TTdrTigressHit> fAddbackHits;  //!<! Used to create addback hits on the fly
   mutable std::vector<UShort_t> fAddbackFrags;  //!<! Number of crystals involved in creating in the addback hit

public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

private:
   static TVector3 gTigressPosition[17];                      //!<! Position of each HPGe Tigress
   void            ClearStatus() const { fTdrTigressBits = 0; } //!<!
   void SetBitNumber(ETdrTigressBits bit, Bool_t set) const;
   Bool_t TestBitNumber(ETdrTigressBits bit) const { return fTdrTigressBits.TestBit(bit); }

   static std::map<int, TSpline*> fEnergyResiduals; //!<!

   // This is where the general untouchable functions live.
   std::vector<TTdrTigressHit>* GetHitVector();      //!<!
   std::vector<TTdrTigressHit>* GetAddbackVector();  //!<!
   std::vector<UShort_t>* GetAddbackFragVector(); //!<!
   void SetAddback(bool flag = true) const;

public:
   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTdrTigress, 1) // TdrTigress Physics structure
   /// \endcond
};
/*! @} */
#endif
