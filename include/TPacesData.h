#ifndef TPACESDATA_H
#define TPACESDATA_H

#include <cstdlib>
#include <cstdio>
#include <vector>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TPacesData : public TGRSIDetectorData {

  private:
    std::vector<UShort_t> fCore_Nbr;		//!
    std::vector<UInt_t>   fCore_Address;		//!
    std::vector<Bool_t>   fCore_IsHighGain; //!
    std::vector<Double_t> fCore_Eng;		//!
    std::vector<Int_t>    fCore_Chg;		//!
    std::vector<Long_t>   fCore_TimeCFD;	//!
    std::vector<Long_t>   fCore_Time;		//!
    std::vector<std::vector<Short_t> >fCore_Wave;	//!

    std::vector<Int_t>        fCore_NbrHits; //!
    std::vector<Int_t>        fCore_MidasId; //!

   std::vector<Int_t>         fPPG; //!

    static bool fIsSet; //!

  public:
    TPacesData();					//!
    virtual ~TPacesData();			//!

    static void Set(bool flag=true) { fIsSet=flag; } //!
    static bool IsSet() { return fIsSet; }            //!

    virtual void Clear(Option_t *opt = "");		//!
    virtual void Print(Option_t *opt = "") const;		//!

    inline void SetCoreNumber(const UShort_t  &CoreNumber)    {fCore_Nbr.push_back(CoreNumber);      }	//!
    inline void SetCoreAddress(const UInt_t  &CoreAddress)    {fCore_Address.push_back(CoreAddress); }	//!
    inline void SetCoreEnergy(const Double_t &CoreEnergy)     {fCore_Eng.push_back(CoreEnergy);      }	//!
    inline void SetCoreCharge(const Int_t &CoreCharge)	     {fCore_Chg.push_back(CoreCharge);      }	//!
    inline void SetCoreCFD(const Long_t &CoreTimeCFD)	        {fCore_TimeCFD.push_back(CoreTimeCFD); }	//!	
    inline void SetCoreTime(const Long_t    &CoreTime)        {fCore_Time.push_back(CoreTime);       }	//!
    inline void SetIsHighGain(const Bool_t &IsHighGain)       {fCore_IsHighGain.push_back(IsHighGain); } //!
    
    inline void SetCoreMidasId(const Int_t &mid)	      {fCore_MidasId.push_back(mid);      }	//!
    inline void SetCoreNbrHits(const Int_t &nbr)	      {fCore_NbrHits.push_back(nbr);      }	//!
    
    inline void SetCoreWave(const std::vector<Short_t> &CoreWave)	{fCore_Wave.push_back(CoreWave);} //!

    inline void SetPPG(const Int_t &ppg)                 {fPPG.push_back(ppg);               }  //!
    
    inline void SetCore(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
	      if(!frag || !channel || !mnemonic) return;

          SetCoreMidasId(frag->MidasId);
          SetCoreNbrHits(frag->Charge.size());

   	    if(mnemonic->outputsensor.compare(0,1,"B")==0) { return; }  //make this smarter.
	       SetCoreNumber(mnemonic->arrayposition);

          //core.fCore_IsHighGain = false;
          SetIsHighGain(false);

         SetPPG(frag->PPG);
          //core.fCore_Address = frag->ChannelAddress;
          SetCoreAddress(frag->ChannelAddress);

          for(int x=0;x<frag->Charge.size();x++) {
            SetCoreCharge(frag->Charge.at(x));
		      SetCoreCFD(frag->Cfd.at(x));		
	        //SetCoreTime(frag->TimeToTrig);		
	         SetCoreTime(frag->GetTimeStamp());		
          }

      } //! 

      inline Int_t    GetCoreMidasId(const unsigned int &i) const    {return fCore_MidasId.at(i); }
      inline Int_t    GetCoreNbrHits(const unsigned int &i) const    {return fCore_NbrHits.at(i); }

      inline UShort_t GetCoreNumber(const unsigned int &i) const     {return fCore_Nbr.at(i);}	//!
      inline UInt_t   GetCoreAddress(const unsigned int &i) const    {return fCore_Address.at(i);}	//!

      inline Double_t GetCoreEnergy(const unsigned int &i) const     {return fCore_Eng.at(i);}	//!
      inline Int_t  GetCoreCharge(const unsigned int &i) const       {return fCore_Chg.at(i);}	//!
      inline Long_t GetCoreCFD(const unsigned int &i) const          {return fCore_TimeCFD.at(i);}	//!
      inline Long_t GetCoreTime(const unsigned int &i) const       {return fCore_Time.at(i);}	//!

      inline Bool_t GetIsHighGain(const unsigned int &i) const       {return fCore_IsHighGain.at(i);} //!

      inline Int_t GetPPG(const unsigned int &i) const               {return fPPG.at(i);} //!

      inline std::vector<Short_t> GetCoreWave(const unsigned int &i) const {return fCore_Wave.at(i);}	//!

      inline unsigned int GetMultiplicity() const	           {return fCore_Address.size();}	//!
		
      //ClassDef(TPacesData,0) //! // TPacesData structure
};


#endif





