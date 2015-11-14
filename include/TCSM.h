#ifndef TCSM_H
#define TCSM_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <map>
#include <iostream>
#if !defined (__CINT__) && !defined (__CLING__)
#include <tuple>
#include <iterator>
#include <algorithm>
#endif
#include <utility>
#include <set>


#include "TFragment.h"
#include "TChannel.h"

#if !defined (__CINT__) && !defined (__CLING__)
#include "TCSMData.h"
#else
class TCSMData;
#endif
#include "TCSMHit.h"

#include "TMath.h"
#include "TVector3.h"

#include "TDetector.h"

#ifndef PI
#define PI                       (TMath::Pi())
#endif

class TCSM :  public TDetector 	{
	public:
		TCSM();
		virtual ~TCSM();

	public: 
		virtual void Clear(Option_t * = "");		//!
		virtual void Print(Option_t * = "") const;		//!
		void BuildHits(TDetectorData *cd = 0,  Option_t * = "");			//!

		TCSMHit *GetCSMHit(const int& i);	//->
		Short_t GetMultiplicity() const	{return csm_hits.size();}	//->

	   static TVector3 GetPosition(int detector, char pos, int horizontalstrip, int verticalstrip, double X=0.00, double Y=0.00, double Z=0.00);	//! 

      void FillData(TFragment*,TChannel*,MNEMONIC*);

	private: 
		TCSMData *data;                             //!
		std::vector <TCSMHit> csm_hits;
		void BuildVH(std::vector<int> &,std::vector<int> &,std::vector<TCSMHit> &,TCSMData*);
		void BuilddEE(std::vector<TCSMHit> &,std::vector<TCSMHit> &,std::vector<TCSMHit> &);
		void OldBuilddEE(std::vector<TCSMHit> &,std::vector<TCSMHit> &,std::vector<TCSMHit> &);
		void MakedEE(std::vector<TCSMHit> &DHitVec,std::vector<TCSMHit> &EHitVec,std::vector<TCSMHit> &BuiltHits);
		TCSMHit MakeHit(int, int, TCSMData*);
		TCSMHit MakeHit(std::vector<int> &,std::vector<int> &, TCSMData*);
		TCSMHit CombineHits(TCSMHit d_hit,TCSMHit e_hit);
		void RecoverHit(char, int, TCSMData *, std::vector<TCSMHit> &);
		bool AlmostEqual(int, int);
		bool AlmostEqual(double,double);
		
		
		double AlmostEqualWindow;

		//int CombineHits(TCSMHit*,TCSMHit*,int,int);				//!
		//void RemoveHits(std::vector<TCSMHit>*,std::set<int>*);	//!

		static int fCfdBuildDiff; //!   // largest acceptable time difference between events (clock ticks)  (50 ns)

        
   ClassDef(TCSM,3)  // CSM Analysis structure
};


#endif
