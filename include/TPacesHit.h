#ifndef TPACESHIT_H
#define TPACESHIT_H

#include <cstdio>
#include <cmath>

#include <TFragment.h>
#include <TChannel.h>
#include <TCrystalHit.h>

#include <TVector3.h>


#include <TGRSIDetectorHit.h>


class TPacesHit : public TGRSIDetectorHit {
	public:
		TPacesHit();
		virtual ~TPacesHit();

	private:
		UShort_t crystal;

      UInt_t address;

      Int_t filter;

      Int_t ppg;

      Int_t charge_lowgain;
      Int_t charge_highgain;
      Int_t cfd;
      Double_t energy_lowgain;
      Double_t energy_highgain;
      Long_t time;

      std::vector<Short_t> waveform;  //!
   
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

      inline void SetAddress(const UInt_t &x)      { address = x; } //!

      inline void SetWaveform(std::vector<Short_t> x) { waveform = x; } //!
		
		/////////////////////////		/////////////////////////////////////
		inline UShort_t GetCrystalNumber() const	     {	return crystal;  }  //!
	
		inline Int_t    GetChargeLow() const			  {	return charge_lowgain;	  }  //!
		inline Int_t    GetChargeHigh() const			  {	return charge_highgain;	  }  //!
      inline Int_t    GetCfd() const                 {   return cfd;      }  //!
      inline Double_t GetEnergyLow() const		     {	return energy_lowgain;   }  //!
      inline Double_t GetEnergyHigh() const		     {	return energy_highgain;   }  //!
		inline Long_t   GetTime() const 			        {	return time;     }  //!

      inline UInt_t   GetAddress() const             {   return address; } //!

      inline Int_t    GetFilterPatter() const         {   return filter;   }  //!
      inline Int_t    GetPPG() const                  {   return ppg;   }  //!
      inline std::vector<Short_t> GetWaveForm() const{   return waveform;} //!

      bool   InFilter(Int_t);  //!

      static bool CompareEnergy(TPacesHit*,TPacesHit*);  //!
      void Add(TPacesHit*);    //! 

	public:
		virtual void Clear(Option_t *opt = "");		                   //!
		virtual void Print(Option_t *opt = "") const; 	                   //!

	ClassDef(TPacesHit,2)
};




#endif
