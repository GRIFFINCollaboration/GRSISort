#ifndef __CSMDATA__
#define __CSMDATA__
/*****************************************************************************
 *
 *  The current layout of the 'data;' is inspired by and should be compatable
 *  with the NPTOOLS analysis kit maintained by our friends at the univeristy 
 *  of Surrey.   These class however are unique to the triumf sort code.
 *
 *  pcb
 *                                                                           
 *****************************************************************************/

// STL
#include <cstdlib>
#include <vector>
//#include <map>
using namespace std ;

// ROOT
#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorData.h"

class TCSMData : public TGRSIDetectorData  {
private:
  // CSM
  // Energy
  vector<UShort_t>   fCSM_StripHorizontal_DetectorNbr;	//!
  vector<Char_t>     fCSM_StripHorizontal_DetectorPos;	//!
  vector<Short_t>    fCSM_StripHorizontal_StripNbr;		//!
  vector<TFragment>  fCSM_StripHorizontal_Fragment;      //!
  //vector<Double_t>   fCSM_StripHorizontal_Energy;			//!
  //vector<Int_t>	   fCSM_StripHorizontal_Charge;			//!
  //vector<Int_t>      fCSM_StripHorizontal_TimeCFD;			//!
  //vector<Int_t>      fCSM_StripHorizontal_TimeLED;			//!
  //vector<Double_t>   fCSM_StripHorizontal_Time;			//!
  //vector<vector<Short_t> > fCSM_StripHorizontal_Wave;			//!  


  vector<UShort_t>   fCSM_StripVertical_DetectorNbr;		//!
  vector<Char_t>     fCSM_StripVertical_DetectorPos;		//!
  vector<Short_t>    fCSM_StripVertical_StripNbr;			//!
  vector<TFragment>  fCSM_StripVertical_Fragment;      //!
  //vector<Double_t>   fCSM_StripVertical_Energy;			   //!
  //vector<Int_t>	   fCSM_StripVertical_Charge;			   //!
  //vector<Int_t>      fCSM_StripVertical_TimeCFD;			//!
  //vector<Int_t>      fCSM_StripVertical_TimeLED;			//!
  //vector<Double_t>   fCSM_StripVertical_Time;				//!
  //vector<vector<Short_t> > fCSM_StripVertical_Wave;			//!	

  static bool fIsSet; //!

public:
  TCSMData();															//!
  virtual ~TCSMData(); //!
  
  void Clear(Option_t *opt = ""); //!
  //void Clear(const Option_t*) {};
  void Print(Option_t *opt = ""); //!
  void Print(int addr,bool horizontal); //!
  
  
  static void Set(bool flag=true) { fIsSet=flag; } //!
  static bool IsSet() { return fIsSet; }           //!

  /////////////////////           SETTERS           ////////////////////////
  inline void SetHorizontal_DetectorNbr(const UShort_t &DetNbr){fCSM_StripHorizontal_DetectorNbr.push_back(DetNbr);}	//!
  inline void SetHorizontal_DetectorPos(const UShort_t &DetPos){fCSM_StripHorizontal_DetectorPos.push_back(DetPos);}	//!
  inline void SetHorizontal_StripNbr(const Short_t &StripNbr)  {fCSM_StripHorizontal_StripNbr.push_back(StripNbr);}		//!
  inline void SetHorizontal_Fragment(const TFragment &frag)    {fCSM_StripHorizontal_Fragment.push_back(frag);}          //! 
  //inline void SetHorizontal_Energy(const Double_t &Energy)     {fCSM_StripHorizontal_Energy.push_back(Energy);}				//!
  //inline void SetHorizontal_Charge(const Int_t &Charge)       {fCSM_StripHorizontal_Charge.push_back(Charge);}				//!
  //inline void SetHorizontal_TimeCFD(const Int_t &TimeCFD)   {fCSM_StripHorizontal_TimeCFD.push_back(TimeCFD);}			//!
  //inline void SetHorizontal_TimeLED(const Int_t &TimeLED)   {fCSM_StripHorizontal_TimeLED.push_back(TimeLED);}			//!
  //inline void SetHorizontal_Time(const Double_t &Time)         {fCSM_StripHorizontal_Time.push_back(Time);}						//!
  //inline void SetHorizontal_Wave(const vector<Short_t> &Wave)    {fCSM_StripHorizontal_Wave.push_back(Wave);}						//!


  inline void SetVertical_DetectorNbr(const UShort_t &DetNbr){fCSM_StripVertical_DetectorNbr.push_back(DetNbr);}		//!
  inline void SetVertical_DetectorPos(const UShort_t &DetPos){fCSM_StripVertical_DetectorPos.push_back(DetPos);}		//!
  inline void SetVertical_StripNbr(const Short_t &StripNbr)  {fCSM_StripVertical_StripNbr.push_back(StripNbr);}		//!
  inline void SetVertical_Fragment(const TFragment &frag)    {fCSM_StripVertical_Fragment.push_back(frag);}          //! 
  //inline void SetVertical_Energy(const Double_t &Energy)     {fCSM_StripVertical_Energy.push_back(Energy);}				//!
  //inline void SetVertical_Charge(const Int_t &Charge)       {fCSM_StripVertical_Charge.push_back(Charge);}					//!
  //inline void SetVertical_TimeCFD(const Int_t &TimeCFD)   {fCSM_StripVertical_TimeCFD.push_back(TimeCFD);}			//!
  //inline void SetVertical_TimeLED(const Int_t &TimeLED)   {fCSM_StripVertical_TimeLED.push_back(TimeLED);}			//!
  //inline void SetVertical_Time(const Double_t &Time)         {fCSM_StripVertical_Time.push_back(Time);}						//!
  //inline void SetVertical_Wave(const vector<Short_t> &Wave)    {fCSM_StripVertical_Wave.push_back(Wave);}							//!

  inline void SetHorizontal(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
    SetHorizontal_DetectorNbr(mnemonic->arrayposition);
	 SetHorizontal_DetectorPos(mnemonic->arraysubposition.c_str()[0]);
	 SetHorizontal_StripNbr(mnemonic->segment);
    SetHorizontal_Fragment(*frag);
	 //SetHorizontal_Energy(channel->CalibrateENG(frag->Charge.at(0)));
	 //SetHorizontal_TimeCFD(frag->Cfd.at(0));
	 //SetHorizontal_TimeLED(frag->Led.at(0));
	 //SetHorizontal_Time(frag->Zc.at(0));
 	 //SetHorizontal_Charge(frag->Charge.at(0));
  };	//! 
	
  inline void SetVertical(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic)	{
		SetVertical_DetectorNbr(mnemonic->arrayposition);
		SetVertical_DetectorPos(mnemonic->arraysubposition.c_str()[0]);
		SetVertical_StripNbr(mnemonic->segment);
      SetVertical_Fragment(*frag);
		//SetVertical_Energy(channel->CalibrateENG(frag->Charge.at(0)));
		//SetVertical_TimeCFD(frag->Cfd.at(0));
		//SetVertical_TimeLED(frag->Led.at(0));
		//SetVertical_Time(frag->Zc.at(0));
 		//SetVertical_Charge(frag->Charge.at(0));
	};	//!
	
  /////////////////////           GETTERS           ////////////////////////
  inline UShort_t GetHorizontal_DetectorNbr(const unsigned int &i) const {return fCSM_StripHorizontal_DetectorNbr[i];}	//!
  inline UShort_t GetHorizontal_DetectorPos(const unsigned int &i) const {return fCSM_StripHorizontal_DetectorPos[i];}	//!
  inline UShort_t GetHorizontal_StripNbr(const int &i)    const {return fCSM_StripHorizontal_StripNbr[i];}		//!
  inline Double_t GetHorizontal_Energy(const unsigned int &i)      const {return fCSM_StripHorizontal_Fragment.at(i).GetEnergy();} //!
  inline Int_t 	GetHorizontal_Charge(const unsigned int &i)      const {return fCSM_StripHorizontal_Fragment.at(i).GetCharge();} //!
  inline Int_t    GetHorizontal_TimeCFD(const unsigned int &i)     const {return fCSM_StripHorizontal_Fragment.at(i).GetCfd();} //!
  inline Int_t    GetHorizontal_TimeLED(const unsigned int &i)     const {return fCSM_StripHorizontal_Fragment.at(i).GetLed();} //!
  inline Double_t GetHorizontal_Time(const unsigned int &i)        const {return fCSM_StripHorizontal_Fragment.at(i).GetTimeStamp();} //!
  inline vector<Short_t> GetHorizontal_Wave(const unsigned int &i) const {return fCSM_StripHorizontal_Fragment.at(i).wavebuffer;} //!

  inline TFragment GetHorizontal_Fragment(const unsigned int &i)      const {return fCSM_StripHorizontal_Fragment.at(i);} //!

  inline UShort_t GetVertical_DetectorNbr(const unsigned int &i) const {return fCSM_StripVertical_DetectorNbr[i];}	//!
  inline UShort_t GetVertical_DetectorPos(const unsigned int &i) const {return fCSM_StripVertical_DetectorPos[i];}	//!
  inline UShort_t GetVertical_StripNbr(const int &i)    const {return fCSM_StripVertical_StripNbr[i];}		//!
  inline Double_t GetVertical_Energy(const unsigned int &i)      const {return fCSM_StripHorizontal_Fragment.at(i).GetEnergy();} //!
  inline Int_t 	GetVertical_Charge(const unsigned int &i)      const {return fCSM_StripHorizontal_Fragment.at(i).GetCharge();} //!
  inline Int_t    GetVertical_TimeCFD(const unsigned int &i)     const {return fCSM_StripHorizontal_Fragment.at(i).GetCfd();} //!
  inline Int_t    GetVertical_TimeLED(const unsigned int &i)     const {return fCSM_StripHorizontal_Fragment.at(i).GetLed();} //!
  inline Double_t GetVertical_Time(const unsigned int &i)        const {return fCSM_StripHorizontal_Fragment.at(i).GetTimeStamp();} //!
  inline vector<Short_t> GetVertical_Wave(const unsigned int &i) const {return fCSM_StripHorizontal_Fragment.at(i).wavebuffer;} //!
  
  inline TFragment GetiVertical_Fragment(const unsigned int &i)      const {return fCSM_StripVertical_Fragment.at(i);} //!

  inline unsigned int GetMultiplicityHorizontal() const {return fCSM_StripHorizontal_DetectorNbr.size();}				//!
  inline unsigned int GetMultiplicityVertical()  const {return fCSM_StripVertical_DetectorNbr.size();}				//!
 
  //ClassDef(TCSMData,0)  //!  // CSMData structure 
};

#endif
