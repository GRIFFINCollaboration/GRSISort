#ifndef TBGODATA_H
#define TBGODATA_H


#include <cstdlib>
#include <cstdio>
#include <vector>

#include "Globals.h"
#include "TFragment.h"
#include "TChannel.h"

#ifndef __CINT__
#include "TGRSIDetectorData.h"
#else
class TGRSIDetectorData;
#endif


////////////////////////////////////////////////////////////////
//                                                            //
// TBGOData                                                   //
//                                                            //
// This Class contains the BGO data that is processed in      //
// the TAnalysisTreeBuilder. This class is not written to a   //
// tree and is not a ROOT class.                              //
//                                                            //
////////////////////////////////////////////////////////////////

class TBGOData : public TGRSIDetectorData	{

	private:

		std::vector<UShort_t> fBGO_CloverNbr;		//!The clover that the BGO is on
		std::vector<UShort_t> fBGO_CrystalNbr;		//!The BGO crystal
		std::vector<UShort_t> fBGO_PmNbr;		//!
		std::vector<Int_t>    fBGO_Charge;		//!The Charge collected by the BGO
		std::vector<Double_t> fBGO_Energy;		//!The Energy collected by the BGO
		std::vector<Double_t> fBGO_TimeCFD;		//!The CFD time of the hit in the BGO
		std::vector<Double_t> fBGO_TimeLED;		//!The LED time of the hit in the BGO
		std::vector<Double_t> fBGO_Time;		//!The time stamp of the hit in the BGO

		std::vector<std::vector<int> > fBGO_Wave;	//!The waveform collected by the BGO

		static bool fIsSet; //!

	public:
		TBGOData();					//!
		virtual ~TBGOData();				//!

		static void Set(bool flag=true) { fIsSet=flag; } //!
		static bool IsSet() { return fIsSet; }           //!

 		virtual void Clear(Option_t *opt = "");		//!
		virtual void Print(Option_t *opt = "");		//!
  
		/////////////////////           SETTERS           ////////////////////////
		inline void SetBGOCloverNbr(const UShort_t &BGOClvNbr)		{fBGO_CloverNbr.push_back(BGOClvNbr);}  //!
		inline void SetBGOCrystalNbr(const UShort_t &BGOCryNbr)		{fBGO_CrystalNbr.push_back(BGOCryNbr);}	//!
		inline void SetBGOPmNbr(const UShort_t &BGOPmNbr)		{fBGO_PmNbr.push_back(BGOPmNbr);}	//!
		inline void SetBGOCharge(const Int_t &BGOCharge)		{fBGO_Charge.push_back(BGOCharge);}	//!	
		inline void SetBGOEnergy(const Double_t &BGOEnergy)		{fBGO_Energy.push_back(BGOEnergy);}	//!
		inline void SetBGOCFD(const Double_t &BGOTimeCFD)		{fBGO_TimeCFD.push_back(BGOTimeCFD);}	//!
		inline void SetBGOLED(const Double_t &BGOTimeLED)		{fBGO_TimeLED.push_back(BGOTimeLED);}	//!
		inline void SetBGOTime(const Double_t &BGOTime)			{fBGO_Time.push_back(BGOTime);}		//!

		inline void SetBGOWave(const std::vector<int> &BGOWave)		{fBGO_Wave.push_back(BGOWave);}		//!

		inline void SetBGO(const UShort_t &BGOCloverNbr, const UShort_t &BGOCrystalNbr, const UShort_t &BGOPmNbr, const Int_t &BGOCharge, const Double_t &BGOEnergy, const Double_t &BGOTimeCFD, const Double_t &BGOTimeLED, const Double_t &BGOTime = 0)	{
			SetBGOCloverNbr(BGOCloverNbr);
			SetBGOCrystalNbr(BGOCrystalNbr);
			SetBGOPmNbr(BGOPmNbr);
			SetBGOCharge(BGOCharge);
			SetBGOEnergy(BGOEnergy);
			SetBGOCFD(BGOTimeCFD);
			SetBGOLED(BGOTimeLED);
			SetBGOTime(BGOTime);
		};	//!

		inline void SetBGO(TFragment *frag,TChannel *channel,MNEMONIC *mnemonic) {
         //Sets the crystal number of the BGO based on the color in the MNEMONIC.
      
      if(!frag || !channel || !mnemonic)
        return;
         
      
      if(frag->Charge.size() == 0 || frag->Cfd.size() == 0 || frag->Led.size() == 0 || frag->Zc.size() == 0)
			  return;
         
			SetBGOCloverNbr(mnemonic->arrayposition);

			UShort_t CrystalNbr=5;
			if(mnemonic->arraysubposition.compare(0,1,"B")==0)
				CrystalNbr=0;
			else if(mnemonic->arraysubposition.compare(0,1,"G")==0)
				CrystalNbr=1;
			else if(mnemonic->arraysubposition.compare(0,1,"R")==0)
				CrystalNbr=2;
			else if(mnemonic->arraysubposition.compare(0,1,"W")==0)
				CrystalNbr=3;
			SetBGOCrystalNbr(CrystalNbr);

			SetBGOPmNbr(mnemonic->segment );
			SetBGOCharge(frag->Charge.at(0));
			SetBGOCFD(frag->Cfd.at(0));
			SetBGOLED(frag->Led.at(0));
			SetBGOTime(frag->Zc.at(0));
		};	
		
		/////////////////////           GETTERS           ////////////////////////
    inline UShort_t GetBGOCloverNumber(const unsigned &i)       {return fBGO_CloverNbr.at(i);} //!
		inline UShort_t GetBGOCoreNumber(const unsigned &i)      {return fBGO_CrystalNbr.at(i);}//!
		inline UShort_t GetBGOPmNbr(const unsigned int &i)       {return fBGO_PmNbr.at(i); }	//!
		inline Int_t    GetBGOCharge(const unsigned int &i)      {return fBGO_Charge.at(i);}	//!		
		inline Double_t GetBGOEnergy(const unsigned int &i)      {return fBGO_Energy.at(i);}	//!
		inline Double_t GetBGOCFD(const unsigned int &i)     {return fBGO_TimeCFD.at(i);}	//!
		inline Double_t GetBGOLED(const unsigned int &i)     {return fBGO_TimeLED.at(i);}	//!
		inline Double_t GetBGOTime(const unsigned int &i)    	 {return fBGO_Time.at(i);}	//!

		inline std::vector<int> GetBGOWave(const unsigned int &i) {return fBGO_Wave.at(i);}	//!

		inline unsigned int GetBGOMultiplicity()  {return fBGO_PmNbr.size();}			//!

  //ClassDef(TBGOData,0) //!  // TBgoData structure
};









#endif








