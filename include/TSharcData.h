#ifndef __SharcDATA__
#define __SharcDATA__
/*****************************************************************************
 *
 *  The current layout of the 'data;' is inspired by and should be compatable
 *  with the NPTOOLS analysis kit maintained by our friends at the univeristy 
 *  of Surrey.   These class however are unique to the triumf sort code.
 *
 *  pcb
 *                                                                           
 *****************************************************************************/

#include "Globals.h"

#include <cstdlib>
#include <vector>

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TSharcData : public TGRSIDetectorData  {
private:
  std::vector<UShort_t>   fSharc_Front_DetectorNbr; //!
  std::vector<UShort_t>   fSharc_Front_StripNbr;    //!
  std::vector<TFragment>  fSharc_FrontFragment      //!

  std::vector<UShort_t>   fSharc_Back_DetectorNbr;  //!
  std::vector<UShort_t>   fSharc_Back_StripNbr;     //!
  std::vector<TFragment>  fSharc_BackFragment       //!

  std::vector<UShort_t>   fSharc_Pad_DetectorNbr;   //!
  std::vector<TFragment>  fSharc_PadFragment        //!

  static bool fIsSet; //!

public:
  TSharcData();          //!
  virtual ~TSharcData(); //!
  
  static void Set(bool flag=true) { fIsSet=flag; } //!
  static bool IsSet() { return fIsSet; }           //!

  void Clear(Option_t *opt=""); //!
  void Print(Option_t *opt="") const; //!
  
  /////////////////////           SETTERS           ////////////////////////
  inline void SetFront_DetectorNbr(const int &det)     { fSharc_Front_DetectorNbr.push_back(det); }
  inline void SetFront_StripNbr(const int &strip)      { fSharc_Front_StripNbr.push_back(strip);  }
  inline void SetFront_Fragment(const TFragment &frag) { fSharc_FrontFragment.push_back(frag);    }
  
  inline void SetBack_DetectorNbr(const int &det)     { fSharc_Back_DetectorNbr.push_back(det); }
  inline void SetBack_StripNbr(const int &strip)      { fSharc_Back_StripNbr.push_back(strip);  }
  inline void SetBack_Fragment(const TFragment &frag) { fSharc_BackFragment.push_back(frag);    }

  inline void SetPad_DetectorNbr(const int &det)     { fSharc_Pad_DetectorNbr.push_back(det); }
  inline void SetPad_Fragment(const TFragment &frag) { fSharc_PadFragment.push_back(frag);    }

  inline void SetFront(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
    if(!frag || !channel || !mnemonic) return;
    SetFront_DetectorNbr(mnemonic->arrayposition);
    SetFront_StripNbr(mnemonic->segment);
    SetFront_Fragment(*frag);
  }
  
  inline void SetBack(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
    if(!frag || !channel || !mnemonic) return;
    SetBack_DetectorNbr(mnemonic->arrayposition);
    SetBack_StripNbr(mnemonic->segment);
    SetBack_Fragment(*frag);
  }
  
  inline void SetPadk(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
    if(!frag || !channel || !mnemonic) return;
    SetPad_DetectorNbr(mnemonic->arrayposition);
    SetPad_StripNbr(mnemonic->segment);
    SetPad_Fragment(*frag);
  }

  ///////////i//////////           GETTERS           ////////////////////////
  const inline UShort_t GetFront_DetectorNbr(const unsigned int &i)    const { return fSharc_Front_DetectorNbr[i];}           //!
  const inline UShort_t GetFront_StripNbr(const unsigned int &i)       const { return fSharc_Front_StripNbr[i];}              //!
  const inline UInt_t   GetFront_Address(const unsigned int &i)        const { return fSharc_FrontFragment->ChannelAddress; } //!
  const inline Double_t GetFront_Energy(const unsigned int &i)         const { return fSharc_FrontFragment->GetEnergy();}     //!
  const inline Double_t GetFront_Charge(const unsigned int &i)         const { return fSharc_FrontFragment->GetCharge();}     //!
  const inline Double_t GetFront_Cfd(const unsigned int &i)            const { return fSharc_FrontFragment->GetCfd();}        //!
  const inline Double_t GetFront_Led(const unsigned int &i)            const { return fSharc_FrontFragment->GetLed();}        //!
  const inline Double_t GetFront_TimeStamp(const unsigned int &i)      const { return fSharc_FrontFragment->GetTimeStamp();}  //!
  inline std::vector<Short_t> GetFront_Wave(const unsigned int &i)     const { return fSharc_FrontFragment->wavebuffer;}      //!

  const inline UShort_t GetBack_DetectorNbr(const unsigned int &i)    const { return fSharc_Back_DetectorNbr[i];}             //!
  const inline UShort_t GetBack_StripNbr(const unsigned int &i)       const { return fSharc_Back_StripNbr[i];}                //!
  const inline UInt_t   GetBack_Address(const unsigned int &i)        const { return fSharc_BackFragment->ChannelAddress; }   //!
  const inline Double_t GetBack_Energy(const unsigned int &i)         const { return fSharc_BackFragment->GetEnergy();}       //!
  const inline Double_t GetBack_Charge(const unsigned int &i)         const { return fSharc_BackFragment->GetCharge();}       //!
  const inline Double_t GetBack_Cfd(const unsigned int &i)            const { return fSharc_BackFragment->GetCfd();}          //!
  const inline Double_t GetBack_Led(const unsigned int &i)            const { return fSharc_BackFragment->GetLed();}          //!
  const inline Double_t GetBack_TimeStamp(const unsigned int &i)      const { return fSharc_BackFragment->GetTimeStamp();}    //!
  inline std::vector<Short_t> GetBack_Wave(const unsigned int &i)     const { return fSharc_BackFragment->wavebuffer;}        //!

  const inline UShort_t GetPad_DetectorNbr(const unsigned int &i)    const { return fSharc_Pad_DetectorNbr[i];}             //!
  const inline UInt_t   GetPad_Address(const unsigned int &i)        const { return fSharc_PadFragment->ChannelAddress; }   //!
  const inline Double_t GetPad_Energy(const unsigned int &i)         const { return fSharc_PadFragment->GetEnergy();}       //!
  const inline Double_t GetPad_Charge(const unsigned int &i)         const { return fSharc_PadFragment->GetCharge();}       //!
  const inline Double_t GetPad_Cfd(const unsigned int &i)            const { return fSharc_PadFragment->GetCfd();}          //!
  const inline Double_t GetPad_Led(const unsigned int &i)            const { return fSharc_PadFragment->GetLed();}          //!
  const inline Double_t GetPad_TimeStamp(const unsigned int &i)      const { return fSharc_PadFragment->GetTimeStamp();}    //!
  inline std::vector<Short_t> GetPad_Wave(const unsigned int &i)     const { return fSharc_PadFragment->wavebuffer;}        //!

  inline unsigned int GetSizeFront() const {return fSharc_Front_DetectorNbr.size();} //!
  inline unsigned int GetSizeBack()  const {return fSharc_Back_DetectorNbr.size();}  //!
  inline unsigned int GetSizePAD()   const {return fSharc_Pad_DetectorNbr.size();    //!
 
  //ClassDef(TSharcData,0)  //!  // SharcData structure 
};

#endif
