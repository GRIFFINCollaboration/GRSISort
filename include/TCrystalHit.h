#ifndef TCRYSTALHIT_H
#define TCRYSTALHIT_H

#include <cstdio>
#include <vector>

#include "TGRSIDetectorHit.h"


class TCrystalHit : public TGRSIDetectorHit	{

	public:
		TCrystalHit();
		TCrystalHit(const TCrystalHit&);
		virtual ~TCrystalHit();

	private: 
		int segment;		//
		//int charge;		  //

		double local_energy;	//
		//double time;		//
		//double cfd;		  //

		//std::vector<int> wave;	//!

	public:
		//void SetHit() {};

		virtual void Clear(Option_t *opt = "");		      //!
		virtual void Print(Option_t *opt = "") const;		//!
      virtual void Copy(TCrystalHit&) const;             //!

		inline int    GetSegment()       { return segment;}   //!
		//inline int    GetCharge()	{	return charge; }	//!		
		inline double GetEnergy() const 	{ return local_energy;	}	//!
//		inline double GetTime(Option_t *opt="") const { return time;	}	//!
		//inline double GetCfd()		{	return cfd;	}	   //!

      TVector3 GetPosition() const {return TVector3();}
		inline void SetSegment(const int &seg) { segment = seg;   }       //!
		//inline void SetCharge(const int &chg)	{	charge = chg;	}	//!
		inline void SetEnergy(const double &e)	{	local_energy = e;	}	//!
		//inline void SetTime(const double &t)	{	time = t;	}	//!
		//inline void SetCfd(const double &c)	{	cfd = c;	}	//!

		//inline void SetWave(const std::vector<int> &w)	{	wave = w;	} //!
		//inline std::vector<int> *GetWave()	{	return &wave;	}	  //!


	ClassDef(TCrystalHit,1)
};

#endif
