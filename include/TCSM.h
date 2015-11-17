#ifndef TCSM_H
#define TCSM_H

/** \addtogroup Detectors
 *  @{
 */

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

#include "TMath.h"
#include "TVector3.h"

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"
#include "TDetector.h"
#include "TCSMHit.h"

class TCSM : public TDetector {
	public:
		TCSM();
		virtual ~TCSM();

	public: 
		virtual void Clear(Option_t * = "");
		virtual void Print(Option_t * = "") const;

		TCSMHit *GetCSMHit(const int& i);	//->
		Short_t GetMultiplicity() const	{return fCsmHits.size();}	//->

		static TVector3 GetPosition(int detector, char pos, int horizontalstrip, int verticalstrip, double X=0.00, double Y=0.00, double Z=0.00);

		void AddFragment(TFragment*, MNEMONIC*);
		void BuildHits();

	private: 
		std::map<int16_t, std::vector<std::vector<std::vector<std::pair<TFragment, MNEMONIC> > > > > fFragments; //!<!
		std::vector<TCSMHit> fCsmHits;
		double fAlmostEqualWindow;

		static int fCfdBuildDiff; //!<! largest acceptable time difference between events (clock ticks)  (50 ns)

		void BuildVH(std::vector<std::vector<std::pair<TFragment, MNEMONIC> > >&, std::vector<TCSMHit>&);
		void BuilddEE(std::vector<std::vector<TCSMHit> >&,std::vector<TCSMHit>&);
		void OldBuilddEE(std::vector<TCSMHit> &,std::vector<TCSMHit> &,std::vector<TCSMHit> &);
		void MakedEE(std::vector<TCSMHit> &DHitVec,std::vector<TCSMHit> &EHitVec,std::vector<TCSMHit> &BuiltHits);
		TCSMHit MakeHit(std::pair<TFragment, MNEMONIC>&, std::pair<TFragment, MNEMONIC>&);
		TCSMHit MakeHit(std::vector<std::pair<TFragment, MNEMONIC> >&,std::vector<std::pair<TFragment, MNEMONIC> >&);
		TCSMHit CombineHits(TCSMHit, TCSMHit);
		void RecoverHit(char, std::pair<TFragment, MNEMONIC>&, std::vector<TCSMHit>&);
		bool AlmostEqual(int, int);
		bool AlmostEqual(double,double);

		//int CombineHits(TCSMHit*,TCSMHit*,int,int);				//!<!
		//void RemoveHits(std::vector<TCSMHit>*,std::set<int>*);	//!<!

/// \cond CLASSIMP
		ClassDef(TCSM,5)  // CSM Analysis structure
/// \endcond
};
/*! @} */
#endif
