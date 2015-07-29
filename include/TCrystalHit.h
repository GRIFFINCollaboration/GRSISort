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
		inline double GetEnergy() const 	{ return local_energy;	}	//!

      TVector3 GetPosition() const {return TVector3();}
		inline void SetSegment(const int &seg) { segment = seg;   }       //!
		inline void SetEnergy(const double &e)	{	local_energy = e;	}	//!

		//inline void SetWave(const std::vector<int> &w)	{	wave = w;	} //!
		//inline std::vector<int> *GetWave()	{	return &wave;	}	  //!


	ClassDef(TCrystalHit,1)
};

#endif
