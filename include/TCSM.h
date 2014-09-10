#ifndef TSHARC_H
#define TSHARC_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <map>
#ifndef __CINT__
#include <tuple>
#include <iterator>
#include <algorithm>
#endif
#include <utility>
#include <set>


#include "TFragment.h"
#include "TChannel.h"

#include "TCSMData.h"
#include "TCSMHit.h"

#include <TMath.h>
#include <TVector3.h>
#include <TObject.h>


#ifndef PI
#define PI                       (TMath::Pi())
#endif


class TCSM :  public TObject 	{
	public:
		TCSM();
		~TCSM();

	public: 
		virtual void Clear(Option_t * = "");		//!
		virtual void Print(Option_t * = "");		//!
		void BuildHits(TCSMData *cd = 0,  Option_t * = "");			//!

		TCSMHit *GetHit(int i)		{return &csm_hits.at(i);}	//->
		Short_t GetMultiplicity()	{return csm_hits.size();}	//->

	   static TVector3 GetPosition(int detector, char pos, int horizontalstrip, int verticalstrip, double X=0.00, double Y=0.00, double Z=0.00);	//! 

      void FillData(TFragment*,TChannel*,MNEMONIC*);

	private: 
		TCSMData *data;
		std::vector <TCSMHit> csm_hits;

		//int CombineHits(TCSMHit*,TCSMHit*,int,int);				//!
		//void RemoveHits(std::vector<TCSMHit>*,std::set<int>*);	//!

		static int fCfdBuildDiff; //!   // largest acceptable time difference between events (clock ticks)  (50 ns)

        
   ClassDef(TCSM,2)  // CSM Analysis structure
};


#endif
