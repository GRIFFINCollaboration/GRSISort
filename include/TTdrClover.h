#ifndef TTDRCLOVER_H
#define TTDRCLOVER_H

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
#include "TTdrCloverHit.h"
#include "TBgo.h"
#include "TBgoHit.h"
#include "TSuppressed.h"
#include "TGRSIRunInfo.h"
#include "TTransientBits.h"
#include "TSpline.h"

class TTdrClover : public TSuppressed {
public:
   enum class ETdrCloverBits {
      kIsAddbackSet    = 1<<0,
      kIsCrossTalkSet  = 1<<1,
      kIsSuppressedSet = 1<<2,
      kIsSupprAddbSet  = 1<<3,
      kBit4            = 1<<4,
      kBit5            = 1<<5,
      kBit6            = 1<<6,
      kBit7            = 1<<7
   };

   TTdrClover();
   TTdrClover(const TTdrClover&);
   ~TTdrClover() override;

public:
   TTdrCloverHit* GetTdrCloverHit(const Int_t& i);
   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   Short_t   GetMultiplicity() const override;

   static TVector3 GetPosition(int DetNbr, int CryNbr = 5, double dist = 110.0); //!<!
   static const char* GetColorFromNumber(Int_t number);
#ifndef __CINT__
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif
   void ClearTransients() override
   {
      fTdrCloverBits = 0;
      for(const auto& hit : fTdrCloverHits) {
         hit.ClearTransients();
      }
   }
   void ResetFlags() const;

   TTdrClover& operator=(const TTdrClover&); //!<!

#if !defined(__CINT__) && !defined(__CLING__)
   void SetAddbackCriterion(std::function<bool(TTdrCloverHit&, TTdrCloverHit&)> criterion)
   {
      fAddbackCriterion = std::move(criterion);
   }
   std::function<bool(TTdrCloverHit&, TTdrCloverHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
   void SetSuppressionCriterion(std::function<bool(TTdrCloverHit&, TBgoHit&)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(TTdrCloverHit&, TBgoHit&)> GetSuppressionCriterion() const { return fSuppressionCriterion; }
#endif

   Int_t        GetAddbackMultiplicity();
   TTdrCloverHit* GetAddbackHit(const int& i);
   bool IsAddbackSet() const;
   void     ResetAddback();
   UShort_t GetNAddbackFrags(const size_t& idx);

   Int_t        GetSuppressedMultiplicity(TBgo*);
   TTdrCloverHit* GetSuppressedHit(const int& i);
   bool IsSuppressedSet() const;
   void     ResetSuppressed();

   Int_t        GetSuppressedAddbackMultiplicity(TBgo*);
   TTdrCloverHit* GetSuppressedAddbackHit(const int& i);
   bool IsSuppressedAddbackSet() const;
   void     ResetSuppressedAddback();
   UShort_t GetNSuppressedAddbackFrags(const size_t& idx);

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(TTdrCloverHit&, TTdrCloverHit&)> fAddbackCriterion;
   static std::function<bool(TTdrCloverHit&, TBgoHit&)> fSuppressionCriterion;
#endif
   std::vector<TTdrCloverHit> fTdrCloverHits;  //  The set of crystal hits

   static bool fSetCoreWave; //!<!  Flag for Waveforms ON/OFF

   long                            fCycleStart; //!<!  The start of the cycle
   mutable TTransientBits<UChar_t> fTdrCloverBits;  // Transient member flags

   mutable std::vector<TTdrCloverHit> fAddbackHits;  //!<! Used to create addback hits on the fly
   mutable std::vector<UShort_t> fAddbackFrags;  //!<! Number of crystals involved in creating in the addback hit

   mutable std::vector<TTdrCloverHit> fSuppressedHits;  //!<! Used to create suppressed hits on the fly
   mutable std::vector<TTdrCloverHit> fSuppressedAddbackHits;  //!<! Used to create suppressed addback hits on the fly
   mutable std::vector<UShort_t> fSuppressedAddbackFrags;  //!<! Number of crystals involved in creating in the suppressed addback hit
public:
   static bool SetCoreWave() { return fSetCoreWave; } //!<!

private:
   static TVector3 gCloverPosition[17];                      //!<! Position of each HPGe Clover
   void            ClearStatus() const { fTdrCloverBits = 0; } //!<!
   void SetBitNumber(ETdrCloverBits bit, Bool_t set) const;
   Bool_t TestBitNumber(ETdrCloverBits bit) const { return fTdrCloverBits.TestBit(bit); }

   static std::map<int, TSpline*> fEnergyResiduals; //!<!

   // This is where the general untouchable functions live.
   std::vector<TTdrCloverHit>& GetHitVector();      //!<!
   std::vector<TTdrCloverHit>& GetAddbackVector();  //!<!
   std::vector<UShort_t>& GetAddbackFragVector(); //!<!
   void SetAddback(bool flag = true) const;
   std::vector<TTdrCloverHit>& GetSuppressedVector();  //!<!
   void SetSuppressed(bool flag = true) const;
   std::vector<TTdrCloverHit>& GetSuppressedAddbackVector();  //!<!
   std::vector<UShort_t>& GetSuppressedAddbackFragVector(); //!<!
   void SetSuppressedAddback(bool flag = true) const;

public:
   void Copy(TObject&) const override;            //!<!
   void Clear(Option_t* opt = "all") override;    //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TTdrClover, 1) // TdrClover Physics structure
   /// \endcond
};
/*! @} */
#endif
