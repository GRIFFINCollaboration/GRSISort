#ifndef TSHARCHIT_H
#define TSHARCHIT_H


#include "Globals.h"

#include <cstdio>
#include <utility>

//#include "TChannel.h"
#include "TVector3.h" 
#include "TObject.h" 
#include "TRandom.h"
#include "Rtypes.h"
#include "TMath.h"
#include "TGRSIDetectorHit.h"


class TSharcHit : public TGRSIDetectorHit 	{
	public:
		TSharcHit();
		virtual ~TSharcHit();
      TSharcHit(const TSharcHit&);

	private:

		UShort_t		front_strip;	  //
		Int_t	    	front_address; //   Should be Address.
		Int_t			 front_charge;	  //	
		//double 	 front_cfd;		  //	
		
		UShort_t		back_strip;		  //
		Int_t	   	  back_address;  //   Should be Address.
		Int_t			  back_charge;	  //
		//double		back_cfd;	  // 		

		UShort_t		detectornumber;  //
		Int_t			  pad_charge;		  //

      Double_t		d_energy_front;  //
      Double_t		d_time_front;	  //
      Double_t		d_energy_back;  //
      Double_t		d_time_back;	  //
      //Double_t		d_cfd;		  //

      Double_t		p_energy;	//			pad only;
 		Double_t		p_time;		//			pad only;
		Int_t       p_address;  //
 		//Double_t		p_cfd;		//

		//Double_t		energy;		//
		//Double_t		time;			//

	public:

		virtual void Clear(Option_t* = "");	//!
		virtual void Print(Option_t* = "");	//!
      virtual void Copy(TSharcHit&) const;        //!

		//void CalibrateSharcHit(TChannel *); //!
		//static bool Compare(TSharcHit *lhs,TSharcHit *rhs); //!

		inline Double_t GetDeltaE()	{	return d_energy_front;	};	//!
		inline Double_t GetDeltaT()	{	return d_time_front;		};	//!

		inline Double_t GetDeltaFrontE()	{	return d_energy_front;	};	//!
		inline Double_t GetDeltaFrontT()	{	return d_time_front;		};	//!
		inline Double_t GetDeltaBackE()	{	return d_energy_back;	};	//!
		inline Double_t GetDeltaBackT()	{	return d_time_back;		};	//!

      inline Int_t GetFrontAddress()   {  return front_address; } //!
      inline Int_t GetBackAddress()   {  return back_address; } //!
      inline Int_t GetPadAddress()   {  return p_address; } //!

		inline Double_t GetPadE()		{	return p_energy;	};	//!
		inline Double_t GetPadT()		{	return p_time;		};	//!

		inline Int_t GetDetectorNumber()	{ return detectornumber;	} //!
		//std::pair<int,int>	GetPixel()	{ return std::make_pair(front_strip,back_strip);	}	//!

		inline Int_t    GetFrontCharge() 	  {	return	front_charge;	}	//!
		       Double_t GetFrontChgHeight() ;	//!
		inline Double_t GetFrontChgDbl() 	  {	return	(Double_t)front_charge + gRandom->Uniform();	}	//!
		inline Int_t    GetBackCharge()		  {	return	back_charge;	}	//!
		       Double_t GetBackChgHeight()  ;	//!
		inline Double_t GetBackChgDbl()		  {	return	(Double_t)back_charge + gRandom->Uniform();	}	//!
		inline Int_t    GetPadCharge()		  {	return	pad_charge;		}	//!
		       Double_t GetPadChgHeight()		;	//!
		
		inline UShort_t GetFrontStrip()	{	return	front_strip;	}	//!
		inline UShort_t GetBackStrip()	{	return	back_strip;		}	//!

		inline Double_t GetEnergy() {	return (p_energy>0) ? (p_energy + d_energy_front) : d_energy_front ;}
		inline Double_t GetTime()	 {	return d_time_front; }

      TVector3 GetPosition() const {return TVector3();}
			
		//inline Double_t GetFrontCFD()		{	return front_cfd;	}	//!
		//inline Double_t GetBackCFD()		{	return back_cfd;	}	//!

      Double_t PadEnergyCal(); //!
      Double_t FrontEnergyCal(); //!

		Double_t GetThetaDeg(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0) { return GetTheta(Xoff,Yoff,Zoff)*TMath::RadToDeg(); } ; //! 
		Double_t GetTheta(double Xoff = 0.0, double Yoff = 0.0, double Zoff = 0.0); //! 

		inline void SetDeltaFrontE(const Double_t &tenergy)	{	d_energy_front = tenergy;	}	//!
		inline void SetDeltaFrontT(const Double_t &ttime)		{	d_time_front   = ttime;	}		//!
		inline void SetDeltaBackE(const Double_t &tenergy)	   {	d_energy_back  = tenergy;	}	//!
		inline void SetDeltaBackT(const Double_t &ttime)		{	d_time_back    = ttime;	}		//!


		inline void SetPadE(const Double_t &tenergy)		{	p_energy = tenergy;	}	//!
		inline void SetPadT(const Double_t &ttime)		{	p_time = ttime;	}		//!

		//void SetFrontCFD(const double &cfd)		{	front_cfd = cfd;	} //!
		//void SetBackCFD(const double &cfd)		{	back_cfd = cfd;		} //!

      //	inline void SetPosition(const TVector3 &pos)	{	position = pos;		} //!

		//void SetPixel(const int &front, const  int &back)	{front_strip = front; back_strip = back;}	//!
		inline void SetDetector(const UShort_t &detector)	{	detectornumber = detector;	}						//!
	
		inline void SetFrontCharge(const Int_t &charge)	{ front_charge = charge;}	//!
		inline void SetBackCharge(const Int_t &charge)	{ back_charge = charge;}	//!
		inline void SetPadCharge(const Int_t &charge)	{ pad_charge = charge;}		//!


      inline void SetFrontAddress(const Int_t &tmp) {front_address = tmp; } //!
      inline void SetBackAddress(const Int_t &tmp)  {back_address  = tmp; } //!
      inline void SetPadAddress(const Int_t &tmp)   {p_address     = tmp; } //!


		//inline void SetEnergy(const Double_t &tenergy)	{	energy = tenergy; }
		//inline void SetTime(const Double_t &ttime)	{	time = ttime; }

		inline void SetFrontStrip(const UShort_t &strip)	{	front_strip = strip; } //!
		inline void SetBackStrip(const UShort_t &strip)		{	back_strip = strip; } //!

		//void SetFrontWave(std::vector<int> &wave)	{ front_strip_wave = wave;	} 
		//void SetBackWave(std::vector<int> &wave)	{ back_strip_wave = wave;	} 
		//void SetPadWave(std::vector<int> &wave)	{ pad_wave = wave;	} 
		
	ClassDef(TSharcHit,4)
};




#endif
