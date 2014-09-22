#ifndef GRIFFINHIT_H
#define GRIFFINHIT_H

#include <cstdio>
#include <cmath>

#include <TFragment.h>
#include <TChannel.h>
#include <TCrystalHit.h>

#include <TVector3.h>


#include <TGRSIDetectorHit.h>


class TGriffinHit : public TGRSIDetectorHit {
	public:
		TGriffinHit();
		~TGriffinHit();

	private:
		UShort_t detector;
		UShort_t crystal;

      UInt_t address;

      Int_t filter;

      Int_t charge_lowgain;
      Int_t charge_highgain;
      Int_t cfd;
      Double_t energy_lowgain;
      Double_t energy_highgain;
      Double_t time;

		TVector3 position;

		std::vector<TCrystalHit> bgo;

      std::vector<UShort_t> waveform;
   
	public:

		/////////////////////////		/////////////////////////////////////
		void SetBGO(TCrystalHit &temp)		 { bgo.push_back(temp);	}    //!

		inline void SetDetectorNumber(const int &x)  { detector = x; }   //!
		inline void SetCrystalNumber(const int &x)   { crystal = x;  }   //!

      inline void SetFilerPattern(const int &x)    { filter = x;   }   //! 

      inline void SetChargeLow(const int &x)       { charge_lowgain  = x;   }   //!
      inline void SetChargeHigh(const int &x)      { charge_highgain = x;   }   //!

      inline void SetCfd(const int &x)             { cfd    = x;   }   //!
      inline void SetEnergyHigh(const Double_t &x)     { energy_highgain = x;   }   //!
      inline void SetEnergyLow(const Double_t &x)     { energy_lowgain = x;   }   //!

      inline void SetTime(const Double_t &x)       { time   = x;   }   //!

      inline void SetAddress(const UInt_t &x)      { address = x; } //!

      inline void SetWaveform(std::vector<UShort_t> x) { waveform = x; } //!

		void SetPosition(double dist =110);                                				  //!

		
		/////////////////////////		/////////////////////////////////////
		inline UShort_t GetDetectorNumber()	     {	return detector; }  //!
		inline UShort_t GetCrystalNumber()	     {	return crystal;  }  //!
	
		inline Int_t    GetChargeLow()			  {	return charge_lowgain;	  }  //!
		inline Int_t    GetChargeHigh()			  {	return charge_highgain;	  }  //!
      inline Int_t    GetCfd()                 {   return cfd;      }  //!
      inline Double_t GetEnergyLow()		     {	return energy_lowgain;   }  //!
      inline Double_t GetEnergyHigh()		     {	return energy_highgain;   }  //!
		inline Double_t GetTime()			        {	return time;     }  //!
		inline TVector3 GetPosition()	           {	return position; }  //!

      inline UInt_t   GetAddress()             {   return address; } //!

      inline Int_t    GetFiterPatter()         {   return filter;   }  //!

		inline int GetBGOMultiplicity()			   {	return bgo.size();	}		      //!
		inline TCrystalHit *GetBGO(const int &i)	{	return &bgo.at(i);	}	        //!

      bool   InFilter(Int_t);  //!

      static bool CompareEnergy(TGriffinHit*,TGriffinHit*);
      void Add(TGriffinHit*);     

	public:
		virtual void Clear(Option_t *opt = "");		                   //!
		virtual void Print(Option_t *opt = "");		                   //!

	ClassDef(TGriffinHit,1)
};




#endif
