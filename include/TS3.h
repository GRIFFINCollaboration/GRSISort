#ifndef TS3_H
#define TS3_H

#include <iostream>

#include "TDetector.h"
#include "TS3Hit.h"

class TS3 : public TDetector {
	public:
		TS3();
		~TS3();

		void AddFragment(TFragment*, MNEMONIC*);
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		TS3Hit* GetS3Hit(const int& i);  
		Short_t GetMultiplicity() { return fS3Hits.size(); }

		TVector3 GetPosition(int front, int back);

      virtual void Clear(Option_t *opt = "all");		     //!
      virtual void Print(Option_t *opt = "") const;		  //!

	private:
		std::vector<TS3Hit> fS3Hits;

		///for geometery
		static int fRingNumber;          //!
		static int fSectorNumber;        //!

		static double fOffsetPhi;        //!
		static double fOuterDiameter;    //!
		static double fInnerDiameter;    //!
		static double fTargetDistance;   //!

		ClassDef(TS3,2)
};

#endif
