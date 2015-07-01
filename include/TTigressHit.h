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

#include "TGRSIDetectorHit.h"


class TTigressHit : public TGRSIDetectorHit {
	public:
		TTigressHit();
		~TTigressHit();

	private:
		UShort_t detector;
		UShort_t crystal;
		UShort_t first_segment;
		Int_t    first_segment_charge;

		TCrystalHit core;
		std::vector<TCrystalHit> segment;
		std::vector<TCrystalHit> bgo;

		//double doppler;

		//need to do sudo tracking to build addback.
		TVector3 lasthit;  //!
      double   lastenergy; //!
		#ifndef __CINT__
		std::tuple<int,int,int> lastpos; //!
		#endif

	public:

		/////////////////////////		/////////////////////////////////////
		void SetCore(TCrystalHit &temp)		  { core = temp;	} 					//!
		void SetSegment(TCrystalHit &temp)	  { segment.push_back(temp);	}  //!
		void SetBGO(TCrystalHit &temp)		  { bgo.push_back(temp);	}     //!

		void SetDetectorNumber(const int &i)  { detector = i;	}              //!
		void SetCrystalNumber(const int &i)	  { crystal = i; }					//!
		void SetInitialHit(const int &i)      { first_segment = i; }         //!

		void SetPosition(const TVector3 &p)  { position = p;	}					//!
		//void SetDoppler(const double &d)	   { doppler = d;	}					//!
		
		/////////////////////////		/////////////////////////////////////
		inline int GetDetectorNumber()	      {  return detector;		}			//!
		inline int GetCrystalNumber()	         {  return crystal;			}			//!
		inline int GetInitialHit()		         {  return first_segment;	}			//!
	
      inline int GetFirstSegmentCharge()     {  return first_segment_charge; }
		inline int GetCharge()			         {  return core.GetCharge();	}		//!
		inline double GetEnergy()		         {  return core.GetEnergy();	}		//!
		inline double GetTime()			         {  return core.GetTime();		}		//!
      inline double GetTimeCFD()             {  return core.GetCfd(); } //!
		//inline double   GetDoppler()	       {	return doppler;				}		//!

		inline double GetDoppler(double beta,TVector3 *vec=0) { 
			bool madevec = false;
			if(vec==0) {
				vec = new TVector3;
				vec->SetXYZ(0,0,1);
				madevec = true;
			}
			double tmp = 0;
			//if(beta != 0.00)  {
      double gamma = 1/(sqrt(1-pow(beta,2)));
      tmp = this->GetEnergy()*gamma *(1 - beta*TMath::Cos(this->GetPosition().Angle(*vec)));
			//}
			if(madevec) 
				delete vec;
			return tmp;
		}

		inline int GetSegmentMultiplicity()		      { return segment.size();	}	//!
		inline TCrystalHit *GetSegment(const int &i)	{ return &segment.at(i);	}	//!

		inline int GetBGOMultiplicity()			      { return bgo.size();	}		      //!
		inline TCrystalHit *GetBGO(const int &i)	   { return &bgo.at(i);	}	        //!
      inline bool Suppress()                       { return GetCore()->Suppress(); }
      inline void SetSuppress(bool flag = true)    { GetCore()->SetSuppress(flag); }

		inline TCrystalHit *GetCore()								{	return &core;	}	       		  //!

		void CheckFirstHit(int charge,int segment);								    //!

		static bool Compare(TTigressHit lhs, TTigressHit rhs);	      //!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
		static bool CompareEnergy(TTigressHit lhs, TTigressHit rhs);	//!     { return (lhs.GetDetectorNumber() < rhs.GetDetectorNumber()); }
		
      
		void Add(TTigressHit*);                                       //!
		TVector3 GetLastHit()	{return lasthit;}                       //!
		#ifndef __CINT__
		inline std::tuple<int,int,int> GetLastPosition() {return lastpos;} //!
		#endif                         

	public:
		virtual void Clear(Option_t *opt = "");		                   //!
		virtual void Print(Option_t *opt = "");		                   //!

	ClassDef(TTigressHit,1)
};




#endif
