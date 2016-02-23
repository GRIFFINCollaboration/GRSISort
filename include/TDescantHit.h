#ifndef DESCANTHIT_H
#define DESCANTHIT_H

/** \addtogroup Detectors
 *  @{
 */

#include <cstdio>
#include <cmath>

#include "TVector3.h"

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
      
      Double_t fSlopePsd;
      std::vector<Short_t> fMonitor;
      Short_t fMax;
      std::vector<Int_t> fPartialSum;

      std::vector<Double_t> fDiffWaveform;
      std::vector<Double_t> fDiffIntWaveform;
      std::vector<Double_t> fShapedWaveform;
      std::vector<Double_t> fPsdMonitor;
      Double_t fMaxElement;
      Int_t    fZc2;

      std::vector<Double_t> fTimes;
      Double_t fCfdTime;

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
		
      /////////////////////////	replacing some TGRSIDetectorHit functions	/////////////////////////////////////
		inline Int_t   GetCfd() const                          { return fCfd&0x001fffff;}           //!<!
		inline Int_t   GetRemainder() const                    { return fCfd>>21;}           //!<!
		Double_t GetTime() const;
      
      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps); //!<!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Short_t> &monitor); //!<!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, unsigned int delay, unsigned int halfSmoothingWindow); //!<!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!<!
      std::vector<Int_t> CalculatePartialSum(); //!<!
      Int_t CalculatePsd(double fraction, unsigned int interpolation_steps); //!<!
      Int_t CalculatePsdAndPartialSums(double fraction, unsigned int interpolation_steps, std::vector<Int_t>& partialsums); //!<!
      
      Short_t CalculateWaveformMaximum(unsigned int halfwindowsize); //!<!
      std::vector<Int_t> CalculateAverageWaveform(unsigned int halfsmoothingwindow); //!<!
      Int_t CalculatePsd(double fraction, int halfsmoothingwindow, unsigned int interpolation_steps); //!<!
      Int_t CalculatePsdAndPartialSums(double fraction, int halfsmoothingwindow, unsigned int interpolation_steps, std::vector<Int_t>& partialsums); //!<!
      Double_t CalculateSlopePsd(unsigned int peakdelay, unsigned int psddelay, unsigned int interpolation_steps); //!<!
      std::vector<Int_t> CalculatePartialSum(int halfsmoothingwindow); //!<!
      bool AnalyzeKentuckyWaveform();                                          //!<!
      bool AnalyzeScopeWaveform();                                          //!<!
   
      Int_t CalculateCfdAndMonitorForSignal(double attenuation, unsigned int delay, unsigned int interpolationSteps, const std::vector<Double_t> inputSignal, std::vector<Double_t> &monitor); //!<!
      Int_t CalculateCfdForPartialSums(double attenuation, unsigned int delay, unsigned int interpolationStep); //!<!
      static std::vector<Double_t> Differentiator(const std::vector<Double_t> inputSignal, double timeconstant); //!<!
      static std::vector<Double_t> Integrator(const std::vector<Double_t> inputSignal, double timeconstant); //!<!
      static std::vector<Double_t> ShortVectorToDouble(const std::vector<Short_t> inputSignal); //!<!
      static std::vector<Double_t> IntVectorToDouble(const std::vector<Int_t> inputSignal); //!<!
      static Int_t CalculateZeroCrossing(std::vector<Double_t> inputSignal, unsigned int interpolationSteps); //!<!
      bool InFilter(Int_t);                                          //!<!
      
      bool AnalyzeWaveform();                                          //!<!

   public:
      void Copy(TObject&) const;        //!<!
      void Clear(Option_t *opt = "");		                    //!<!
      void Print(Option_t *opt = "") const;		                    //!<!
      
   private:
      TVector3 GetChannelPosition(Double_t dist = 222) const; //!<!
      
      /// \cond CLASSIMP
      ClassDef(TDescantHit,5)
      /// \endcond
};
/*! @} */
#endif
