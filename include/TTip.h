#ifndef TTIP_H
#define TTIP_H

#include "Globals.h"

#include <vector>
#include <cstdio>
#include <iostream>
#include <set>
#include <stdio.h>

#include "TTipHit.h"
#ifndef __CINT__
#include "TTipData.h"
#else
class TTipData;
#endif
#include "TVector3.h" 

#include "TGRSIDetector.h" 
#include "TObject.h"

class TTip : public TGRSIDetector {

  public:

    virtual ~TTip();
    TTip();  
    TTip(const TTip& rhs);

    TTipHit *GetTipHit(const int& i) ;//!
    TGRSIDetectorHit *GetHit(const int& i);
    Short_t GetMultiplicity() const         {  return tip_hits.size();}  //!

	 void BuildHits(TDetectorData *data =0,Option_t *opt = "");           //!
	 void FillData(TFragment*,TChannel*,MNEMONIC*);                           //!
	 void BuildHits(TFragment*, MNEMONIC*); //!
    void Copy(TObject &rhs) const;

    TTip& operator=(const TTip&);  //!

    void Clear(Option_t *opt = "");
    void Print(Option_t *opt = "") const;

   protected:
     void PushBackHit(TGRSIDetectorHit* tiphit);

  private:

    TTipData *tipdata;                                               //!  Used to build TIP Hits
    std::vector <TTipHit> tip_hits;                                  //   The set of detector hits

  private:
    //  SFU CsI waveform fitting routines.
    typedef struct WaveFormPar {
      //parameters for baseline
      int    baseline_range;
      double baseline; //baseline
      double baselineStDev; //baseline variance
      int    bflag; //flag for baseline determiniation
      
      //paremeters for exclusion zone
      double max; //max value of the filtered waveform 
      double tmax; //x position of the max value of the filtered waveform
      double baselineMin; //max crossing point for exclusion zone
      double baselineMax; //max crossing point for exclusion zone
      int    temax; //x position, exclusion zone upper limit
      int    temin; //x position, exclusion zone lower limit
      double afit,bfit; //parameters for the line which fits risetime above temax
      int    mflag; //flag for tmax found
      int    teflag; //flag for exclusion zone determined
          
      double t0; //required for compilation of map.c - check that it works
      double t0_local;
      
      //new stuff necessary for compiliation of Kris' waveform analyzer changes
      double b0;
      double b1;
      long double s0;
      long double s1;
      long double s2;
      double t90;
      double t50;
      double t30;
      double t10;
      double t10t90;
      int t10_flag;
      int t30_flag;
      int t50_flag;
      int t90_flag;
      int t10t90_flag;
      int    thigh;
      double sig2noise;
    } WaveFormPar;

    typedef struct WaveShapPar { 
      double chisq;
      int    ndf;
      int    type;
      std::vector<double> t;  //decay constants for the fits
      std::vector<double> am; //associated aplitudes for the decay constants
      double rf[5];

      //new stuff necessary for compiliation of Kris' waveform analyzer changes
      long double chisq_ex;
      long double chisq_f;
      int    ndf_ex;
      int    ndf_f;
    } WaveShapPar;  

    WaveFormPar GetExclusionZone();
    WaveShapPar GetShape(WaveFormPar&);


  public:

  ClassDef(TTip,1);

}; 

#endif

