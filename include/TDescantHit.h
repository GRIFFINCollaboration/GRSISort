#ifndef DESCANTHIT_H
#define DESCANTHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

#include "TFragment.h"
#include "TChannel.h"

#include "TGRSIDetectorHit.h"

class TDescantHit : public TGRSIDetectorHit {
   public:
      TDescantHit();
      virtual ~TDescantHit();
      TDescantHit(const TDescantHit&);
      
   private:
      Int_t    fFilter;
      Int_t    fPsd;
      Int_t    fZc;
      Int_t    fCcShort;
      Int_t    fCcLong;
      
   public:
      /////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter   = x; }   //!<!
      inline void SetPsd(const int &x)             { fPsd      = x; }   //!<!
      inline void SetZc(const int &x)              { fZc       = x; }   //!<!
      inline void SetCcShort(const int &x)         { fCcShort  = x; }   //!<!
      inline void SetCcLong(const int &x)          { fCcLong   = x; }   //!<!
      
      inline void SetWaveform(std::vector<Short_t> x) {
         std::vector<Short_t> *waveform = GetWaveform();
         *waveform=x;
      }  //!<!
      
      /////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()       { return fFilter;   }  //!<!
      inline Int_t    GetPsd()                 { return fPsd;      }  //!<!
      inline Int_t    GetZc()                  { return fZc;      }  //!<!
      inline Int_t    GetCcShort()             { return fCcShort;      }  //!<!
      inline Int_t    GetCcLong()              { return fCcLong;      }  //!<!
      
      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps); //!<!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Short_t> &monitor); //!<!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, unsigned int delay, unsigned int halfSmoothingWindow); //!<!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!<!
      std::vector<Int_t> CalculatePartialSum(); //!<!
      Int_t CalculatePsd(double fraction, unsigned int interpolation_steps); //!<!
      Int_t CalculatePsdAndPartialSums(double fraction, unsigned int interpolation_steps, std::vector<Int_t>& partialsums); //!<!
      
      bool InFilter(Int_t);                                          //!<!
      
      bool AnalyzeWaveform();                                          //!<!
      
      TVector3 GetPosition(Double_t dist) const; //!<!
      TVector3 GetPosition() const; //!<!

   public:
      void Copy(TObject&) const;        //!<!
      void Clear(Option_t *opt = "");		                    //!<!
      void Print(Option_t *opt = "") const;		                    //!<!
      
   private:
      Double_t GetDefaultDistance() const { return 222.; }
      
      /// \cond CLASSIMP
      ClassDef(TDescantHit,4)
      /// \endcond
};
/*! @} */
#endif
