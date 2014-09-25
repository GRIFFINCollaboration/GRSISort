#ifndef TGRIFFINDATA_H
#define TGRIFFINDATA_H

#include <cstdlib>
#include <cstdio>
#include <vector>

#include <Globals.h>
#include <TFragment.h>
#include <TChannel.h>

#include <TGRSIDetectorData.h>
#include <TGriffinCoreData.h>

class TGriffinData : public TGRSIDetectorData {

	private:
      std::vector<TGriffinCoreData> fCore; //!
		//std::vector<UShort_t> fClover_Nbr;		//!
		//std::vector<UShort_t> fCore_Nbr;		//!
		//std::vector<UInt_t>   fCore_Address;		//!
      //std::vector<Bool_t>   fCore_IsHighGain; //!
      //std::vector<Double_t> fCore_Eng;		//!
		//std::vector<Int_t>    fCore_Chg;		//!
		//std::vector<Double_t> fCore_TimeCFD;		//!
		//std::vector<Double_t> fCore_TimeLED;		//!
		//std::vector<Double_t> fCore_Time;		//!
		//std::vector<std::vector<UShort_t> >fCore_Wave;	//!


      //std::vector<Int_t>        fCore_NbrHits; //!
      //std::vector<Int_t>        fCore_MidasId; //!


		static bool fIsSet; //!

	public:
		TGriffinData();					//!
		virtual ~TGriffinData();			//!

		static void Set(bool flag=true) { fIsSet=flag; } //!
		static bool IsSet() { return fIsSet; }           //!

		virtual void Clear(Option_t *opt = "");		//!
		virtual void Print(Option_t *opt = "");		//!

		//inline void SetCloverNumber(const UShort_t &CloverNumber) {fClover_Nbr.push_back(CloverNumber);  }//!
		//inline void SetCoreNumber(const UShort_t  &CoreNumber)	  {fCore_Nbr.push_back(CoreNumber);      }	//!
		//inline void SetCoreAddress(const UInt_t  &CoreAddress)	  {fCore_Address.push_back(CoreAddress);      }	//!
      //inline void SetCoreEnergy(const Double_t  &CoreEnergy)	  {fCore_Eng.push_back(CoreEnergy);      }	//!
		//inline void SetCoreCharge(const Int_t &CoreCharge)	      {fCore_Chg.push_back(CoreCharge);      }	//!
		//inline void SetCoreCFD(const Double_t &CoreTimeCFD)	      {fCore_TimeCFD.push_back(CoreTimeCFD); }	//!	
		//inline void SetCoreLED(const Double_t &CoreTimeLED)	      {fCore_TimeLED.push_back(CoreTimeLED); }	//!	
		//inline void SetCoreTime(const Double_t    &CoreTime)	    {fCore_Time.push_back(CoreTime);       }	//!
		//inline void SetIsHighGain(const Bool_t &IsHighGain)    {fCore_IsHighGain.push_back(IsHighGain); } //!

		//inline void SetCoreMidasId(const Int_t &mid)	      {fCore_MidasId.push_back(mid);      }	//!
		//inline void SetCoreNbrHits(const Int_t &nbr)	      {fCore_NbrHits.push_back(nbr);      }	//!

		//inline void SetCoreWave(const std::vector<UShort_t> &CoreWave)	{fCore_Wave.push_back(CoreWave);} //!
	
		inline void SetCore(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
				if(!frag || !channel || !mnemonic) return;

            //printf(DBLUE "+++++++++++++++++++++++++++++++++++++++++++++++" RESET_COLOR "\n");
            //printf(DBLUE "+++++++++++++++++++++++++++++++++++++++++++++++" RESET_COLOR "\n");
            //printf(DBLUE "+++++++++++++++++++++++++++++++++++++++++++++++" RESET_COLOR "\n");
            //frag->Print();
            //channel->Print();
            //PrintMNEMONIC(mnemonic);

            TGriffinCoreData core;

            core.fCore_MidasId = frag->MidasId;
            core.fCore_NbrHits = frag->Charge.size();

            //SetCoreMidasId(frag->MidasId);
            //SetCoreNbrHits(frag->Charge.size());

   			if(mnemonic->outputsensor.compare(0,1,"B")==0) { return; }  //make this smarter.

            core.fCore_IsHighGain = false;
            //SetIsHighGain(false);

//printf("IsHigGain = %i\n", (int)fCore_IsHighGain.back());
//printf(MAGENTA "-----------------------------------------------------------------------------" RESET_COLOR "\n");
//printf(MAGENTA "-----------------------------------------------------------------------------" RESET_COLOR "\n");
//printf(MAGENTA "-----------------------------------------------------------------------------" RESET_COLOR "\n");

            core.fCore_Address = frag->ChannelAddress;
            //SetCoreAddress(frag->ChannelAddress);

				UShort_t CoreNbr=5;
				if(mnemonic->arraysubposition.compare(0,1,"B")==0)
					CoreNbr=0;
				else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
					CoreNbr=1;
				else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
					CoreNbr=2;
				else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
					CoreNbr=3;

            for(int x=0;x<frag->Charge.size();x++) {
         		SetCloverNumber(mnemonic->arrayposition);
	   			SetCoreNumber(CoreNbr);
               SetCoreEnergy(channel->CalibrateENG(frag->Charge.at(x)));		
               SetCoreCharge(frag->Charge.at(x));
				   SetCoreCFD(frag->Cfd.at(x));		
   				//SetCoreLED(frag->Led.at(x));		
	   			//SetCoreTime(frag->TimeToTrig);		
	   			SetCoreTime(frag->GetTimeStamp());		
            }

      }; //! 

      inline Int_t    GetCoreMidasId(const unsigned int &i)    {return fCore_MidasId.at(i); }
      inline Int_t    GetCoreNbrHits(const unsigned int &i)    {return fCore_NbrHits.at(i); }

		inline UShort_t GetCloverNumber(const unsigned int &i)	{return fClover_Nbr.at(i);}	//!
		inline UShort_t GetCoreNumber(const unsigned int &i)	   {return fCore_Nbr.at(i);}	//!
		inline UInt_t   GetCoreAddress(const unsigned int &i)	   {return fCore_Address.at(i);}	//!

      inline Double_t GetCoreEnergy(const unsigned int &i)	   {return fCore_Eng.at(i);}	//!
		inline Int_t    GetCoreCharge(const unsigned int &i)	   {return fCore_Chg.at(i);}	//!
		inline Double_t GetCoreCFD(const unsigned int &i)	      {return fCore_TimeCFD.at(i);}	//!
		inline Double_t GetCoreLED(const unsigned int &i)	      {return fCore_TimeLED.at(i);}	//!	
		inline Double_t GetCoreTime(const unsigned int &i)	      {return fCore_Time.at(i);}	//!

      inline Bool_t GetIsHighGain(const unsigned int &i)       {return fCore_IsHighGain.at(i);} //!

		inline std::vector<UShort_t> GetCoreWave(const unsigned int &i)	{return fCore_Wave.at(i);}	//!

		inline unsigned int GetMultiplicity()		{return fCore_Nbr.size();}	//!
		
		ClassDef(TGriffinData,0) //! // TGriffinData structure
};


#endif





