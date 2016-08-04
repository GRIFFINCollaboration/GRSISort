#ifndef TTRIFOIL_PHASE_H
#define TTRIFOIL_PHASE_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <iostream>
#include <stdio.h>

#include "TDetector.h"
#include "TFragment.h"

class TTriFoil :  public TDetector {
	public:
		TTriFoil();
		virtual ~TTriFoil();
		TTriFoil(const TTriFoil& rhs);

		std::vector<Short_t> GetWave() { return fTfWave; }
		bool Beam() const { return fBeam; }
		int TBeam() const { return fTBeam; }

		bool HasWave() const { return !fTfWave.empty(); }
		time_t GetTimeStamp() const { return fTimestamp; }

		void AddFragment(TFragment*, TChannel*); //!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		void Clear(Option_t* opt = ""); 	//!<!
		void Print(Option_t* opt = "") const; 	//!<!
		void Copy(TObject &rhs) const;

	private:
		std::vector<Short_t> fTfWave;
		time_t fTimestamp;
		bool fBeam;
		int fTBeam;

/// \cond CLASSIMP
		ClassDef(TTriFoil,2)
/// \endcond
};
/*! @} */
#endif
