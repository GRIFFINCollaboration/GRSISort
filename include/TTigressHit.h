#ifndef TIGRESSHIT_H
#define TIGRESSHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>
#if !defined (__CINT__) && !defined (__CLING__)
#include <tuple>
#endif

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TGRSIDetectorHit.h"

class TTigressHit : public TGRSIDetectorHit {
  public:
    TTigressHit();
    TTigressHit(const TTigressHit&);
    TTigressHit(const TFragment& frag) : TGRSIDetectorHit(frag) {}
    virtual ~TTigressHit();

  private:
	 UInt_t   fCrystal;              //!<!
	 UShort_t fFirstSegment;        
	 Float_t  fFirstSegmentCharge; //!<!

    std::vector<TGRSIDetectorHit> fSegments;
    std::vector<TGRSIDetectorHit> fBgos;

    Double_t fTimeFit;
    Double_t fSig2Noise;

	//need to do sudo tracking to build addback.
	TVector3 fLastHit;                //!<!
#if !defined (__CINT__) && !defined (__CLING__)
	std::tuple<int,int,int> fLastPos; //!<!
#endif

  public:
	void SetHit() {}
	/////////////////////////		/////////////////////////////////////
	void SetCore(TGRSIDetectorHit& core)		  { Copy(core);	} 					//!<!
	void AddSegment(TGRSIDetectorHit& seg) 	  { fSegments.push_back(seg);	}	//!<!
	void AddBGO(TGRSIDetectorHit& bgo) 		    { fBgos.push_back(bgo);	}	   //!<!

  //int SetCrystal(char color);
  //int SetCrystal(int crynum);
	void SetInitalHit(const int &i)		 { fFirstSegment = i; }				//!<!
	//Bool_t IsCrystalSet() const          { return IsSubDetSet();}

	/////////////////////////		/////////////////////////////////////
	int GetCrystal() const;	          //{	return crystal;			}		//!<!
	//int GetCrystal();
	int GetInitialHit() const           {	return fFirstSegment;	}			//!<!
	
	void SetWavefit(TFragment&);
	void SetWavefit();
	inline Double_t GetSignalToNoise()	  { return fSig2Noise;	} //!<!
	inline Double_t GetFitTime()			  { return fTimeFit;	} //!<!

	int GetArrayNumber()	{	
														int number = 4*(GetDetector()-1) + GetCrystal(); 
														return number;
													}

	inline double GetDoppler(double beta, TVector3 *vec=0) { 
		if(vec==0) {
			vec = GetBeamDirection();
		}
		double tmp = 0;
		double gamma = 1/(sqrt(1-pow(beta,2)));
		tmp = this->GetEnergy()*gamma *(1 - beta*TMath::Cos(this->GetPosition().Angle(*vec)));
		return tmp;
	}



	int GetSegmentMultiplicity()		  const      { return fSegments.size(); }	//!<!
	int GetNSegments()		            const      { return fSegments.size(); }	//!<!
	int GetBGOMultiplicity()			    const      { return fBgos.size();     }   //!<!
	int GetNBGOs()			              const      { return fBgos.size();     }   //!<!
	using TGRSIDetectorHit::GetSegment;
	TGRSIDetectorHit& GetSegment(int &i)       { return fSegments.at(i);  }   //!<!
	TGRSIDetectorHit& GetBGO( int &i)	         { return fBgos.at(i);	     }   //!<!
	TGRSIDetectorHit& GetCore()                { return *this;	           }   //!<!
	
  TGRSIDetectorHit GetSegment(int &i) const { return fSegments.at(i);  }   //!<!
	TGRSIDetectorHit GetBGO( int &i)	   const { return fBgos.at(i);	     }   //!<!
	TGRSIDetectorHit GetCore()          const { return *this;	           }   //!<!

	void CheckFirstHit(int charge,int segment);								               //!<!

	static bool Compare(TTigressHit lhs, TTigressHit rhs);	      //!<!
	static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);	//!<!
		
	void SumHit(TTigressHit*);                                      //!<!
	TVector3 GetLastHit() { return fLastHit; }                      //!<!
#if !defined (__CINT__) && !defined (__CLING__)
	inline std::tuple<int,int,int> GetLastPosition() {return fLastPos;} //!<!
#endif                         

  private:
    TVector3 GetChannelPosition(Double_t dist=110.0) const;

  public:
	virtual void Clear(Option_t *opt = "");		                      //!<!
	virtual void Copy(TObject&) const;                             //!<!
	virtual void Print(Option_t *opt = "") const;       		                //!<!

/// \cond CLASSIMP
	ClassDef(TTigressHit,1)
/// \endcond
};
/*! @} */
#endif
