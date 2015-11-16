#ifndef DESCANTHIT_H
#define DESCANTHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"
#include "TCrystalHit.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TDescantHit : public TGRSIDetectorHit {
   public:
      TDescantHit();
      virtual ~TDescantHit();
      TDescantHit(const TDescantHit&);
      
   private:
      Int_t    filter;
      Int_t    psd;
      Int_t    zc;
      Int_t    ccShort;
      Int_t    ccLong;
      
   public:
      /////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { filter   = x; }   //!
      inline void SetPsd(const int &x)             { psd      = x; }   //!
      inline void SetZc(const int &x)              { zc       = x; }   //!
      inline void SetCcShort(const int &x)         { ccShort  = x; }   //!
      inline void SetCcLong(const int &x)          { ccLong   = x; }   //!
      
      inline void SetWaveform(std::vector<Short_t> x) {
         std::vector<Short_t> *waveform = GetWaveform();
         *waveform=x;
      } //!
      
      /////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()       { return filter;   }  //!
      inline Int_t    GetPsd()                 { return psd;      }  //!
      inline Int_t    GetZc()                  { return zc;      }  //!
      inline Int_t    GetCcShort()             { return ccShort;      }  //!
      inline Int_t    GetCcLong()              { return ccLong;      }  //!
      
      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps); //!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Short_t> &monitor); //!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow); //!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!
      std::vector<Int_t> CalculatePartialSum(); //!
      Int_t CalculatePsd(double fraction, unsigned int interpolation_steps); //!
      Int_t CalculatePsdAndPartialSums(double fraction, unsigned int interpolation_steps, std::vector<Int_t>& partialsums); //!
      
      bool   InFilter(Int_t);                                          //!
      
      //     static bool CompareEnergy(TDescantHit*,TDescantHit*);            //!
      //     void Add(TDescantHit*);                                          //!
      
      bool AnalyzeWaveform();                                          //!
      
   public:
      void Copy(TObject&) const;        //!
      void Clear(Option_t *opt = "");		                    //!
      void Print(Option_t *opt = "") const;		                    //!
   private:
      TVector3 GetChannelPosition(Double_t dist = 222) const; //!
      
      ClassDef(TDescantHit,4)
};

#endif
