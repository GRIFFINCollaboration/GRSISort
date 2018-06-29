#ifndef TLABR_H
#define TLABR_H

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////
///
/// \class TLaBr
///
/// The TLaBr class defines the observables and algorithms used
/// when analyzing LaBr data. It includes detector positions,
/// etc.
///
/////////////////////////////////////////////////////////////

#include <vector>
#include <cstdio>

#include "TVector3.h"

#include "Globals.h"
#include "TSuppressed.h"
#include "TLaBrHit.h"

class TLaBr : public TSuppressed {
public:
   enum class ELaBrBits {
      kIsSuppressed = 1<<0,
      kBit1         = 1<<1,
      kBit2         = 1<<2,
      kBit3         = 1<<3,
      kBit4         = 1<<4,
      kBit5         = 1<<5,
      kBit6         = 1<<6,
      kBit7         = 1<<7
   };

   TLaBr();
   ~TLaBr() override;
   TLaBr(const TLaBr& rhs);

   TGRSIDetectorHit* GetHit(const Int_t& idx = 0) override;
   void Copy(TObject& rhs) const override;
   TLaBrHit* GetLaBrHit(const int& i);                          //!<!
   Short_t GetMultiplicity() const override { return fLaBrHits.size(); } //!<!

#if !defined(__CINT__) && !defined(__CLING__)
   void SetSuppressionCriterion(std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> criterion)
   {
      fSuppressionCriterion = std::move(criterion);
   }
   std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> GetSuppressionCriterion() const { return fSuppressionCriterion; }

   bool SuppressionCriterion(const TGRSIDetectorHit& hit, const TBgoHit& bgoHit) override { return fSuppressionCriterion(hit, bgoHit); }
#endif

   TLaBrHit* GetSuppressedHit(const int& i);                          //!<!
   Short_t GetSuppressedMultiplicity(const TBgo* fBgo);
   bool IsSuppressed() const;
	void SetSuppressed(const bool flag);
   void ResetSuppressed();

#if !defined(__CINT__) && !defined(__CLING__)
   void AddFragment(const std::shared_ptr<const TFragment>&, TChannel*) override; //!<!
#endif

   static TVector3 GetPosition(int DetNbr) { return gPosition[DetNbr]; } //!<!

   void ClearTransients() override
   {
      for(const auto& hit : fLaBrHits) {
         hit.ClearTransients();
      }
   }

   TLaBr& operator=(const TLaBr&); //!<!

private:
#if !defined(__CINT__) && !defined(__CLING__)
   static std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> fSuppressionCriterion;
#endif
   std::vector<TLaBrHit> fLaBrHits; //   The set of LaBr hits
   std::vector<TLaBrHit> fSuppressedHits; //   The set of LaBr hits

   static TVector3 gPosition[9]; //!<!  Position of each Paddle

   mutable TTransientBits<UChar_t> fLaBrBits;  // Transient member flags

   void ClearStatus() const { fLaBrBits = 0; } //!<!
   void SetBitNumber(const ELaBrBits bit, const bool set) const { fLaBrBits.SetBit(bit, set); }
   Bool_t TestBitNumber(const ELaBrBits bit) const { return fLaBrBits.TestBit(bit); }

public:
   void Clear(Option_t* opt = "") override;       //!<!
   void Print(Option_t* opt = "") const override; //!<!

   /// \cond CLASSIMP
   ClassDefOverride(TLaBr, 1) // LaBr Physics structure
	/// \endcond
};
/*! @} */
#endif
