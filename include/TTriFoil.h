#ifndef TTRIFOIL_PHASE_H
#define TTRIFOIL_PHASE_H


#include <vector>
#include <iostream>
#include <stdio.h>

#include "TDetector.h"

#include "TFragment.h"
#ifndef __CINT__
#include "TTriFoilData.h"
#else
class TTriFoilData;
#endif

class TTriFoil :  public TDetector {
	
	public:
		TTriFoil();
		virtual ~TTriFoil();
      TTriFoil(const TTriFoil& rhs);

		std::vector<Short_t> GetWave() { return tf_wave;	};
		bool Beam() const{return beam;};
		int TBeam() const {return tbeam;};
	
		bool HasWave() const { return !tf_wave.empty(); };
      time_t GetTimeStamp() const {return timestamp;}

		void BuildHits(TDetectorData *data=0, Option_t * = "");	//!
		void FillData(TFragment*,TChannel*,MNEMONIC*);	//!

		void Clear(Option_t *opt = ""); 	//!
		void Print(Option_t *opt = "") const; 	//!
      void Copy(TTriFoil &rhs) const;

		
	
	private:
		TTriFoilData *data;		//!

		std::vector<Short_t> tf_wave;
		time_t timestamp;
		bool beam;
		int tbeam;
		
	ClassDef(TTriFoil,2)

};



#endif
