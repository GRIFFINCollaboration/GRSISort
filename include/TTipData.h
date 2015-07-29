#ifndef TTIPDATA_H
#define TTIPDATA_H

#ifndef __CINT__

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TTipData : public TGRSIDetectorData {

	private:
		std::vector<Double_t> fDet_Pid;		//!
		std::vector<UShort_t> fDet_Nbr;		//!
		std::vector<UInt_t>   fDet_Address;		//!
      std::vector<Double_t> fDet_Eng;		//!
		std::vector<Float_t>  fDet_Chg;		//!
		std::vector<Int_t>    fDet_CFD;		//!
		std::vector<Long_t>   fDet_Time;		//!
		std::vector<std::vector<Short_t> >fDet_Wave;	//!

      static bool fSetWave;    //!  
		static bool fIsSet;      //!
   public:
		TTipData();					//!
		virtual ~TTipData();			//!

		static void Set(bool flag=true) { fIsSet=flag; } //!
		static bool IsSet() { return fIsSet; }           //!

		void Clear(Option_t *opt = "");		      //!
		void Print(Option_t *opt = "") const;		//!

	private:

		inline void SetPID(const Double_t &DetPID)					{fDet_Pid.push_back(DetPID);			 } //!
		inline void SetDetNumber(const UShort_t  &DetNumber)	   {fDet_Nbr.push_back(DetNumber);      }	//!
		inline void SetDetAddress(const UInt_t  &DetAddress)	   {fDet_Address.push_back(DetAddress); }	//!
      inline void SetDetEnergy(const Double_t  &DetEnergy)	   {fDet_Eng.push_back(DetEnergy);      }	//!
		inline void SetDetCharge(const Float_t &DetCharge)	      {fDet_Chg.push_back(DetCharge);      }	//!
		inline void SetDetCFD(const Int_t &DetCFD)	            {fDet_CFD.push_back(DetCFD); 			 }	//!	
		inline void SetDetTime(const Long_t    &DetTime)	      {fDet_Time.push_back(DetTime);       }	//!

		inline void SetWave(const std::vector<Short_t> &DetWave)	{fDet_Wave.push_back(DetWave);} //!

   public:
		inline void SetDet(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
		   if(!frag || !channel || !mnemonic) return;
   
			   SetDetNumber(mnemonic->arrayposition);
	   	   SetDetAddress(frag->ChannelAddress);
            SetDetEnergy(channel->CalibrateENG(frag->Charge.at(0)));
		      SetDetCharge(frag->Charge.at(0));
            SetDetCFD(frag->Cfd.at(0));	 
  	         SetDetTime(frag->Zc.at(0));	
            if(fSetWave)
 		        SetWave(frag->wavebuffer);

				// Check CsI
				if(mnemonic->subsystem.compare(0,1,"W")==0) {
					SetPID(DoPID(frag->wavebuffer));
				}
				else SetPID(-1);

      }; //! 

		inline Double_t GetPID(const unsigned int &i)				{return fDet_Pid.at(i);}	//!
		inline UShort_t GetDetNumber(const unsigned int &i)	   {return fDet_Nbr.at(i);}	//!
		inline UInt_t   GetDetAddress(const unsigned int &i)	   {return fDet_Address.at(i);}	//!
      inline Double_t GetDetEnergy(const unsigned int &i)	   {return fDet_Eng.at(i);}	//!
		inline Float_t  GetDetCharge(const unsigned int &i)	   {return fDet_Chg.at(i);}	//!
		inline Int_t    GetDetCFD(const unsigned int &i)	      {return fDet_CFD.at(i);}	//!
		inline Long_t   GetDetTime(const unsigned int &i)	      {return fDet_Time.at(i);}	//!

		Double_t DoPID(std::vector<Short_t> waveform);

		inline std::vector<Short_t> GetDetWave(const unsigned int &i)	{return fDet_Wave.at(i);}	//!

		inline unsigned int GetMultiplicity()		{return fDet_Nbr.size();}	//!
		
};

#endif

#endif
