#ifndef SCEPTARHIT_H
#define SCEPTARHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TSceptarHit : public TGRSIDetectorHit {
  public:
    TSceptarHit();
    virtual ~TSceptarHit();
    TSceptarHit(const TSceptarHit&);

  private:
    Int_t    filter;
//    std::vector<Short_t> waveform;
   
  public:
  // void SetHit();
		/////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //! 
   //   inline void SetPosition(TVector3 x)          { position = x; }   //!

      inline void SetWaveform(std::vector<Short_t> x) {
         std::vector<Short_t> *waveform = GetWaveform();
         *waveform=x;
      } //!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     { return filter;   }  //!
  //    inline std::vector<Short_t> GetWaveform() 		{ return waveform; }  //!

      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps); //!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Short_t> &monitor); //!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow); //!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!

      bool   InFilter(Int_t);                                          //!

 //     static bool CompareEnergy(TSceptarHit*,TSceptarHit*);            //!
 //     void Add(TSceptarHit*);                                          //!
   bool AnalyzeWaveform();                                          //!

	public:
		void Clear(Option_t *opt = "");		                    //!
		void Print(Option_t *opt = "") const;		                    //!
      virtual void Copy(TObject&) const;        //!

   private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!

	ClassDef(TSceptarHit,2) //Stores the information for a SceptarHit
};

#endif
