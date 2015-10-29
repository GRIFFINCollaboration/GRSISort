#ifndef TSILI_H
#define TSILI_H

#include <cstdio>
#include <iostream>

#include "TDetector.h"

#include "TSiLiHit.h"

class TSiLi: public TDetector  {

	public:
		TSiLi();
		~TSiLi();

		void AddFragment(TFragment*, MNEMONIC*);
		void BuildHits() {} //no need to build any hits, everything already done in AddFragment

		void Print(Option_t *opt="") const;
		void Clear(Option_t *opt="");


		Short_t GetMultiplicity() const { return fSiLiHits.size(); }
		TSiLiHit *GetSiLiHit(const int& i);

		TVector3 GetPosition(int segment);

	private:
		std::vector<TSiLiHit> fSiLiHits;


		ClassDef(TSiLi,2);
};


#endif
