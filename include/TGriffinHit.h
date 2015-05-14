#ifndef GRIFFINHIT_H
#define GRIFFINHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
//#include "TCrystalHit.h"

#include "TVector3.h"

//#include "TSceptarHit.h"

#include "TGRSIDetectorHit.h"


class TGriffinHit : public TGRSIDetectorHit {
	public:
		TGriffinHit();
		TGriffinHit(const TGriffinHit&);
		virtual ~TGriffinHit();

	private:
      Int_t filter;
      Int_t ppg;
      UInt_t crystal; //!

   //flags
   private:
      Bool_t is_crys_set; //!

	public:
		/////////////////////////  Setters	/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter = x;   }                  //! 
      inline void SetPPG(const int &x)             { ppg = x;   }                     //! 
      //void SetHit();
      virtual double GetTime(Option_t *opt = "") const;                                 //!

//		void SetPosition(double dist =110);   //!
      TVector3 GetPosition(Double_t dist = 110.0); //!

		/////////////////////////  Getters	/////////////////////////////////////
      inline Int_t    GetFilterPattern() const         {   return filter;   }          //!
      inline Int_t    GetPPG() const                  {   return ppg;   }             //!

   //          Int_t    GetCharge(Option_t *opt ="low") const;                          //!
		//inline Long_t   GetTime() const 			        {	return time;     }           //!

      /////////////////////////  Required Functions ///////////////////////////
  //    double GetEnergy(Option_t *opt ="low") const;                             //!

      /////////////////////////  Recommended Functions/////////////////////////



		/////////////////////////  TChannel Helpers /////////////////////////////////////
      const UInt_t GetCrystal()  const;                                                //!
      UInt_t SetCrystal();

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

      bool   InFilter(Int_t);  //!

//      static bool CompareEnergy(TGriffinHit*,TGriffinHit*);  //!
//      void Add(TGriffinHit*);    //! 
      //Bool_t BremSuppressed(TSceptarHit*);

	public:
		virtual void Clear(Option_t *opt = "");		 //!
		virtual void Print(Option_t *opt = "") const; //!
      virtual void Copy(TGriffinHit&) const;        //!

	ClassDef(TGriffinHit,2);
};




#endif
