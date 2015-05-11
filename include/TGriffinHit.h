#ifndef GRIFFINHIT_H
#define GRIFFINHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
//#include "TCrystalHit.h"

#include "TVector3.h"

#include "TSceptarHit.h"

#include "TGRSIDetectorHit.h"


class TGriffinHit : public TGRSIDetectorHit {
	public:
		TGriffinHit();
		virtual ~TGriffinHit();

	private:
      Int_t filter;
      Int_t ppg;

      Int_t charge_lowgain;
      Int_t charge_highgain;
      Int_t cfd;
      Long_t time;
   
	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter = x;   }                  //! 
      inline void SetPPG(const int &x)             { ppg = x;   }                     //! 

      inline void SetChargeLow(const int &x)       { charge_lowgain  = x;   }         //!
      inline void SetChargeHigh(const int &x)      { charge_highgain = x;   }         //!

      inline void SetCfd(const int &x)             { cfd    = x;   }                  //!
      inline void SetTime(const Long_t &x)         { time   = x;   }                  //!

		void SetPosition(double dist =110);                                				  //!

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPatter() const         {   return filter;   }          //!
      inline Int_t    GetPPG() const                  {   return ppg;   }             //!

      inline Int_t    GetChargeLow() const			  {	return charge_lowgain;	  }  //!
		inline Int_t    GetChargeHigh() const			  {	return charge_highgain;	  }  //!

      inline Int_t    GetCfd() const                 {   return cfd;      }           //!
		inline Long_t   GetTime() const 			        {	return time;     }           //!

      /////////////////////////  Required Functions ///////////////////////////
      double GetEnergy();                                                             //!
      double GetTime();                                                               //!

      /////////////////////////  Recommended Functions/////////////////////////



		/////////////////////////  TChannel Helpers /////////////////////////////////////
      Int_t GetDetector();                                                           //!
      Int_t GetCrystal();                                                            //!

		/////////////////////////		/////////////////////////////////////
		//inline UShort_t GetDetectorNumber() const	     {	return detector; }  //!
		//inline UShort_t GetCrystalNumber() const	     {	return crystal;  }  //!

      //inline UShort_t GetArrayNumber() { return( 4*(GetDetectorNumber()-1)+(GetCrystalNumber()+1)); } //!
      // returns a number 1-64 ( 1 = Detector 1 blue;  64 =  Detector 16 white; ) 

      //inline Int_t    GetChargeLow() const			  {	return charge_lowgain;	  }  //!
		//inline Int_t    GetChargeHigh() const			  {	return charge_highgain;	  }  //!
      //inline Int_t    GetCfd() const                 {   return cfd;      }  //!
      //inline Double_t GetEnergyLow() const		     {	return energy_lowgain;   }  //!
      //inline Double_t GetEnergyHigh() const		     {	return energy_highgain;   }  //!
		//inline Long_t   GetTime() const 			        {	return time;     }  //!

      //inline Int_t    GetFilterPatter() const         {   return filter;   }  //!
      //inline Int_t    GetPPG() const                  {   return ppg;   }  //!
      //inline std::vector<Short_t> GetWaveForm() const{   return waveform;} //!

//      bool   InFilter(Int_t);  //!

//      static bool CompareEnergy(TGriffinHit*,TGriffinHit*);  //!
//      void Add(TGriffinHit*);    //! 

      //Bool_t BremSuppressed(TSceptarHit*);

	public:
		virtual void Clear(Option_t *opt = "");		                   //!
		virtual void Print(Option_t *opt = "") const; 	                   //!

	ClassDef(TGriffinHit,2)
};




#endif
