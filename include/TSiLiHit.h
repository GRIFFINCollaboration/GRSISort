#ifndef TSILIHIT_H
#define TSILIHIT_H

#include <cstdio>
#include <utility>

#include "TVector3.h" 
#include "TObject.h" 

class TSiLiHit : public TGRSIDetectorHit {
	public:
			TSiLiHit();
			~TSiLiHit();

			TVector3 position;

			double energy;
			unsigned int charge;
			double time;
			double cfd;

			int segment;

			void Clear();
			void Print();

			int GetSegment()		{	return segment;	}
			double GetEnergy()	{	return energy;	}
			double GetCharge()	{	return charge;	}
			double GetTime()		{	return time;		}

	
	ClassDef(TSiLiHit,2);

};



#endif
