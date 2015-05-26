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
  // Sharc
  // Energy
  std::vector<UShort_t>   fSharc_StripFront_DetectorNbr;	//!
  std::vector<Int_t>      fSharc_StripFront_ChannelAddress;		//!
  std::vector<UShort_t>   fSharc_StripFront_StripNbr;		//!
  std::vector<Double_t>   fSharc_StripFront_Energy;			//!
  std::vector<Double_t>   fSharc_StripFront_EngChi2;			//!
  std::vector<Int_t>	    fSharc_StripFront_Charge;			//!
  std::vector<Double_t>   fSharc_StripFront_TimeCFD;			//!
  std::vector<Double_t>   fSharc_StripFront_TimeLED;			//!
  std::vector<Double_t>   fSharc_StripFront_Time;			//!
  std::vector<std::vector<Int_t> > fSharc_StripFront_Wave;			//!  


  std::vector<UShort_t>   fSharc_StripBack_DetectorNbr;		//!
  std::vector<Int_t>      fSharc_StripBack_ChannelAddress;
  std::vector<UShort_t>   fSharc_StripBack_StripNbr;			//!
  std::vector<Double_t>   fSharc_StripBack_Energy;			   //!
  std::vector<Double_t>   fSharc_StripBack_EngChi2;			//!
  std::vector<Int_t>	    fSharc_StripBack_Charge;			//!
  std::vector<Double_t>   fSharc_StripBack_TimeCFD;			//!
  std::vector<Double_t>   fSharc_StripBack_TimeLED;			//!
  std::vector<Double_t>   fSharc_StripBack_Time;				//!
  std::vector<std::vector<Int_t> > fSharc_StripBack_Wave;	//!	

  std::vector<UShort_t>   fSharc_PAD_DetectorNbr;			//!
  std::vector<Int_t>      fSharc_PAD_ChannelAddress;     //!
  std::vector<Double_t>   fSharc_PAD_Energy;					//!
  std::vector<Double_t>   fSharc_PAD_EngChi2;			   //!
  std::vector<Int_t>	     fSharc_PAD_Charge;					//!
  std::vector<Double_t>   fSharc_PAD_TimeCFD;				//!
  std::vector<Double_t>   fSharc_PAD_TimeLED;				//!
  std::vector<Double_t>   fSharc_PAD_Time;					//!
  std::vector<std::vector<Int_t> > fSharc_PAD_Wave;		//!

	static bool fIsSet; //!

public:
  TSharcData();															//!
  virtual ~TSharcData(); //!
  
	static void Set(bool flag=true) { fIsSet=flag; } //!
	static bool IsSet() { return fIsSet; }           //!

  void Clear(Option_t *opt=""); //!
  //void Clear(const Option_t*) {};
  void Print(Option_t *opt=""); //!
  
  /////////////////////           SETTERS           ////////////////////////
  inline void SetFront_DetectorNbr(const UShort_t &DetNbr){fSharc_StripFront_DetectorNbr.push_back(DetNbr);}	//!
  inline void SetFront_ChannelAddress(const Int_t &ChanAdd){fSharc_StripFront_ChannelAddress.push_back(ChanAdd);}	//!
  inline void SetFront_StripNbr(const UShort_t &StripNbr){fSharc_StripFront_StripNbr.push_back(StripNbr);}		//!
  inline void SetFront_Energy(const Double_t &Energy){fSharc_StripFront_Energy.push_back(Energy);}				//!
  inline void SetFront_EngChi2(const Double_t &Chi2){fSharc_StripFront_EngChi2.push_back(Chi2);}				//!
  inline void SetFront_Charge(const  Int_t &Charge){fSharc_StripFront_Charge.push_back(Charge);}				//!
  inline void SetFront_TimeCFD(const Double_t &TimeCFD){fSharc_StripFront_TimeCFD.push_back(TimeCFD);}			//!
  inline void SetFront_TimeLED(const Double_t &TimeLED){fSharc_StripFront_TimeLED.push_back(TimeLED);}			//!
  inline void SetFront_Time(const Double_t &Time){fSharc_StripFront_Time.push_back(Time);}						//!

  inline void SetFront_Wave(const std::vector<Int_t> &Wave){fSharc_StripFront_Wave.push_back(Wave);}						//!


  inline void SetBack_DetectorNbr(const UShort_t &DetNbr){fSharc_StripBack_DetectorNbr.push_back(DetNbr);}		//!
  inline void SetBack_ChannelAddress(const Int_t &ChanAdd){fSharc_StripBack_ChannelAddress.push_back(ChanAdd);}	//!
  inline void SetBack_StripNbr(const UShort_t &StripNbr){fSharc_StripBack_StripNbr.push_back(StripNbr);}		//!
  inline void SetBack_Energy(const Double_t &Energy){fSharc_StripBack_Energy.push_back(Energy);}				//!
  inline void SetBack_EngChi2(const Double_t &Chi2){fSharc_StripBack_EngChi2.push_back(Chi2);}				//!
  inline void SetBack_Charge(const  Int_t &Charge){fSharc_StripBack_Charge.push_back(Charge);}					//!
  inline void SetBack_TimeCFD(const Double_t &TimeCFD){fSharc_StripBack_TimeCFD.push_back(TimeCFD);}			//!
  inline void SetBack_TimeLED(const Double_t &TimeLED){fSharc_StripBack_TimeLED.push_back(TimeLED);}			//!
  inline void SetBack_Time(const Double_t &Time){fSharc_StripBack_Time.push_back(Time);}						//!

  inline void SetBack_Wave(const std::vector<Int_t> &Wave){fSharc_StripBack_Wave.push_back(Wave);}							//!


  inline void SetPAD_DetectorNbr(const UShort_t &DetNbr){fSharc_PAD_DetectorNbr.push_back(DetNbr);}				//!
  inline void SetPAD_ChannelAddress(const Int_t &ChanAdd){fSharc_PAD_ChannelAddress.push_back(ChanAdd);}  //!
  inline void SetPAD_Energy(const Double_t &Energy){fSharc_PAD_Energy.push_back(Energy);}						//!
  inline void SetPAD_EngChi2(const Double_t &Chi2){fSharc_PAD_EngChi2.push_back(Chi2);}				//!
  inline void SetPAD_Charge(const Int_t &Charge){fSharc_PAD_Charge.push_back(Charge);}							//!
  inline void SetPAD_TimeCFD(const Double_t &TimeCFD){fSharc_PAD_TimeCFD.push_back(TimeCFD);}					//!
  inline void SetPAD_TimeLED(const Double_t &TimeLED){fSharc_PAD_TimeLED.push_back(TimeLED);}					//!
  inline void SetPAD_Time(const Double_t &Time){fSharc_PAD_Time.push_back(Time);}								//!

  inline void SetPAD_Wave(const std::vector<Int_t> &Wave){fSharc_PAD_Wave.push_back(Wave);}							//!


	inline void SetFront(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
		if(!frag || !channel || !mnemonic) return;
		SetFront_DetectorNbr(mnemonic->arrayposition);
		SetFront_StripNbr(mnemonic->segment);
		SetFront_ChannelAddress(frag->ChannelAddress);
		if(channel->GetENGChi2() < 10000 && channel->GetENGChi2()!=0.00) /// Tell SJC to make this a variable, he loves calibrating sharc.
         SetFront_Energy(frag->GetEnergy(0));
			//SetFront_Energy(channel->CalibrateENG(frag->Charge.at(0)));
		else 
			SetFront_Energy(0.0);
		SetFront_TimeCFD(frag->Cfd.at(0));
		SetFront_TimeLED(frag->Led.at(0));
		SetFront_Time(frag->Zc.at(0));
 		SetFront_Charge(frag->Charge.at(0));
		SetFront_EngChi2(channel->GetENGChi2());
	}
	

	inline void SetBack(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
		if(!frag || !channel || !mnemonic) return;
		SetBack_DetectorNbr(mnemonic->arrayposition);
		SetBack_StripNbr(mnemonic->segment);
		SetBack_ChannelAddress(frag->ChannelAddress);

		if(channel->GetENGChi2() < 10000 && channel->GetENGChi2()!=0.00)
			SetBack_Energy(frag->GetEnergy(0));
			//SetBack_Energy(channel->CalibrateENG(frag->Charge.at(0)));
		else
			SetBack_Energy(0.0);

		SetBack_TimeCFD(frag->Cfd.at(0));
		SetBack_TimeLED(frag->Led.at(0));
		SetBack_Time(frag->Zc.at(0));
 		SetBack_Charge(frag->Charge.at(0));
		SetBack_EngChi2(channel->GetENGChi2());
	}

	
	inline void SetPAD(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
		if(!frag || !channel || !mnemonic) return;
		SetPAD_DetectorNbr(mnemonic->arrayposition);
		SetPAD_ChannelAddress(frag->ChannelAddress);

		if(channel->GetENGChi2() < 10000 && channel->GetENGChi2()!=0.00)
			SetPAD_Energy(frag->GetEnergy(0));
		else
			SetPAD_Energy(0.0);


		SetPAD_TimeCFD(frag->Cfd.at(0));
		SetPAD_TimeLED(frag->Led.at(0));
		SetPAD_Time(frag->Zc.at(0));
 		SetPAD_Charge(frag->Charge.at(0));
		SetPAD_EngChi2(channel->GetENGChi2());
	}






	
  ///////////i//////////           GETTERS           ////////////////////////
  inline UShort_t GetFront_DetectorNbr(const unsigned int &i) const {return fSharc_StripFront_DetectorNbr[i];}	//!
  inline Int_t    GetFront_ChannelAddress(const unsigned int &i)    const {return fSharc_StripFront_ChannelAddress[i];}		//!
  inline UShort_t GetFront_StripNbr(const unsigned int &i)    const {return fSharc_StripFront_StripNbr[i];}		//!
  inline Double_t GetFront_Energy(const unsigned int &i)      const {return fSharc_StripFront_Energy[i];}		//!
  inline Double_t GetFront_EngChi2(const unsigned int &i)      const {return fSharc_StripFront_EngChi2[i];}		//!
  inline Int_t 	  GetFront_Charge(const unsigned int &i)      const {return fSharc_StripFront_Charge[i];}		//!
  inline Double_t GetFront_TimeCFD(const unsigned int &i)     const {return fSharc_StripFront_TimeCFD[i];}		//!
  inline Double_t GetFront_TimeLED(const unsigned int &i)     const {return fSharc_StripFront_TimeLED[i];}		//!
  inline Double_t GetFront_Time(const unsigned int &i)     const {return fSharc_StripFront_Time[i];}			//!

  inline std::vector<Int_t> GetFront_Wave(const unsigned int &i)	const {return fSharc_StripFront_Wave.at(i);}		//!

  inline UShort_t GetBack_DetectorNbr(const unsigned int &i) const {return fSharc_StripBack_DetectorNbr[i];}	//!
  inline Int_t    GetBack_ChannelAddress(const unsigned int &i)    const {return fSharc_StripBack_ChannelAddress[i];}		//!
  inline UShort_t GetBack_StripNbr(const unsigned int &i)    const {return fSharc_StripBack_StripNbr[i];}		//!
  inline Double_t GetBack_Energy(const unsigned int &i)      const {return fSharc_StripBack_Energy[i];}			//!
  inline Double_t GetBack_EngChi2(const unsigned int &i)      const {return fSharc_StripBack_EngChi2[i];}		//!
  inline Int_t	  GetBack_Charge(const unsigned int &i)      const {return fSharc_StripBack_Charge[i];}		//!
  inline Double_t GetBack_TimeCFD(const unsigned int &i)     const {return fSharc_StripBack_TimeCFD[i];}		//!
  inline Double_t GetBack_TimeLED(const unsigned int &i)     const {return fSharc_StripBack_TimeLED[i];}		//!
  inline Double_t GetBack_Time(const unsigned int &i)     const {return fSharc_StripBack_Time[i];}				//!

  inline std::vector<Int_t> GetBack_Wave(const unsigned int &i)	const {return fSharc_StripBack_Wave.at(i);}			//!

  inline UShort_t GetPAD_DetectorNbr(const unsigned int &i) const {return fSharc_PAD_DetectorNbr[i];}			//!
  inline Int_t    GetPAD_ChannelAddress(const unsigned int &i) const {return fSharc_PAD_ChannelAddress[i];}				//!
  inline Double_t GetPAD_Energy(const unsigned int &i)      const {return fSharc_PAD_Energy[i];}				//!
  inline Double_t GetPAD_EngChi2(const unsigned int &i)      const {return fSharc_PAD_EngChi2[i];}		//!
  inline Int_t		GetPAD_Charge(const int &i)      		const {return fSharc_PAD_Charge[i];}				//!
  inline Double_t GetPAD_TimeCFD(const unsigned int &i)     const {return fSharc_PAD_TimeCFD[i];}				//!
  inline Double_t GetPAD_TimeLED(const unsigned int &i)     const {return fSharc_PAD_TimeLED[i];}				//!
  inline Double_t GetPAD_Time(const unsigned int &i)    	  const {return fSharc_PAD_Time[i];}				//!

  inline std::vector<Int_t> GetPAD_Wave(const unsigned int &i)	const {return fSharc_PAD_Wave.at(i);}				//!

  inline unsigned int GetMultiplicityFront() const {return fSharc_StripFront_DetectorNbr.size();}				//!
  inline unsigned int GetMultiplicityBack()  const {return fSharc_StripBack_DetectorNbr.size();}				//!
  inline unsigned int GetMultiplicityPAD()   const {return fSharc_PAD_DetectorNbr.size();}						//!
 
  //ClassDef(TSharcData,0)  //!  // SharcData structure 
};

#endif
