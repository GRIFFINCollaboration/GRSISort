#ifndef TPACES_H
#define TPACES_H

/** \addtogroup Detectors
 *  @{
 */

#include <vector>
#include <cstdio>

#include "TBits.h"
#include "TVector3.h" 

#include "Globals.h"
#include "TGRSIDetector.h" 
#include "TPacesHit.h"

class TPaces : public TGRSIDetector {

	public:
		TPaces();
		TPaces(const TPaces&);
		virtual ~TPaces();

	public: 
		TPacesHit* GetPacesHit(const int& i); //!<!
		TGRSIDetectorHit* GetHit(const Int_t& idx = 0);
		Short_t GetMultiplicity() const { return fPacesHits.size(); }

		static TVector3 GetPosition(int DetNbr);		//!<!
		void AddFragment(TFragment*, TChannel*); //!<!
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		TPaces& operator=(const TPaces&);  //!<! 


	private: 
		std::vector<TPacesHit> fPacesHits; //  The set of crystal hits

		static bool fSetCoreWave;		         //!<!  Flag for Waveforms ON/OFF

	public:
		static bool SetCoreWave()        { return fSetCoreWave;  }	//!<!

		virtual void Copy(TObject&) const;                //!<!
		virtual void Clear(Option_t* opt = "all");		     //!<!
		virtual void Print(Option_t* opt = "") const;		  //!<!

	protected:
		void PushBackHit(TGRSIDetectorHit* phit);

/// \cond CLASSIMP
		ClassDef(TPaces,4)  // Paces Physics structure
/// \endcond
};
/*! @} */
#endif


