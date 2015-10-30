#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

#include <cstdio>
#include <cmath>
#ifndef __CINT__
#include <tuple>
#endif

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"
#include "TPulseAnalyzer.h"

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TGRSIDetectorHit.h"

class TTigressHit : public TGRSIDetectorHit {
  public:
	TTigressHit();
	TTigressHit(const TTigressHit&);
   TTigressHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}
	virtual ~TTigressHit();

  private:
	UInt_t   fCrystal;              //!
	UShort_t fFirstSegment;        
	Float_t    fFirstSegmentCharge; //!

	Double_t fEnergy;

	std::vector<TGRSIDetectorHit> fSegments;
	std::vector<TGRSIDetectorHit> fBgos;

	Double_t    fTimeFit;
	Double_t    fSig2Noise;

	//double doppler;

	//need to do sudo tracking to build addback.
	TVector3 fLastHit;                //!
#ifndef __CINT__
	std::tuple<int,int,int> fLastPos; //!
#endif

	static TVector3 fBeam;

  public:
	void SetHit() {}
	/////////////////////////		/////////////////////////////////////
	void SetCore(TGRSIDetectorHit& core)		  { Copy(core);	} 					//!
	void AddSegment(TGRSIDetectorHit& seg) 	  { fSegments.push_back(seg);	}	//!
	void AddBGO(TGRSIDetectorHit& bgo) 		     { fBgos.push_back(bgo);	}	   //!

	void SetCrystal()	                   { fCrystal = GetCrystal(); SetFlag(TGRSIDetectorHit::kIsSubDetSet,true); }		//!
	void SetCrystal(UShort_t crystal)    { fCrystal = crystal;      SetFlag(TGRSIDetectorHit::kIsSubDetSet,true); }		//!
	void SetInitalHit(const int &i)		 { fFirstSegment = i; }				//!
	Bool_t IsCrystalSet() const          { return IsSubDetSet();}

	/////////////////////////		/////////////////////////////////////
	int GetCrystal() const;	          //{	return crystal;			}		//!
	inline int GetInitialHit()		               {	return fFirstSegment;	}			//!
	
	void SetWavefit(TFragment&);
	inline Double_t GetSignalToNoise()	  { return fSig2Noise;	} //!
	inline Double_t GetFitTime()			  { return fTimeFit;	} //!

	inline double GetDoppler(double beta,TVector3 *vec=0) { 
		if(vec==0) {
			vec = &fBeam;
		}
		double tmp = 0;
		double gamma = 1/(sqrt(1-pow(beta,2)));
		tmp = this->GetEnergy()*gamma *(1 - beta*TMath::Cos(this->GetPosition().Angle(*vec)));
		return tmp;
	}

	inline int GetSegmentMultiplicity()		        {	return fSegments.size();	}	//!
	inline int GetBGOMultiplicity()			        {	return fBgos.size();	}		//!
	inline TGRSIDetectorHit& GetSegment(const int &i)	  {	return fSegments.at(i);	}	      //!
	inline TGRSIDetectorHit& GetBGO(const int &i)	     {	return fBgos.at(i);	}	         //!
	inline TGRSIDetectorHit& GetCore()                   {	return *this;	}	       		   //!

	void CheckFirstHit(int charge,int segment);								                  //!

	static bool Compare(TTigressHit lhs, TTigressHit rhs);	      //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
	static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);	//!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
		
	void SumHit(TTigressHit*);                                        //!
	TVector3 GetLastHit()	{return fLastHit;}                      //!
#ifndef __CINT__
	inline std::tuple<int,int,int> GetLastPosition() {return fLastPos;} //!
#endif                         

   private:
    TVector3 GetChannelPosition(Double_t dist=110.0) const;


  public:
	virtual void Clear(Option_t *opt = "");		                      //!
	virtual void Copy(TObject&) const;                             //!
	virtual void Print(Option_t *opt = "") const;       		                //!

	ClassDef(TTigressHit,1)
};
#endif
