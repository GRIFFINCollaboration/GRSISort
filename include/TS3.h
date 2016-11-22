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

		enum ES3Bits {
			kPixelsSet 			= BIT(0),
			kMultHit     	 	= BIT(1),
			kBit2        		= BIT(2),
			kBit3         		= BIT(3),
			kBit4         		= BIT(4),
			kBit5  				= BIT(5),
			kBit6   				= BIT(6),
			kBit7   				= BIT(7)
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

		bool MultiHit() const { return TestBitNumber(kMultHit);	 } // Get allow shared hits
		void SetMultiHit(bool flag=true)	{ SetBitNumber(kMultHit, flag); SetPixels(false);	 } // Set allow shared hits

		bool PixelsSet()	const { return TestBitNumber(kPixelsSet); }
		void SetPixels(bool flag=true) 	{ SetBitNumber(kPixelsSet, flag); }
		void BuildPixels();

		static TVector3 GetPosition(int ring, int sector, bool smear=false);
		static TVector3 GetPosition(int ring, int sector, double offsetphi, double offsetZ, bool sectorsdownstream,bool smear=false);

		void SetTargetDistance(double dist)	{ fTargetDistance = dist; }

		void ClearTransients() { fS3Bits = 0; for(auto hit : fS3Hits) hit.ClearTransients(); for(auto hit : fS3RingHits) hit.ClearTransients(); for(auto hit : fS3SectorHits) hit.ClearTransients(); }

		void Copy(TObject&) const;
		TS3& operator=(const TS3&);  // 
      virtual void Clear(Option_t *opt = "all");		     //!<!
      virtual void Print(Option_t *opt = "") const;		  //!<!
		
	private:
		std::vector<TS3Hit> fS3Hits; //!<!
		std::vector<TS3Hit> fS3RingHits, fS3SectorHits;

		TTransientBits<UChar_t> fS3Bits;                  // flags for transient members
		void ClearStatus() { fS3Bits = 0; }
		void SetBitNumber(enum ES3Bits bit,Bool_t set=true);
		Bool_t TestBitNumber(enum ES3Bits bit) const {return (fS3Bits.TestBit(bit));}
	
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
		

/// \cond CLASSIMP
		ClassDef(TS3,4)
/// \endcond
};
/*! @} */
#endif
