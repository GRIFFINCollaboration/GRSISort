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
         if(x.size() <= 8) {
            return;
         }
         size_t length = x.size() - (x.size()%8);
         fwaveform.resize(length-8);
         for(size_t i = 0; i < length; ++i) {
            // reorder so that samples 0-7 are: 7,6,1,0,3,2,5,4
            //                                  0,1,2,3,4,5,6,7
            // pairwise swap: 0->1,1->0 => i+1 (1,2) => i+1-(2*i%2) (1,0)
            // 67,01,32,45: shift all by +2, except for the last pair which need to be shifted by -6
            Int_t reordered = i-2;
            reordered = reordered+1-2*(reordered%2);
            if(reordered >= (Int_t) fwaveform.size()) {
               continue;
            }
            if(reordered%8 < 6) {
               //std::cout<<i<<" => "<<reordered+2<<std::endl;
               fwaveform[reordered+2] = x[i];
            } else {
               //std::cout<<i<<" => "<<reordered-6<<std::endl;
               fwaveform[reordered-6] = x[i];
            }
         }
      } //!

		/////////////////////////		/////////////////////////////////////
      inline Int_t    GetFilterPattern()    const     { return filter;   }  //!
      TVector3 GetPosition(Double_t dist = 0) const; //!
  //    inline std::vector<Short_t> GetWaveform() 		{ return waveform; }  //!

      Int_t CalculateCfd(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps); //!
      Int_t CalculateCfdAndMonitor(double attenuation, int delay, int halfsmoothingwindow, int interpolation_steps, std::vector<Short_t> &monitor); //!
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

	ClassDef(TSceptarHit,2) //Stores the information for a SceptarHit
};

#endif
