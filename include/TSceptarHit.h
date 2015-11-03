#ifndef SCEPTARHIT_H
#define SCEPTARHIT_H

#include <cstdio>
#include <cmath>

#include "TFragment.h"
#include "TChannel.h"

#include "TVector3.h"

#include "TGRSIDetectorHit.h"

class TSceptarHit : public TGRSIDetectorHit {
  public:
    TSceptarHit();
    virtual ~TSceptarHit();
    TSceptarHit(const TSceptarHit&);

  private:
    Int_t    fFilter;
   
  public:
		/////////////////////////		/////////////////////////////////////
      inline void SetFilterPattern(const int &x)   { fFilter   = x; }   //! 

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
      } //!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     { return fFilter;   }  //!

      Int_t CalculateCfd(double attenuation, int delay, int halfSmoothingWindow, int interpolationSteps); //!
      Int_t CalculateCfdAndMonitor(double attenuation, int delay, int halfSmoothingWindow, int interpolationSteps, std::vector<Short_t>& monitor); //!
      std::vector<Short_t> CalculateCfdMonitor(double attenuation, int delay, int halfSmoothingWindow); //!
      std::vector<Short_t> CalculateSmoothedWaveform(unsigned int halfSmoothingWindow); //!

      bool   InFilter(Int_t);                                          //!

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
