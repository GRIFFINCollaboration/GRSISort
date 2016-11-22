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
		int NTBeam() const { return fTBeam.size(); }
		//int TBeam() const { return TBeam(0); }
		int TBeam(unsigned int n=0) const { 	if(n<fTBeam.size()) 
			return fTBeam.at(n);
			else
				return -1; }

		bool HasWave() const { return !fTfWave.empty(); }
		time_t GetTimeStamp() const { return fTimestamp; }

#ifndef __CINT__
		void AddFragment(std::shared_ptr<const TFragment>, TChannel*); //!<!
#endif

		void Clear(Option_t* opt = "");   //!<!
		void Print(Option_t* opt = "") const;   //!<!
		void Copy(TObject &rhs) const;

	private:
		std::vector<Short_t> fTfWave;
		Long_t fTimestamp;
		bool fBeam;
		std::vector<int> fTBeam;

/// \cond CLASSIMP
		ClassDef(TTriFoil,2)
/// \endcond
};
/*! @} */
#endif
