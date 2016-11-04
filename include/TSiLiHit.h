#ifndef TSILIHIT_H
#define TSILIHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <utility>
#include <TGraph.h>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
	public:    
		
		enum ESiLiHitBits { // Inherited TObject fBits, Enum via TDetector
			kUseFitCharge	= kDetHitBit0,
			kSiLiHitBit1	= kDetHitBit1,
		};
    
		TSiLiHit();
		TSiLiHit(const TFragment &);	
		virtual ~TSiLiHit();
		TSiLiHit(const TSiLiHit&);
		
		void Copy(TObject&,int=0) const;        //!
		void Clear(Option_t *opt="");
		void Print(Option_t *opt="") const;

		Int_t GetRing()const;
		Int_t GetSector()const;
		Int_t GetPreamp()const;
		Double_t GetTimeFit()   { return fTimeFit;  }
		Double_t GetSig2Noise()const { return fSig2Noise;}    
		
		Int_t GetTimeStampLow()   { return GetTimeStamp() & 0x0fffffff;  }
		Double_t GetTimeFitCfd()  const { 
			if(fTimeFit!=0 && fTimeFit<1000 && fTimeFit>-1000){
				long ts=GetTimeStamp()<<4 & 0x07ffffff;//bit shift by 4 (x16) then knock off the highest bit which is absent from cfd					
				return ts+fTimeFit*16;
			}
			return 0;
		}
		
		void SetTimeFit(double t0 ) { fTimeFit = t0 ; }
		
		void SetWavefit(const TFragment&);
		TVector3 GetPosition(Double_t dist) const; //!  
		TVector3 GetPosition() const; //!  
		
		std::vector<short> fAddBackSegments;
		std::vector<double> fAddBackEnergy; //probably not needed after development finished
		
		void SumHit(TSiLiHit*);
		
		void UseFitCharge(bool set=true){
			SetBit(kIsEnergySet,false);
			SetBit(kUseFitCharge,set);
		}
		
		double GetWaveformEnergy() const {return GetFitEnergy();}
		double GetFitEnergy() const;		
		double GetFitCharge() const {return fFitCharge;}
		double GetEnergy(Option_t* opt=0) const;
		
				// Not strictly "doppler" but consistent
		inline double GetDoppler(double beta, TVector3 *vec=0)  { 
			if(vec==0) {
				vec = GetBeamDirection();
			}
			TVector3 pos = GetPosition();
			pos.SetTheta(120.*TMath::Pi()/180.);	
			double costhe=TMath::Cos(pos.Angle(*vec));
			double e=this->GetEnergy();
			double gamma = 1/(sqrt(1-pow(beta,2)));
			
			return ((e+511-beta*costhe*sqrt(e*(e+1022)))*gamma)-511;;
		}
		
		
		
	private:
		Double_t GetDefaultDistance() const { return 0.0; }
      
		Double_t    fTimeFit;
		Double_t    fSig2Noise;
		Double_t    fFitCharge;
		Double_t    fFitBase;
		

/// \cond CLASSIMP
		ClassDef(TSiLiHit,8);
/// \endcond
};
/*! @} */
#endif
