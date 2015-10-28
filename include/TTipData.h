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
    std::vector <TFragment> fTip_Frag;  //!
    std::vector <std::string> fTip_Name; //!
	std::vector <UInt_t> fTip_Address;	

    static bool fIsSet;      //!
   public:
     TTipData();          //!
     virtual ~TTipData();      //!

     static void Set(bool flag=true) { fIsSet=flag; } //!
     static bool IsSet() { return fIsSet; }           //!

     void Clear(Option_t *opt = "");          //!
     void Print(Option_t *opt = "") const;    //!

  private:

    inline void SetTipFragment(const TFragment &Frag)       {fTip_Frag.push_back(Frag);        }  //!  
    inline void SetTipName(const char *Name)                {fTip_Name.push_back(std::string(Name));        }  //!  
	inline void SetTipAddress(const UInt_t Address)			{fTip_Address.push_back(Address);	}

  public:
    void SetDet(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)  {
      if(!frag || !channel || !mnemonic) return;
      SetTipFragment(*frag);
      SetTipName(channel->GetChannelName());
	  SetTipAddress(frag->ChannelAddress);
    }; //! 

    inline TFragment   GetTipFragment(const unsigned int &i)   {return fTip_Frag.at(i);}  //!
    inline std::string GetTipName(const unsigned int &i)       {return fTip_Name.at(i);}  //!
	inline UInt_t 	   GetTipAddress(const unsigned int &i)	   {return fTip_Address.at(i);} //!

    inline unsigned int GetMultiplicity()    {return fTip_Frag.size();}                   //!
    
};

#endif

#endif
