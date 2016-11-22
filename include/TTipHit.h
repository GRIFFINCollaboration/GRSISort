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
		TTipHit(const TFragment &);	
		virtual ~TTipHit();
		TTipHit(const TTipHit&);

	private:
		Int_t    fFilter;    // 
		Double_t fPID;       // 
		Int_t		 fChiSq;

		//Double_t fFastAmplitude;
		//Double_t fSlowAmplitude;
		//Double_t fGammaAmplitude;

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
		inline Int_t		GetFitChiSq()									{ return fChiSq;			} //!<!
		inline Double_t GetFitTime()			           { return fTimeFit;	   } //!<!
		inline Double_t GetSignalToNoise()		        { return fSig2Noise;	} //!<!
		inline Int_t	  GetTipChannel()			        { return fTipChannel; } //!<!

		inline bool IsCsI()									  { return csi_flag; } //!<!
		inline void SetCsI(bool flag="true")	        { csi_flag = flag; } //!<!
		inline void SetFitChiSq(int chisq)						{ fChiSq = chisq; }	//!<!

		bool   InFilter(Int_t);                                         //!<!

		//void SetVariables(const TFragment &frag) { SetAddress(frag.ChannelAddress);
		//									 SetCfd(frag.GetCfd());
		//                                   SetCharge(frag.GetCharge());
		//                                     SetTimeStamp(frag.GetTimeStamp()); }

		void SetUpNumbering(TChannel *chan) { 
			TChannel *channel = GetChannel();
			if(!channel) {
				Error("SetDetector","No TChannel exists for address %u",GetAddress());
				return;
			}
			Int_t tmp = atoi(channel->GetMnemonic()->ArraySubPositionString().c_str()); 
			SetTipChannel(10*channel->GetMnemonic()->ArrayPosition() + tmp); 
			if(channel->GetMnemonic()->SubSystemString().compare(0,1,"W")==0)
				SetCsI();
		}

		void SetWavefit(const TFragment&);

		void SetPID(const TFragment&);

	public:
		void Clear(Option_t *opt = "");                        //!<!
		void Print(Option_t *opt = "") const;                  //!<!
		virtual void Copy(TObject&) const;                     //!<!

/// \cond CLASSIMP
	ClassDef(TTipHit,1);
/// \endcond
};
/*! @} */
#endif
