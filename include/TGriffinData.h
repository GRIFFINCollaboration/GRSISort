#ifndef TGRIFFINDATA_H
#define TGRIFFINDATA_H

#if !defined (__CINT__) && !defined (__CLING__)

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <iostream>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"
//#include "TGriffinCoreData.h"

class TGriffinData : public TGRSIDetectorData {

  private:
    //std::vector<TGriffinCoreData> fCore; //!
    std::vector<UShort_t> fClover_Nbr;		//!
    std::vector<UShort_t> fCore_Nbr;		//!
    std::vector<Int_t>    fCore_Address;		//!
    std::vector<Bool_t>   fCore_IsHighGain; //!
    std::vector<Double_t> fCore_Eng;		//!
    std::vector<Float_t>  fCore_Chg;		//!
    std::vector<Int_t>    fCore_TimeCFD;	//!
    std::vector<Long_t>   fCore_Time;		//!
    std::vector<std::vector<Short_t> >fCore_Wave;	//!

    std::vector<Int_t>        fCore_NbrHits; //!
    std::vector<Int_t>        fCore_PUHit; //!
    std::vector<Int_t>        fCore_MidasId; //!

    static bool fIsSet; //!

  public:
    TGriffinData();					//!
    virtual ~TGriffinData();			//!

    static void Set(bool flag=true) { fIsSet=flag; } //!
    static bool IsSet() { return fIsSet; }            //!

    virtual void Clear(Option_t *opt = "");		//!
    virtual void Print(Option_t *opt = "") const;		//!

    inline void SetCloverNumber(const UShort_t &CloverNumber) {fClover_Nbr.push_back(CloverNumber);  }//!
    inline void SetCoreNumber(const UShort_t  &CoreNumber)    {fCore_Nbr.push_back(CoreNumber);      }	//!
    inline void SetCoreAddress(const Int_t  &CoreAddress)     {fCore_Address.push_back(CoreAddress); }	//!
    inline void SetCoreEnergy(const Double_t &CoreEnergy)     {fCore_Eng.push_back(CoreEnergy);      }	//!
    inline void SetCoreCharge(const Float_t &CoreCharge)     {fCore_Chg.push_back(CoreCharge);      }	//!
    inline void SetCoreCFD(const Int_t &CoreTimeCFD)	        {fCore_TimeCFD.push_back(CoreTimeCFD); }	//!	
    inline void SetCoreTime(const Long_t    &CoreTime)        {fCore_Time.push_back(CoreTime);       }	//!
    inline void SetIsHighGain(const Bool_t &IsHighGain)       {fCore_IsHighGain.push_back(IsHighGain); } //!
    
    inline void SetCoreMidasId(const Int_t &mid)	      {fCore_MidasId.push_back(mid);      }	//!
    inline void SetCoreNbrHits(const Int_t &nbr)	      {fCore_NbrHits.push_back(nbr);      }	//!
    inline void SetCorePUHit(const Int_t &puhit)         {fCore_PUHit.push_back(puhit);      }  //!
    
    inline void SetCoreWave(const std::vector<Short_t> &CoreWave)	{fCore_Wave.push_back(CoreWave);} //!

    inline void SetCore(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
	   if(!frag || !channel || !mnemonic) return;

     	   if(mnemonic->outputsensor.compare(0,1,"B")==0) { return; }  //make this smarter.
   
         UShort_t CoreNbr=5;
            if(mnemonic->arraysubposition.compare(0,1,"B")==0)
       	      CoreNbr=0;
            else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
       	      CoreNbr=1;
            else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
       	      CoreNbr=2;
            else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
       	      CoreNbr=3;
   
            for(size_t x=0;x<frag->Charge.size();x++) {
               SetCoreMidasId(frag->MidasId);
               SetCoreNbrHits(frag->Charge.size());
               SetCorePUHit(x);
               SetIsHighGain(false);
               SetCoreAddress(frag->ChannelAddress);
           	   SetCloverNumber(mnemonic->arrayposition);
            	SetCoreNumber(CoreNbr);
               //if(frag->KValue.size()>x)
               //  SetCoreEnergy(channel->CalibrateENG(frag->Charge.at(x),frag->KValue.at(x)));		
               //else
               //  SetCoreEnergy(channel->CalibrateENG(frag->Charge.at(x)));		
               SetCoreEnergy(frag->GetEnergy(x));
               SetCoreCharge(frag->GetCharge(x));
     	         SetCoreCFD(frag->Cfd.at(x));		
             //SetCoreTime(frag->Zc.at(0));		
               SetCoreTime(frag->GetTimeStamp());		
            }
   
      } //! 

      inline Int_t    GetCoreMidasId(const unsigned int &i) const    {return fCore_MidasId.at(i); }
      inline Int_t    GetCoreNbrHits(const unsigned int &i) const    {return fCore_NbrHits.at(i); }
      inline Int_t    GetCorePUHit(const unsigned int &i) const          {return fCore_PUHit.at(i);}//!

      inline UShort_t GetCloverNumber(const unsigned int &i) const   {return fClover_Nbr.at(i);}	//!
      inline UShort_t GetCoreNumber(const unsigned int &i) const     {return fCore_Nbr.at(i);}	//!
      inline Int_t    GetCoreAddress(const unsigned int &i) const    {return fCore_Address.at(i);}	//!

      inline Double_t GetCoreEnergy(const unsigned int &i) const     {return fCore_Eng.at(i);}	//!
      inline Float_t  GetCoreCharge(const unsigned int &i) const       {return fCore_Chg.at(i);}	//!
      inline Int_t  GetCoreCFD(const unsigned int &i) const          {return fCore_TimeCFD.at(i);}	//!
      inline Long_t GetCoreTime(const unsigned int &i) const       {return fCore_Time.at(i);}	//!

      inline Bool_t GetIsHighGain(const unsigned int &i) const       {return fCore_IsHighGain.at(i);} //!

      inline std::vector<Short_t> GetCoreWave(const unsigned int &i) const {return fCore_Wave.at(i);}	//!

      inline unsigned int GetMultiplicity() const	           {return fCore_Nbr.size();}	//!

      //ClassDef(TGriffinData,0) //! // TGriffinData structure
};


#endif //ifndef __CINT__

#endif //ifndef TGRIFFINDATA_H



