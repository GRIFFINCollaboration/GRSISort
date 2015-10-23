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

#include "TMath.h"
#include "TVector3.h"
#include "TClonesArray.h"

#include "TGRSIDetectorHit.h"

class TTigressHit : public TGRSIDetectorHit {
	public:
		TTigressHit();
		TTigressHit(const TTigressHit&);
		virtual ~TTigressHit();

	private:
		//UShort_t detector;
		UInt_t   crystal;              //!
		UShort_t first_segment;        
		Float_t    first_segment_charge; //!

      Double_t fEnergy;

		TCrystalHit core;
		//std::vector<TCrystalHit> segment;
      TClonesArray segment;
		//std::vector<TCrystalHit> bgo;
      TClonesArray bgo;

		//double doppler;

		//need to do sudo tracking to build addback.
		TVector3 lasthit;                //!
		#ifndef __CINT__
		std::tuple<int,int,int> lastpos; //!
		#endif

      static TVector3 beam;

	public:
      void SetHit() {}
		/////////////////////////		/////////////////////////////////////
		void SetCore(TCrystalHit &temp)		  { core = temp;	} 					//!
		void AddSegment(TCrystalHit &temp);	  //{ segment.push_back(temp);	}		//!
		void AddBGO(TCrystalHit &temp);		  //{ bgo.push_back(temp);	}			//!

		//void SetDetectorNumber(const int &i) { detector = i;	} 				//!
		void SetCrystal()	                   { crystal = GetCrystal(); SetFlag(TGRSIDetectorHit::kIsSubDetSet,true); }		//!
		void SetInitalHit(const int &i)		 { first_segment = i; }				//!
      Bool_t IsCrystalSet() const {return IsSubDetSet();}

//		void SetPosition(const TVector3 &p)  { position = p;	}					//!
		//void SetDoppler(const double &d)	   { doppler = d;	}					//!

		/////////////////////////		/////////////////////////////////////
		//inline int GetDetectorNumber()	     {	return detector;		}			//!
		int GetCrystal() const;	          //{	return crystal;			}		//!
		inline int GetInitialHit()		               {	return first_segment;	}			//!
	
		inline int GetCharge()			                  {	return core.GetCharge();	}		//!
		inline double GetEnergy(Option_t *opt ="")const	{	return core.GetEnergy();	}		//!
		inline double GetTime(Option_t *opt ="") const	{	return core.GetTime();		}		//!
		inline double GetTimeCFD()                      {  return core.GetCfd(); } //!
      TVector3 GetPosition(Double_t dist = 110.0) const; //!
		//inline double   GetDoppler()	       {	return doppler;				}		//!


		inline double GetDoppler(double beta,TVector3 *vec=0) { 
			if(vec==0) {
				vec = &beam;
			}
			double tmp = 0;
         double gamma = 1/(sqrt(1-pow(beta,2)));
         tmp = this->GetEnergy()*gamma *(1 - beta*TMath::Cos(this->GetPosition().Angle(*vec)));
			return tmp;
		}

		inline int GetSegmentMultiplicity()		        {	return segment.GetEntries();	}	//!
		inline int GetBGOMultiplicity()			        {	return bgo.GetEntries();	}		//!
		inline TCrystalHit *GetSegment(const int &i)	  {	return (TCrystalHit*)segment.At(i);	}	      //!
		inline TCrystalHit *GetBGO(const int &i)	     {	return (TCrystalHit*)bgo.At(i);	}	         //!
		inline TCrystalHit *GetCore()                  {	return &core;	}	       		   //!

		void CheckFirstHit(int charge,int segment);								                  //!

		static bool Compare(TTigressHit lhs, TTigressHit rhs);	      //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
		static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);	//!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
		
		void SumHit(TTigressHit*);                                        //!
		TVector3 GetLastHit()	{return lasthit;}                      //!
		#ifndef __CINT__
		inline std::tuple<int,int,int> GetLastPosition() {return lastpos;} //!
		#endif                         

	public:
		virtual void Clear(Option_t *opt = "");		                      //!
		virtual void Copy(TObject&) const;                             //!
      virtual void Print(Option_t *opt = "") const;       		                //!

	ClassDef(TTigressHit,1)
};




#endif
