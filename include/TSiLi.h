
#ifndef TSILI_H
#define TSILI_H

#include <cstdio>

#include <TGRSIDetector.h>

#include "TSiLiData.h"
#include "TSiLiHit.h"

class TSiLi: public TGRSIDetector	{

	public:
		TSiLi();
		~TSiLi();

		void BuildHits();		
		void Print();
		void Clear();
		
		
		int GetMultiplicity()	{	return	sili_hits.size();	}
		SiLiHit GetHit(int i) 	{	return	sili_hits.at(i);	}

		TVector3 GetPosition(int segment);

//	private:
	
		std::vector<TSiLiHit> sili_hits;


	ClassDef(TSiLi,2);
	
};


#endif
