#ifndef TTIPHIT_H
#define TTIPHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"
#include "TPulseAnalyzer.h"

#include "TGRSIDetectorHit.h"

class TTipHit : public TGRSIDetectorHit {
  public:
    TTipHit();
	TTipHit(TFragment &);	
    virtual ~TTipHit();
    TTipHit(const TTipHit&);

  private:
    Int_t    fFilter;    // 
    Double_t fPID;       // 

    Double_t fFastAmplitude;
    Double_t fSlowAmplitude;
    Double_t fGammaAmplitude;
   
	bool csi_flag;

	 Int_t	 fTipChannel;

    Double_t   fTimeFit;
    Double_t   fSig2Noise;

  public:
    /////////////////////////    /////////////////////////////////////
    inline void SetFilterPattern(const int &x)    { fFilter   = x; }   //!<! 
    inline void SetPID(Double_t x)                { fPID = x;     }   //!<!
	 inline void SetTipChannel(const int x)		  { fTipChannel = x; } //!<!

    inline Int_t    GetFiterPatter()              { return fFilter;     } //!<!
    inline Double_t GetPID()                      { return fPID;        } //!<!
	 inline Double_t GetFitTime()			           { return fTimeFit;	   } //!<!
	 inline Double_t GetSignalToNoise()		        { return fSig2Noise;	} //!<!
	 inline Int_t	  GetTipChannel()			        { return fTipChannel; } //!<!

	 inline bool IsCsI()									  { return csi_flag; } //!<!
	 inline void SetCsI(bool flag="true")	        { csi_flag = flag; } //!<!

    bool   InFilter(Int_t);                                         //!<!

    //void SetVariables(TFragment &frag) { SetAddress(frag.ChannelAddress);
	//									 SetCfd(frag.GetCfd());
    //                                   SetCharge(frag.GetCharge());
    //                                     SetTimeStamp(frag.GetTimeStamp()); }

	 void SetUpNumbering(TChannel &chan) { 
			MNEMONIC mnemonic;
			TChannel *channel = GetChannel();
			if(!channel) {
				Error("SetDetector","No TChannel exists for address %u",GetAddress());
				return;
			}
			ClearMNEMONIC(&mnemonic);
			ParseMNEMONIC(channel->GetChannelName(), &mnemonic); 
			Int_t tmp = atoi(mnemonic.arraysubposition.c_str()); 
			SetTipChannel(10*mnemonic.arrayposition + tmp); 
			if(mnemonic.subsystem.compare(0,1,"W")==0)
				SetCsI();
	 }

    void SetWavefit(TFragment&);

		void SetPID(TFragment&);

  public:
    void Clear(Option_t *opt = "");                        //!<!
    void Print(Option_t *opt = "") const;                  //!<!
    virtual void Copy(TObject&) const;                     //!<!

  private:
    TVector3 GetChannelPosition(Double_t dist=0) const   { return TVector3();}

/// \cond CLASSIMP
    ClassDef(TTipHit,1);
/// \endcond
};
/*! @} */
#endif
