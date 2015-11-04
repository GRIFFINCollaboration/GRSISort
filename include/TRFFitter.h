#ifndef __TRF_FITTER_H
#define __TRF_FITTER_H

#include <vector>

#include <TNamed.h>
#include <Rtypes.h>

class TFragment;

class TRFFitter : public TNamed {
  public:
    typedef Short_t vector_element_t;
  private:
    void CalculateDerivative(const std::vector<TRFFitter::vector_element_t> &waveform);
    void CalculateDoubleDerivative(const std::vector<TRFFitter::vector_element_t> &waveform);
  public:
    TRFFitter();
    ~TRFFitter();

    double GetPhase()     { return phase;	}
    double GetTime()	     { return rftime;	}
    time_t GetMidasTime() { return midastime; }
    Long_t GetTimeStamp() { return timestamp; }


    const std::vector<double> &GetDerivative() {return derivative;}
    const std::vector<double> &GetDoubleDerivative() { return doubleDerivative; }
    const std::vector<double> &GetResiduals() {return residuals;}
    const std::vector<double> &GetScaledResiduals() {return scaledResiduals;}
    void FindPhase(const std::vector<TRFFitter::vector_element_t> &waveform, Bool_t calcErrors);
    void FindPhase(const TFragment &frag, Bool_t calcErrors = false);

    bool IsSet() { return set; }

    void Clear(Option_t *opt = "");
    void Print(Option_t *opt = "") const;

    void DrawWave(const TFragment &frag);


  private:   
    bool   set;
    time_t midastime;
    Long_t timestamp;

    double rf_omega; 
    double phase;
    double rftime;
    double amplitude;
    double offset;
    
    double chisquare;
    double error_phase;
    double error_amplitude;
    double error_offset;
    double correlation;
    
    double scale_factor;
    double signal_noise;

    std::vector<double> derivative;  
    std::vector<double> doubleDerivative;
    std::vector<double> residuals;
    std::vector<double> scaledResiduals;
  
    //std::vector<Short_t> waveform;

   ClassDef(TRFFitter, 0);
};

#endif // __TRF_FITTER_H
