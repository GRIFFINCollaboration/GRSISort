#ifndef TS3_H
#define TS3_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>

#include "TGRSIDetector.h"
#include "TS3Hit.h"
#include "TChannel.h"

class TS3 : public TGRSIDetector {
	public:
		TS3();
		TS3(const TS3&);
		virtual  ~TS3();

		virtual void AddFragment(TFragment*, MNEMONIC*);
		virtual void BuildHits();

		TGRSIDetectorHit* GetHit(const int& idx =0);
		TS3Hit* GetS3Hit(const int& i);  
		Short_t GetMultiplicity() const { return fS3Hits.size(); }
		void PushBackHit(TGRSIDetectorHit* deshit);

		static TVector3 GetPosition(int ring, int sector, bool downstream, double offset);

		void SetTargetDistance(double dist)	{ fTargetDistance = dist; }

		void Copy(TObject&) const;
		TS3& operator=(const TS3&);  // 
      virtual void Clear(Option_t *opt = "all");		     //!<!
      virtual void Print(Option_t *opt = "") const;		  //!<!
		
	private:
		std::vector<TS3Hit> fS3Hits;
		std::vector<TFragment*> fS3_RingFragment; //! 
		std::vector<TFragment*> fS3_SectorFragment; //! 

		///for geometery
		static int fRingNumber;          //!<!
		static int fSectorNumber;        //!<!

		static double fOffsetPhi;        //!<!
		static double fOuterDiameter;    //!<!
		static double fInnerDiameter;    //!<!
		static double fTargetDistance;   //!<!

		static Int_t fFrontBackTime;   //!
		static double fFrontBackEnergy;   //!

/// \cond CLASSIMP
		ClassDef(TS3,3)
/// \endcond
};
/*! @} */
#endif
