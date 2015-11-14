#ifndef TSILIHIT_H
#define TSILIHIT_H

#include "Globals.h"

#include <cstdio>
#include <utility>

#include "TFragment.h"
#include "TChannel.h"
#include "TGRSIDetectorHit.h"
#include "TPulseAnalyzer.h"

class TSiLiHit : public TGRSIDetectorHit {
  public:
    TSiLiHit();
    TSiLiHit(TFragment &);	
    virtual ~TSiLiHit();
	 TSiLiHit(const TSiLiHit&);

    void Copy(TObject&) const;        //!
    void Clear(Option_t *opt="");
    void Print(Option_t *opt="") const;
    
    Double_t GetLed()      const {  return led;     }
    Short_t  GetSegment()  const {  return segment; }
    Double_t GetSig2Noise()const {  return sig2noise;}    
    Int_t GetRing()        const {  return 9-(segment/12); }
    Int_t GetSector()      const {  return segment%12; }
    Int_t GetPreamp()      const {  return  ((GetSector()/3)*2)+(((GetSector()%3)+GetRing())%2); }
    Double_t  GetTimeFit() const {  return time_fit; }

    void SetSegment(Short_t seg)       { segment = seg;	}
//     using TGRSIDetectorHit::SetPosition; //This is here to fix warnings. Will leave when lean-ness occurs
//     void SetPosition(TVector3 &vec)    { fposition = vec; }
    void SetVariables(TFragment &frag) { //SetEnergy(frag.GetEnergy());
// 					 SetAddress(frag.ChannelAddress);	
//                                          SetCfd(frag.GetCfd());
//                                          SetCharge(frag.GetCharge());
//                                          SetTimeStamp(frag.GetTimeStamp()); 
                                         //SetTime(frag.GetZCross());
					  led    = frag.GetLed(); }
    void SetWavefit(TFragment&);
    
  private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!  
	  
    //TVector3 position;  //held in base.
    Double_t    led;
    Short_t  segment;
    Double_t    time_fit;
    Double_t    sig2noise;
  
  ClassDef(TSiLiHit,7);

};



#endif
