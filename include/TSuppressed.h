#ifndef TSUPPRESSED_H
#define TSUPPRESSED_H

#include "TGRSIDetector.h"
#include "TGRSIDetectorHit.h"
#include "TBgo.h"

/** \addtogroup Detectors
 *  @{
 */

/////////////////////////////////////////////////////////////////
///
/// \class TSuppressed
///
/// This is an abstract class that adds basic functionality for
/// Compton suppressed detectors like GRIFFIN.
///
/////////////////////////////////////////////////////////////////

class TSuppressed : public TGRSIDetector {
public:
	TSuppressed() {}
	~TSuppressed() {}

#if !defined(__CINT__) && !defined(__CLING__)
	void SetAddbackCriterion(std::function<bool(const TGRSIDetectorHit&, const TGRSIDetectorHit&)> criterion) { fAddbackCriterion = criterion; }
	std::function<bool(const TGRSIDetectorHit&, const TGRSIDetectorHit&)> GetAddbackCriterion() const { return fAddbackCriterion; }
	void SetSuppressionCriterion(std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> criterion) { fSuppressionCriterion = criterion; }
	std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> GetSuppressionCriterion() const { return fSuppressionCriterion; }
#endif
private:
	void CreateAddback(const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& addbacks, std::vector<UShort_t>& nofFragments);
	void CreateSuppressed(const TBgo* bgo, const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& suppressedHits);
	void CreateSuppressedAddback(const TBgo* bgo, const std::vector<TGRSIDetectorHit>& hits, std::vector<TGRSIDetectorHit>& addbacks, std::vector<UShort_t>& nofFragments);

#if !defined(__CINT__) && !defined(__CLING__)
	static std::function<bool(const TGRSIDetectorHit&, const TGRSIDetectorHit&)> fAddbackCriterion;
	static std::function<bool(const TGRSIDetectorHit&, const TBgoHit&)> fSuppressionCriterion;
#endif

   /// \cond CLASSIMP
   ClassDefOverride(TSuppressed, 1)
   /// \endcond
};
/*! @} */
#endif
