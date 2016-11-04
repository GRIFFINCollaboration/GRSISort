#ifndef TSILI_H
#define TSILI_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <iostream>
#include <TRandom2.h>

#include "TGRSIDetector.h"
#include "TSiLiHit.h"
#include "TGRSIRunInfo.h"

class TSiLi: public TGRSIDetector  {
	public:
		enum ESiLiBits { // Inherited TObject fBits, Enum via TDetector
			kAddbackSet = kDetBit0,
			kSiLiBit1 = kDetBit1,
			kSiLiBit2 = kDetBit2,
			kSiLiBit3 = kDetBit3,
			kSiLiBit4 = kDetBit4,
			kSiLiBit5 = kDetBit5,
			kSiLiBit6 = kDetBit6,
		};
		
		TSiLi();
		TSiLi(const TSiLi&);
		virtual ~TSiLi();

#ifndef __CINT__
      void AddFragment(std::shared_ptr<const TFragment>, TChannel*); //!<!
#endif

		void ClearTransients() { for(auto hit : fSiLiHits) hit.ClearTransients(); }

		TSiLi& operator=(const TSiLi&);  // 

		void Copy(TObject&) const;
		void Clear(Option_t *opt="");   
		void Print(Option_t *opt="") const;

		Short_t GetMultiplicity() const { return fSiLiHits.size(); }
		TGRSIDetectorHit* GetHit(const Int_t& idx =0);
		TSiLiHit* GetSiLiHit(const Int_t& idx = 0);
		
		
		TSiLiHit* GetAddbackHit(const Int_t& idx = 0);
		Int_t GetAddbackMultiplicity();
		void ResetAddback() {
			SetBit(kAddbackSet, false);
			fAddbackHits.clear();
		}
		
		
		void UseFitCharge(){
			for(unsigned int s=0;s<fSiLiHits.size();s++)fSiLiHits[s].UseFitCharge();
		}
		
		static TVector3 GetPosition(int ring, int sector, bool smear=false);
		
		static std::vector< TGraph > UpstreamShapes();

		static double sili_noise_fac;// Sets the level of integration to remove noise during waveform fitting
		static Int_t GetRing(Int_t seg) {  return seg/12; }
		static Int_t GetSector(Int_t seg) {  return seg%12; }
		static Int_t GetPreamp(Int_t seg) {  return  ((GetSector(seg)/3)*2)+(((GetSector(seg)%3)+GetRing(seg))%2); }
		
		static double GetSegmentArea(Int_t seg);
		
		bool fAddbackCriterion(TSiLiHit*, TSiLiHit*);
		
	private:
		std::vector<TSiLiHit> fSiLiHits;
		std::vector<TSiLiHit> fAddbackHits;
		
		void SortCluster(std::vector<unsigned>&);
		
		///for geometery
		static int fRingNumber;          //!<!
		static int fSectorNumber;        //!<!
		static double fOffsetPhi;        //!<!
		static double fOuterDiameter;    //!<!
		static double fInnerDiameter;    //!<!
		static double fTargetDistance;   //!<!
		
		static TRandom2 sili_rand;// random number gen for TVectors

/// \cond CLASSIMP
		ClassDef(TSiLi,4);
/// \endcond
};
/*! @} */
#endif
