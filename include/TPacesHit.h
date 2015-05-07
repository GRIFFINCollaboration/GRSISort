// Author: Peter C. Bender

#ifndef TPACESHIT_H
#define TPACESHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"


#include "TGRSIDetectorHit.h"


class TPacesHit : public TGRSIDetectorHit {
	public:
		TPacesHit();
		virtual ~TPacesHit();

	private:
		UShort_t crystal; //The paces crystal number

      Int_t filter;     //The Master Filter Pattern

      Int_t ppg;        //The current PPG pattern

      Int_t charge_lowgain;  //The charge collected from the low gain 
      Int_t charge_highgain; //The charge collected from the high gain
      Int_t cfd;             //The cfd time
      Double_t energy_lowgain; //The calibrated energy from the low gain
      Double_t energy_highgain; //The calibrated energy from the high gain
      Long_t time;               //The time stamp of the hit

      std::vector<Short_t> waveform;  //! waveform of PACES hit
   
	public:

		/////////////////////////		/////////////////////////////////////

		inline void SetCrystalNumber(const int &x)   { crystal = x;  }   //!

      inline void SetFilterPattern(const int &x)   { filter = x;   }   //! 

      inline void SetPPG(const int &x)             { ppg = x;   }      //! 

      inline void SetChargeLow(const int &x)       { charge_lowgain  = x;   }   //!
      inline void SetChargeHigh(const int &x)      { charge_highgain = x;   }   //!

      inline void SetCfd(const int &x)             { cfd    = x;   }   //!
      inline void SetEnergyHigh(const Double_t &x)     { energy_highgain = x;   }   //!
      inline void SetEnergyLow(const Double_t &x)     { energy_lowgain = x;   }   //!

      inline void SetTime(const Long_t &x)       { time   = x;   }   //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!
		
		/////////////////////////		/////////////////////////////////////
		inline UShort_t GetCrystalNumber() const	     {	return crystal;  }  //!
	
		inline Int_t    GetChargeLow() const			  {	return charge_lowgain;	  }  //!
		inline Int_t    GetChargeHigh() const			  {	return charge_highgain;	  }  //!
      inline Int_t    GetCfd() const                 {   return cfd;      }  //!
      inline Double_t GetEnergyLow() const		     {	return energy_lowgain;   }  //!
      inline Double_t GetEnergyHigh() const		     {	return energy_highgain;   }  //!
		inline Long_t   GetTime() const 			        {	return time;     }  //!

      inline Int_t    GetFilterPatter() const         {   return filter;   }  //!
      inline Int_t    GetPPG() const                  {   return ppg;   }  //!
      inline std::vector<Short_t> GetWaveForm() const{   return waveform;} //!

      bool   InFilter(Int_t);  //!

      static bool CompareEnergy(TPacesHit*,TPacesHit*);  //!
      void Add(TPacesHit*);    //! 

	public:
		virtual void Clear(Option_t *opt = "");		                   //!
		virtual void Print(Option_t *opt = "") const; 	                   //!

	ClassDef(TPacesHit,2) //Holds information for individual PACES hits
};

#endif
