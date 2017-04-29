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
		enum ESiLiHitBits { 
			kUseFitCharge	= BIT(0),
	  		kSiLiHitBit1	= BIT(1)
		};

		TSiLiHit();
		TSiLiHit(const TFragment &);	
		virtual ~TSiLiHit();
		TSiLiHit(const TSiLiHit&);
		
		void Copy(TObject&, bool = false) const;        //!
		void Clear(Option_t *opt="");
		void Print(Option_t *opt="") const;

		Int_t GetRing()const;
		Int_t GetSector()const;
		Int_t GetPreamp()const;
		Double_t GetTimeFit()   { return fTimeFit;  }
		Double_t GetSig2Noise()const { return fSig2Noise;} 
		Double_t GetSmirnov()const { return fSmirnov;} 
		
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
		static TChannel* GetSiLiHitChannel(int segment);
		static TPulseAnalyzer* FitFrag(const TFragment &frag,int ShapeFit,int segment);
		static TPulseAnalyzer* FitFrag(const TFragment &frag,int ShapeFit=0,TChannel* =0);
		static int FitPulseAnalyzer(TPulseAnalyzer* pulse,int ShapeFit,int segment);
		static int FitPulseAnalyzer(TPulseAnalyzer* pulse,int ShapeFit=0,TChannel* =0);
		TVector3 GetPosition(Double_t dist, bool) const; //!  
		TVector3 GetPosition(Double_t dist) const { return GetPosition(dist, false); } //!  
		TVector3 GetPosition(bool) const; //!  
		TVector3 GetPosition() const { return GetPosition(false); } //!  
		
		void SumHit(TSiLiHit*);
		
		void UseFitCharge(bool set=true){
	  		SetHitBit(kIsEnergySet,false);
			fSiLiHitBits.SetBit(kUseFitCharge,set);
		}

	
		double GetWaveformEnergy() const {return GetFitEnergy();}
		double GetFitEnergy() const;		
		double GetFitCharge() const {return fFitCharge;}
		double GetEnergy(Option_t* opt=0) const;
		
		// Not strictly "doppler" but consistent
		inline double GetDoppler(double beta, TVector3 *vec=0) { 
			if(vec==0) {
				vec = GetBeamDirection();
			}
			TVector3 pos = GetPosition();
			pos.SetTheta(130.*TMath::Pi()/180.);	
			double costhe=TMath::Cos(pos.Angle(*vec));
			double e=this->GetEnergy();
			double gamma = 1/(sqrt(1-pow(beta,2)));
			
			return ((e+511-beta*costhe*sqrt(e*(e+1022)))*gamma)-511;;
		}
		
		unsigned int GetAddbackSize(){
			if(fAddBackSegments.size()==fAddBackEnergy.size())return fAddBackEnergy.size();
			return 0;
		}
		
		double GetAddbackEnergy(unsigned int i){
			if(i< GetAddbackSize())return fAddBackEnergy[i];
			return 0;
		}
		short GetAddbackSegment(unsigned int i){
			if(i< GetAddbackSize())return fAddBackSegments[i];
			return 0;
			
		}
		
	private:
		Double_t GetDefaultDistance() const { return 0.0; }
		
		std::vector<short> fAddBackSegments;
		std::vector<double> fAddBackEnergy; //probably not needed after development finished
		TTransientBits<UChar_t> fSiLiHitBits;

		Double_t    fTimeFit;
		Double_t    fSig2Noise;
		Double_t    fSmirnov;
		Double_t    fFitCharge;
		Double_t    fFitBase;
		

/// \cond CLASSIMP
		ClassDef(TSiLiHit,9);
/// \endcond
};
/*! @} */
#endif
