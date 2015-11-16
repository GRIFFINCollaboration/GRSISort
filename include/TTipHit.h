#ifndef TTIPHIT_H
#define TTIPHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"
#include "TPulseAnalyzer.h"

#include "TVector3.h"


#include "TGRSIDetectorHit.h"

class TTipHit : public TGRSIDetectorHit {
  public:
    TTipHit();
    virtual ~TTipHit();
    TTipHit(const TTipHit&);

  private:
    Int_t    filter;    // 
    Double_t fPID;       // 

    Double_t fast_amplitude;
    Double_t slow_amplitude;
    Double_t gamma_amplitude;
   
	bool	csi_flag;

	Int_t	 tip_channel;

    Double_t    time_fit;
    Double_t    sig2noise;

  public:
    /////////////////////////    /////////////////////////////////////
    inline void SetFilterPattern(const int &x)    { filter   = x; }   //! 
	inline void SetTipChannel(const int x)		  { tip_channel = x; } //!

    inline Int_t    GetFiterPatter()           { return filter;   }  //!
    inline Double_t GetPID()                   { return fPID;      }  //!
	inline Double_t GetFitTime()			   { return time_fit;	} //!
	inline Double_t GetSignalToNoise()		   { return sig2noise;	} //!
	inline Int_t	GetTipChannel()			   { return tip_channel; } //!

	inline bool   IsCsI()								{return csi_flag; }
	inline void	  SetCsI(bool flag="true")   { csi_flag = flag; }

    bool   InFilter(Int_t);                                         //!

    void SetVariables(TFragment &frag) { SetCfd(frag.GetCfd());
                                         SetCharge(frag.GetCharge());
                                         SetTimeStamp(frag.GetTimeStamp()); }

	void SetUpNumbering(TChannel &chan) { MNEMONIC mnemonic;
										  TChannel *channel = GetChannel();
										  if(!channel){
										    Error("SetDetector","No TChannel exists for address %u",GetAddress());
										    return;
										  }
										  ClearMNEMONIC(&mnemonic);
										  ParseMNEMONIC(channel->GetChannelName(),&mnemonic); 
										  Int_t tmp = (int16_t)atoi(mnemonic.arraysubposition.c_str()); 
										  SetTipChannel((Int_t)(10*mnemonic.arrayposition + tmp)); 
										  if(mnemonic.subsystem.compare("W")==0 || mnemonic.subsystem.compare("C")==0)
											SetCsI(); 
										  }

	void SetWavefit(TFragment&);
    void SetPID(TFragment&);

  public:
    void Clear(Option_t *opt = "");                        //!
    void Print(Option_t *opt = "") const;                  //!
    virtual void Copy(TObject&) const;                     //!

  private:
    TVector3 GetChannelPosition(Double_t dist=0) const   { return TVector3();}

    ClassDef(TTipHit,1);

};

#endif
