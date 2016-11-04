#ifndef TS3_H
#define TS3_H

/** \addtogroup Detectors
 *  @{
 */

#include <iostream>
#include <TRandom2.h>

#include "TGRSIDetector.h"
#include "TS3Hit.h"
#include "TChannel.h"

class TS3 : public TGRSIDetector {
	public:
		enum ES3Bits { // Inherited TObject fBits, Enum via TDetector
			kPixelsSet = kDetBit0,
			kMultHit = kDetBit1,
			kS3Bit2	= kDetBit2,
			kS3Bit3	= kDetBit3,
			kS3Bit4	= kDetBit4,
			kS3Bit5	= kDetBit5,
			kS3Bit6	= kDetBit6,
		};

		TS3();
		TS3(const TS3&);
		virtual  ~TS3();

#ifndef __CINT__
      void AddFragment(std::shared_ptr<const TFragment>, TChannel*); //!<!
#endif

		Short_t GetRingMultiplicity() 	const { return fS3RingHits.size(); }
		Short_t GetSectorMultiplicity() const { return fS3SectorHits.size(); }

		Int_t GetPixelMultiplicity();
		void	SetFrontBackEnergy(double de)	{ fFrontBackEnergy = de; SetPixels(false); } // Set fractional allowed energy difference
		void	SetFrontBackTime(int time)		{ fFrontBackTime = time; SetPixels(false); } // Set absolute allow time difference

		TGRSIDetectorHit* GetHit(const int& idx =0);
		TS3Hit* GetS3Hit(const int& i);  
		TS3Hit* GetRingHit(const int& i);  
		TS3Hit* GetSectorHit(const int& i);  
		
		Short_t GetMultiplicity() const { return fS3Hits.size(); }

		bool MultiHit(){ return TestBit(kMultHit);	 } // Get allow shared hits
		void SetMultiHit(bool flag=true)	{ SetBit(kMultHit, flag); SetPixels(false);	 } // Set allow shared hits

		bool PixelsSet(){ return TestBit(kPixelsSet); }
		void SetPixels(bool flag=true) { SetBit(kPixelsSet, flag); }
		void BuildPixels();

		static TVector3 GetPosition(int ring, int sector, bool smear=false);
		static TVector3 GetPosition(int ring, int sector, double offsetphi, double offsetZ, bool sectorsdownstream,bool smear=false);

		void SetTargetDistance(double dist)	{ fTargetDistance = dist; }

		void ClearTransients() { TDetector::ClearBits(); for(auto hit : fS3Hits) hit.ClearTransients(); for(auto hit : fS3RingHits) hit.ClearTransients(); for(auto hit : fS3SectorHits) hit.ClearTransients(); }

		void Copy(TObject&) const;
		TS3& operator=(const TS3&);  // 
      virtual void Clear(Option_t *opt = "all");		     //!<!
      virtual void Print(Option_t *opt = "") const;		  //!<!
		
	private:
		std::vector<TS3Hit> fS3Hits; //!<!
		std::vector<TS3Hit> fS3RingHits, fS3SectorHits;

		void ClearStatus() { TDetector::ClearBits(); }
	
		///for geometery	
		static int fRingNumber;          //!<!
		static int fSectorNumber;        //!<!

		static double fOffsetPhiCon;        //!<!
		static double fOffsetPhiSet;        //!<!

		static double fOuterDiameter;    //!<!
		static double fInnerDiameter;    //!<!
		static double fTargetDistance;   //!<!

		static Int_t fFrontBackTime;   //!
		static double fFrontBackEnergy;   //!
		
		static TRandom2 s3_rand;// random number gen for TVectors

/// \cond CLASSIMP
		ClassDef(TS3,4)
/// \endcond
};
/*! @} */
#endif
