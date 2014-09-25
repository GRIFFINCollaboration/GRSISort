#ifndef TTRIFOIL_PHASE_H
#define TTRIFOIL_PHASE_H


#include <vector>
#include <iostream>
#include <stdio.h>

#include "TGRSIDetector.h"

#include "TFragment.h"
#include <TTriFoilData.h>

using namespace std;

class TTriFoil :  public TGRSIDetector {
	
	public:
		TTriFoil();
		~TTriFoil();

		std::vector<Short_t> GetWave() { return tf_wave;	};
		bool Beam(){return beam;};
		int TBeam(){return tbeam;};
	
		bool HasWave() { return !tf_wave.empty(); };

		void BuildHits(TGRSIDetectorData *data=0, Option_t * = "");	//!
		void FillData(TFragment*,TChannel*,MNEMONIC*);	//!

		void Clear(Option_t *opt = ""); 	//!
		void Print(Option_t *opt = ""); 	//!

		
	
	private:
		TTriFoilData *data;		//!

		std::vector<Short_t> tf_wave;
		time_t timestamp;
		bool beam;
		int tbeam;
		
	ClassDef(TTriFoil,2)

};



#endif
