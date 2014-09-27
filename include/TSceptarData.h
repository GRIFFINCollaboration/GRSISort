#ifndef TSCEPTARDATA_H
#define TSCEPTARDATA_H

#include <cstdlib>
#include <cstdio>
#include <vector>

#include <Globals.h>
#include <TFragment.h>
#include <TChannel.h>

#include <TGRSIDetectorData.h>

class TSceptarData : public TGRSIDetectorData {

	private:
		std::vector<UShort_t> fDet_Nbr;		//!
		std::vector<UInt_t>   fDet_Address;		//!
      std::vector<Double_t> fDet_Eng;		//!
		std::vector<Int_t>    fDet_Chg;		//!
		std::vector<Int_t>    fDet_CFD;		//!
		std::vector<Long_t>   fDet_Time;		//!
		std::vector<std::vector<Short_t> >fDet_Wave;	//!

      static bool fSetWave;    //!  
		static bool fIsSet;      //!
   public:
		TSceptarData();					//!
		virtual ~TSceptarData();			//!

		static void Set(bool flag=true) { fIsSet=flag; } //!
		static bool IsSet() { return fIsSet; }           //!

		virtual void Clear(Option_t *opt = "");		//!
		virtual void Print(Option_t *opt = "");		//!

	private:
		inline void SetDetNumber(const UShort_t  &DetNumber)	   {fDet_Nbr.push_back(DetNumber);      }	//!
		inline void SetDetAddress(const UInt_t  &DetAddress)	   {fDet_Address.push_back(DetAddress); }	//!
      inline void SetDetEnergy(const Double_t  &DetEnergy)	   {fDet_Eng.push_back(DetEnergy);      }	//!
		inline void SetDetCharge(const Int_t &DetCharge)	      {fDet_Chg.push_back(DetCharge);      }	//!
		inline void SetDetCFD(const Int_t &DetCFD)	            {fDet_CFD.push_back(DetCFD); }	//!	
		inline void SetDetTime(const Long_t    &DetTime)	      {fDet_Time.push_back(DetTime);       }	//!

		inline void SetWave(const std::vector<Short_t> &DetWave)	{fDet_Wave.push_back(DetWave);} //!

   public:
		inline void SetDet(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
		   if(!frag || !channel || !mnemonic) return;
         for(int x=0;x<frag->Charge.size();x++) {
     		   SetDetNumber(frag->ChannelNumber);
	   	   SetDetAddress(frag->ChannelAddress);
            SetDetEnergy(channel->CalibrateENG(frag->Charge.at(x)));
		      SetDetCharge(frag->Charge.at(x));
		      SetDetCFD(frag->Cfd.at(x));	 
		      SetDetTime(frag->GetTimeStamp());	
            if(fSetWave)
   		      SetWave(frag->wavebuffer);
         }
      }; //! 

		inline UShort_t GetDetNumber(const unsigned int &i)	   {return fDet_Nbr.at(i);}	//!
		inline UInt_t   GetDetAddress(const unsigned int &i)	   {return fDet_Address.at(i);}	//!
      inline Double_t GetDetEnergy(const unsigned int &i)	   {return fDet_Eng.at(i);}	//!
		inline Int_t    GetDetCharge(const unsigned int &i)	   {return fDet_Chg.at(i);}	//!
		inline Int_t    GetDetCFD(const unsigned int &i)	      {return fDet_CFD.at(i);}	//!
		inline Long_t   GetDetTime(const unsigned int &i)	      {return fDet_Time.at(i);}	//!

		inline std::vector<Short_t> GetDetWave(const unsigned int &i)	{return fDet_Wave.at(i);}	//!

		inline unsigned int GetMultiplicity()		{return fDet_Nbr.size();}	//!
		
		ClassDef(TSceptarData,0) //! // TSceptarData structure
};


#endif





