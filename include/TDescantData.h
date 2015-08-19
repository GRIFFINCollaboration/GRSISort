#ifndef TDESCANTDATA_H
#define TDESCANTDATA_H

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TDescantData : public TGRSIDetectorData {

	private:
		std::vector<UShort_t> fDet_Nbr;		//!
		std::vector<UInt_t>   fDet_Address;		//!
      std::vector<Double_t> fDet_Eng;		//!
		std::vector<Float_t>    fDet_Chg;		//!
		std::vector<Int_t>    fDet_CFD;		//!
		std::vector<Int_t>    fDet_Zc;		//!
		std::vector<Int_t>    fDet_CcShort;		//!
		std::vector<Int_t>    fDet_CcLong;		//!
		std::vector<Long_t>   fDet_Time;		//!
		std::vector<std::vector<Short_t> >fDet_Wave;	//!

      static bool fSetWave;    //!  
		static bool fIsSet;      //!
   public:
		TDescantData();					//!
		virtual ~TDescantData();			//!

		static void Set(bool flag=true) { fIsSet=flag; } //!
		static bool IsSet() { return fIsSet; }           //!

		void Clear(Option_t *opt = "");		//!
		void Print(Option_t *opt = "") const;		//!

	private:
		inline void SetDetNumber(const UShort_t  &DetNumber)	   {fDet_Nbr.push_back(DetNumber);      }	//!
		inline void SetDetAddress(const UInt_t  &DetAddress)	   {fDet_Address.push_back(DetAddress); }	//!
      inline void SetDetEnergy(const Double_t  &DetEnergy)	   {fDet_Eng.push_back(DetEnergy);      }	//!
		inline void SetDetCharge(const Float_t &DetCharge)	      {fDet_Chg.push_back(DetCharge);      }	//!
		inline void SetDetCFD(const Int_t &DetCFD)	            {fDet_CFD.push_back(DetCFD); }	//!	
		inline void SetDetZc(const Int_t &DetZc)	            {fDet_Zc.push_back(DetZc); }	//!	
		inline void SetDetCcShort(const Int_t &DetCcShort)	            {fDet_CcShort.push_back(DetCcShort); }	//!	
		inline void SetDetCcLong(const Int_t &DetCcLong)	            {fDet_CcLong.push_back(DetCcLong); }	//!	
		inline void SetDetTime(const Long_t    &DetTime)	      {fDet_Time.push_back(DetTime);       }	//!

		inline void SetWave(const std::vector<Short_t> &DetWave)	{fDet_Wave.push_back(DetWave);} //!

   public:
		inline void SetDet(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
		   if(!frag || !channel || !mnemonic) return;
         
         for(int x=0;x<frag->Charge.size();x++) {
     		   SetDetNumber(mnemonic->arrayposition);
	   	   SetDetAddress(frag->ChannelAddress);
            //SetDetEnergy(channel->CalibrateENG(frag->Charge.at(x)));
            SetDetEnergy(frag->GetEnergy(x));
		      SetDetCharge(frag->GetCharge(x));
		      SetDetCFD(frag->Cfd.at(x));	 
				if(x < frag->Zc.size()) {
					SetDetZc(frag->Zc.at(x));	 
				} else {
					SetDetZc(0);
				}
				if(x < frag->ccShort.size()) {
					SetDetCcShort(frag->ccShort.at(x));	 
				} else {
					SetDetCcShort(0);
				}
				if(x < frag->ccLong.size()) {
					SetDetCcLong(frag->ccLong.at(x));	 
				} else {
					SetDetCcLong(0);
				}
		      SetDetTime(frag->GetTimeStamp());	
            if(fSetWave)
   		      SetWave(frag->wavebuffer);
         }
      }; //! 

		inline UShort_t GetDetNumber(const unsigned int &i)	   {return fDet_Nbr.at(i);}	//!
		inline UInt_t   GetDetAddress(const unsigned int &i)	   {return fDet_Address.at(i);}	//!
      inline Double_t GetDetEnergy(const unsigned int &i)	   {return fDet_Eng.at(i);}	//!
		inline Float_t  GetDetCharge(const unsigned int &i)	   {return fDet_Chg.at(i);}	//!
		inline Int_t    GetDetCFD(const unsigned int &i)	      {return fDet_CFD.at(i);}	//!
		inline Int_t    GetDetZc(const unsigned int &i)	         {return fDet_Zc.at(i);}	//!
		inline Int_t    GetDetCcShort(const unsigned int &i)	   {return fDet_CcShort.at(i);}	//!
		inline Int_t    GetDetCcLong(const unsigned int &i)	   {return fDet_CcLong.at(i);}	//!
		inline Long_t   GetDetTime(const unsigned int &i)	      {return fDet_Time.at(i);}	//!

		inline std::vector<Short_t> GetDetWave(const unsigned int &i)	{if(fSetWave) { return fDet_Wave.at(i);} else {std::vector<Short_t> w; return w;}}	//!

		inline unsigned int GetMultiplicity()		{return fDet_Nbr.size();}	//!
		
		//ClassDef(TDescantData,0) //! // TDescantData structure
};


#endif





