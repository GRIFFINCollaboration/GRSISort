#ifndef TSHARCHIT_H
#define TSHARCHIT_H

#include <cstdio>
#include <utility>

//#include "TChannel.h"
#include <TVector3.h> 
#include <TObject.h> 
#include <TRandom.h>
#include <Rtypes.h>

#include "Globals.h"

class TSharcHit : public TObject 	{
	public:
		TSharcHit();
		~TSharcHit();

	private:

		UShort_t		front_strip;	  //
		UShort_t		front_channelId; //   Should be Address.
		Int_t			front_charge;	  //	
		//double 	front_cfd;		  //	
		
		UShort_t		back_strip;		  //
		UShort_t		back_channelId;  //   Should be Address.
		Int_t			back_charge;	  //
		//double			back_cfd;	  // 		

		UShort_t		detectornumber;  //
		Int_t			pad_charge;		  //

	  	Double_t		d_energy;		  //		
  		Double_t		d_time;			  //		
	  	//Double_t		d_cfd;			  //		

  		Double_t		p_energy;	//			pad only;
 		Double_t		p_time;		//			pad only;
 		//Double_t		p_cfd;		//

		//Double_t		energy;		//
		//Double_t		time;			//

		TVector3 	position;  	//

	public:

		virtual void Clear(Option_t* = "");	//!
		virtual void Print(Option_t* = "");	//!

		//void CalibrateSharcHit(TChannel *); //!
		//static bool Compare(TSharcHit *lhs,TSharcHit *rhs); //!

		inline Double_t GetDeltaE()	{	return d_energy;	};	//!
		inline Double_t GetDeltaT()	{	return d_time;		};	//!
		inline Double_t GetPadE()		{	return p_energy;	};	//!
		inline Double_t GetPadT()		{	return p_time;		};	//!

		inline Int_t GetDetectorNumber()	{ return detectornumber;	} //!
		//std::pair<int,int>	GetPixel()	{ return std::make_pair(front_strip,back_strip);	}	//!

		inline Int_t    GetFrontCharge() 	{	return	front_charge;	}	//!
		inline Double_t GetFrontChgDbl() 	{	return	(Double_t)front_charge + gRandom->Uniform();	}	//!
		inline Int_t    GetBackCharge()		{	return	back_charge;	}	//!
		inline Double_t GetBackChgDbl()		{	return	(Double_t)back_charge + gRandom->Uniform();	}	//!
		inline Int_t GetPadCharge()		{	return	pad_charge;		}	//!
		
		inline UShort_t GetFrontStrip()	{	return	front_strip;	}	//!
		inline UShort_t GetBackStrip()	{	return	back_strip;		}	//!

		inline UShort_t GetFrontChanId()	{	return	front_channelId;	}	//!
		inline UShort_t GetBackChanId()	{	return	back_channelId;	}	//!
		
		inline Double_t GetEnergy() {	return (p_energy>0) ? (p_energy + d_energy) : d_energy ;}
		inline Double_t GetTime()	 {	return d_time; }
			
		//inline Double_t GetFrontCFD()		{	return front_cfd;	}	//!
		//inline Double_t GetBackCFD()		{	return back_cfd;	}	//!

		TVector3 GetPosition()	{	return position;	}	//!

		inline void SetDeltaE(const Double_t &tenergy)	{	d_energy = tenergy;	}	//!
		inline void SetDeltaT(const Double_t &ttime)		{	d_time = ttime;	}		//!
		inline void SetPadE(const Double_t &tenergy)		{	p_energy = tenergy;	}	//!
		inline void SetPadT(const Double_t &ttime)		{	p_time = ttime;	}		//!

		//void SetFrontCFD(const double &cfd)		{	front_cfd = cfd;	} //!
		//void SetBackCFD(const double &cfd)		{	back_cfd = cfd;		} //!

		inline void SetPosition(const TVector3 &pos)	{	position = pos;		} //!

		//void SetPixel(const int &front, const  int &back)	{front_strip = front; back_strip = back;}	//!
		inline void SetDetector(const UShort_t &detector)	{	detectornumber = detector;	}						//!
	
		inline void SetFrontCharge(const Int_t &charge)	{ front_charge = charge;}	//!
		inline void SetBackCharge(const Int_t &charge)	{ back_charge = charge;}	//!
		inline void SetPadCharge(const Int_t &charge)	{ pad_charge = charge;}		//!

		//inline void SetEnergy(const Double_t &tenergy)	{	energy = tenergy; }
		//inline void SetTime(const Double_t &ttime)	{	time = ttime; }

		inline void SetFrontChanId(const UShort_t &chan)	{	front_channelId = chan;	} //!
		inline void SetBackChanId(const UShort_t &chan)		{	back_channelId = chan;		} //!

		//void SetFrontWave(std::vector<int> &wave)	{ front_strip_wave = wave;	} 
		//void SetBackWave(std::vector<int> &wave)	{ back_strip_wave = wave;	} 
		//void SetPadWave(std::vector<int> &wave)	{ pad_wave = wave;	} 
		
	ClassDef(TSharcHit,3)
};




#endif
