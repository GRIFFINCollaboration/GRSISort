#ifndef TS3HIT_H
#define TS3HIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include "TFragment.h"
#include "TChannel.h"
#include "TMath.h"
#include "TPulseAnalyzer.h"
#include "TGRSIDetectorHit.h"

class TS3Hit : public TGRSIDetectorHit {
	public:
		TS3Hit();
		TS3Hit(TFragment &);
		virtual ~TS3Hit();
		TS3Hit(const TS3Hit&);

		Short_t  GetRing()  const  { return fRing;   }
		Short_t  GetSector() const { return fSector; }
		Bool_t	 GetIsDownstream() const { return fIsDownstream; }

		Double_t   fTimeFit;
		Double_t   fSig2Noise;

	public:
		void Copy(TObject&) const;        //!
		void Print(Option_t* opt="") const;
		void Clear(Option_t* opt="");

		inline Double_t GetFitTime()			           { return fTimeFit;	   } //!<!
		inline Double_t GetSignalToNoise()		        { return fSig2Noise;	} //!<!

		void SetRingNumber(Short_t rn)     			{ fRing = rn;   }
		void SetSectorNumber(Short_t sn)   			{ fSector = sn; }
		void SetIsDownstream(Bool_t dwnstrm) 		{ fIsDownstream = dwnstrm; }

		void SetRingNumber(TFragment &frag)     { fRing = GetMnemonicSegment(frag);   	}
		void SetSectorNumber(TFragment &frag)   { fSector = GetMnemonicSegment(frag) ; 	}
		void SetSectorNumber(int n)   					{ fSector = n ; 												}
		void SetRingNumber(int n)   						{ fRing 	= n ; 												}

		void SetWavefit(TFragment&);
		void SetTimeFit(Double_t time)					{ fTimeFit = time;											}
		void SetSig2Noise(Double_t sig2noise)		{ fSig2Noise = sig2noise;								}

    Short_t GetMnemonicSegment(TFragment &frag);	//could be added to TGRSIDetectorHit base class

		Double_t GetPhi(double offset=0) {
			return this->GetChannelPosition(offset).Phi();
		}

		Double_t GetTheta(double offset=0, TVector3 *vec=0) {
			if(vec==0) {
				vec = new TVector3();
				vec->SetXYZ(0,0,1);
			}
			return this->GetChannelPosition(offset).Angle(*vec);
		}
		TVector3 GetChannelPosition(Double_t offset = 0, Double_t dist = 0) const; //!

	private:

		Bool_t 	 fIsDownstream; // Downstream check
		Short_t  fRing;   //front
		Short_t  fSector; //back

		/// \cond CLASSIMP
		ClassDef(TS3Hit,4);
		/// \endcond
};
/*! @} */
#endif
