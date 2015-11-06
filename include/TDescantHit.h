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
         std::vector<Short_t>* waveform = GetWaveform();
         if(x.size() <= 8) {
            return;
         }
         size_t length = x.size() - (x.size()%8);
         waveform->resize(length-8);
         for(size_t i = 0; i < length; ++i) {
            // reorder so that samples 0-7 are: 7,6,1,0,3,2,5,4
            //                                  0,1,2,3,4,5,6,7
            // pairwise swap: 0->1,1->0 => i+1 (1,2) => i+1-(2*i%2) (1,0)
            // 67,01,32,45: shift all by +2, except for the last pair which need to be shifted by -6
            Int_t reordered = i-2;
            reordered = reordered+1-2*(reordered%2);
            if(reordered >= (Int_t) waveform->size()) {
               continue;
            }
            if(reordered%8 < 6) {
               //std::cout<<i<<" => "<<reordered+2<<std::endl;
               waveform->at(reordered+2) = x[i];
            } else {
               //std::cout<<i<<" => "<<reordered-6<<std::endl;
               waveform->at(reordered-6) = x[i];
            }
         }
      } //!<!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()       { return fFilter;   }  //!<!
      inline Int_t    GetPsd()                 { return fPsd;      }  //!<!
      inline Int_t    GetZc()                  { return fZc;      }  //!<!
      inline Int_t    GetCcShort()             { return fCcShort;      }  //!<!
      inline Int_t    GetCcLong()              { return fCcLong;      }  //!<!

      Int_t CalculateCfd(double attenuation, unsigned int delay, int halfsmoothingwindow, int interpolation_steps); //!<!
      Int_t CalculateCfdAndMonitor(double attenuation, unsigned int delay, int halfsmoothingwindow, int interpolation_steps, std::vector<Short_t> &monitor); //!<!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfsmoothingwindow); //!<!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfsmoothingwindow); //!<!
      std::vector<Int_t> CalculatePartialSum(); //!<!
      Int_t CalculatePsd(double fraction, int interpolation_steps); //!<!
      Int_t CalculatePsdAndPartialSums(double fraction, int interpolation_steps, std::vector<Int_t>& partialsums); //!<!

      bool   InFilter(Int_t);                                          //!<!

      bool AnalyzeWaveform();                                          //!<!

	public:
      void Copy(TObject&) const;        //!<!
		void Clear(Option_t *opt = "");		                    //!<!
		void Print(Option_t *opt = "") const;		                    //!<!

   private:
      TVector3 GetChannelPosition(Double_t dist = 0) const; //!<!

/// \cond CLASSIMP
	ClassDef(TDescantHit,4)
/// \endcond
};
/*! @} */
#endif
