#ifndef TCSMHIT_H
#define TCSMHIT_H

#include "Globals.h"

#include <stdio.h>
#include <utility>

#include "TChannel.h"
#include "TVector3.h" 

#include <TGRSIDetectorHit.h>

class TCSMHit : public TGRSIDetectorHit 	{
	public:
		TCSMHit();
		~TCSMHit();

	private:

		UShort_t			hor_d_strip;	//
		Int_t      	   hor_d_charge;	//	
		Int_t 			hor_d_cfd;		//	

		UShort_t			ver_d_strip;		//
		Int_t      	   ver_d_charge;	//
		Int_t  			ver_d_cfd;		// 		

		UShort_t			hor_e_strip;	//
		Int_t         	hor_e_charge;	//	
		Int_t 			hor_e_cfd;		//	

		Short_t			ver_e_strip;		//
		Int_t	         ver_e_charge;	//
		Int_t 			ver_e_cfd;		// 		

	  	Double_t hor_d_energy;		//		
	  	Double_t ver_d_energy;		//		
  		Double_t hor_d_time;			//		
  		Double_t ver_d_time;			//		
		TVector3 d_position;	//

  		Double_t hor_e_energy;		//			
  		Double_t ver_e_energy;		//			
  		Double_t hor_e_time;			//		
  		Double_t ver_e_time;			//		

		TVector3 e_position;	//

		//double energy;	//
		//double time;	//

		UShort_t   detectornumber;	//

		//TVector3 position;  //

	public:

		virtual void Clear(Option_t* = "");	//!
		virtual void Print(Option_t* = "");	//!

		//static bool Compare(TCSMHit *lhs,TCSMHit *rhs); //!

		Double_t GetEHorizontalEnergy()	{	return hor_e_energy;	};	//!
		Double_t GetEVerticalEnergy()	   {	return ver_e_energy;	};	//!
		Double_t GetEEnergy()	         { return hor_e_energy; } //!
		Double_t GetETime()		         {	return hor_e_time;		}	//!
		
		Double_t GetDHorizontalEnergy()	{	return hor_d_energy;	};	//!
		Double_t GetDVerticalEnergy()    {	return ver_d_energy;	};	//!
		Double_t GetDEnergy()	         { return ver_d_energy; } //!
		Double_t GetDTime()		         {	return ver_d_time;		}	//!

		UShort_t GetDetectorNumber()	   { return detectornumber;	} //!

		Int_t GetEHorizontalCharge() 	   {	return	hor_e_charge;}	//!
		Int_t GetEVerticalCharge()		   {	return	ver_e_charge;	}	//!

		Int_t GetDHorizontalCharge() 	   {	return	hor_d_charge;}	//!
		Int_t GetDVerticalCharge()		   {	return	ver_d_charge;	}	//!


		Double_t GetEHorizontalTime()	   {	return hor_e_time;	}	//!
		Double_t GetEVerticalTime()		{	return ver_e_time;	}	//!

		Double_t GetDHorizontalTime()	   {	return hor_d_time;	}	//!
		Double_t GetDVerticalTime()		{	return ver_d_time;	}	//!



		Int_t GetEHorizontalStrip(){	return	hor_e_strip;	}	//!
		Int_t GetEVerticalStrip()	{	return	ver_e_strip;	}	//!
		
		Int_t GetDHorizontalStrip(){	return	hor_d_strip;	}	//!
		Int_t GetDVerticalStrip()	{	return	ver_d_strip;	}	//!
	
		Int_t GetEHorizontalCFD()	{	return hor_e_cfd;	}	//!
		Int_t GetEVerticalCFD()		{	return ver_e_cfd;	}	//!

		Int_t GetDHorizontalCFD()	{	return hor_d_cfd;	}	//!
		Int_t GetDVerticalCFD()		{	return ver_d_cfd;	}	//!

		TVector3 GetEPosition()	{	return e_position;	}	//!
		TVector3 GetDPosition()	{	return d_position;	}	//!



		TVector3 GetPosition()	{	return d_position;	 }	//!
		Double_t GetEnergy()		{return ver_e_energy + hor_d_energy;} //!
		Double_t GetTime()		{return ver_d_time;} //!

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////

		inline void  SetDetectorNumber(const Int_t &tempnum)			{ detectornumber = tempnum;	} //!

		inline void SetEHorizontalCharge(const Int_t temp) 	{hor_e_charge = temp;}	//!
		inline void SetEVerticalCharge(const Int_t temp)	{ver_e_charge  = temp;}	//!

		inline void SetDHorizontalCharge(const Int_t temp) 	{hor_d_charge = temp;}	//!
		inline void SetDVerticalCharge(const Int_t temp)	{ver_d_charge  = temp;}	//!

		inline void SetEHorizontalStrip(const Int_t temp)	{hor_e_strip  = temp;}	//!
		inline void SetEVerticalStrip(const Int_t temp)		{ver_e_strip   = temp;}	//!

		inline void SetDHorizontalStrip(const Int_t temp)	{hor_d_strip  = temp;}	//!
		inline void SetDVerticalStrip(const Int_t temp)		{ver_d_strip   = temp;}	//!
	
		inline void SetEHorizontalCFD(const Int_t tempd)	{ hor_e_cfd = tempd;	}	//!
		inline void SetEVerticalCFD(const Int_t tempd)	{ ver_e_cfd  = tempd;	}	//!

		inline void SetDHorizontalCFD(const Int_t tempd)	{ hor_d_cfd = tempd;	}	//!
		inline void SetDVerticalCFD(const Int_t tempd)	{ ver_d_cfd  = tempd;	}	//!


		inline void SetEHorizontalTime(const Int_t tempd)	{ hor_e_time = tempd;	}	//!
		inline void SetEVerticalTime(const Int_t tempd)	  { ver_e_time  = tempd;	}	//!

		inline void SetDHorizontalTime(const Int_t tempd)	{ hor_d_time = tempd;	}	//!
		inline void SetDVerticalTime(const Int_t tempd)	  { ver_d_time  = tempd;	}	//!



		inline void SetEPosition(TVector3 tempp)	{ e_position = tempp;	}	//!
		inline void SetDPosition(TVector3 tempp)	{ d_position = tempp;	}	//!





		inline void SetDHorizontalEnergy(const Double_t tempd) { hor_d_energy = tempd; }
		inline void SetDVerticalEnergy(const Double_t tempd) { ver_d_energy = tempd; }

		inline void SetEHorizontalEnergy(const Double_t tempd) { hor_e_energy = tempd; }
		inline void SetEVerticalEnergy(const Double_t tempd) { ver_e_energy = tempd; }

		//void SetHorizontalWave(std::vector<int> &wave)	{ hor_strip_wave = wave;	} 
		//void SetVerticalWave(std::vector<int> &wave)	{ ver_strip_wave = wave;	} 
		//void SetPadWave(std::vector<int> &wave)	{ pad_wave = wave;	} 


		
	ClassDef(TCSMHit,2)
};




#endif
