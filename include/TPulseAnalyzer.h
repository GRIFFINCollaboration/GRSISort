#ifndef TPULSE_ANALYZER_H
#define TPULSE_ANALYZER_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include "TFragment.h"
#include "TGRSIFunctions.h"

#include <vector>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cmath>

#include "TNamed.h"
#include "Rtypes.h"
#include "TH1.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"

////////////////////////////////////////////////////////////////////////////////
/// \class TPulseAnalyzer
///
/// Mostly a direct port of SFU code
/// I have stripped out some surplus and encapsulated it, but I havent changed much
/// I'm sure there is more that can be stripped and reformated but it is working currently.
///
////////////////////////////////////////////////////////////////////////////////

class TPulseAnalyzer {
private:
   struct WaveFormPar {
      // parameters for baseline
      int    baseline_range;
      double baseline;           // baseline
      double baselineStDev;      // baseline variance
      double baselinefin;        // baseline
      double baselineStDevfin;   // baseline variance
      bool   bflag;              // flag for baseline determiniation
      // paramaters for oscillation
      double baseamp;     // amplitude of baseline oscillations
      double basefreq;    // tick frequency of baseline oscillations
      double basephase;   // flag for baseline determination
      int    osciflag;    // flag for baseline determination
      // paremeters for exclusion zone
      double max;           // max value of the filtered waveform
      double tmax;          // x position of the max value of the filtered waveform
      double baselineMin;   // max crossing point for exclusion zone
      double baselineMax;   // max crossing point for exclusion zone
      int    temax;         // x position, exclusion zone upper limit
      int    temin;         // x position, exclusion zone lower limit
      double afit, bfit;    // parameters for the line which fits risetime above temax
      int    mflag;         // flag for tmax found
      int    teflag;        // flag for exclusion zone determined
      double t0;            // required for compilation of map.c - check that it works
      double t0_local;
      // new stuff necessary for compiliation of Kris' waveform analyzer changes
      double      b0;
      double      b1;
      long double s0;
      long double s1;
      long double s2;
      double      t90;
      double      t50;
      double      t30;
      double      t10;
      double      t10t90;
      int         t10_flag;
      int         t30_flag;
      int         t50_flag;
      int         t90_flag;
      int         t10t90_flag;
      int         thigh;
      double      sig2noise;
      double      amplitude;   // amplitude from sili fits
      double      tauDecay;
      double      tauRise;
   };

   struct LinePar {
      double slope;
      double intercept;
      double chisq;
      double ndf;
   };

   struct ParPar {
      double constant;
      double linear;
      double quadratic;
      double chisq;
      double ndf;
   };

   struct SinPar {
      double A;
      double t0;
      double C;
   };

   struct ShapePar {
      double chisq;
      int    ndf;
      int    type;
      // decay constants for the fits
      long double t[5];    // NOLINT(*-avoid-c-arrays)
                           // associated aplitudes for the decay constants
      long double am[5];   // NOLINT(*-avoid-c-arrays)
      double      rf[5];   // NOLINT(*-avoid-c-arrays)

      // new stuff necessary for compiliation of Kris' waveform analyzer changes
      long double chisq_ex;
      long double chisq_f;
      int         ndf_ex;
      int         ndf_f;
   };

public:
   TPulseAnalyzer();
   explicit TPulseAnalyzer(const TFragment& fragment, double = 0);
   explicit TPulseAnalyzer(const std::vector<Short_t>& wave, double = 0, std::string name = "");
   TPulseAnalyzer(const TPulseAnalyzer&)                = default;
   TPulseAnalyzer(TPulseAnalyzer&&) noexcept            = default;
   TPulseAnalyzer& operator=(const TPulseAnalyzer&)     = default;
   TPulseAnalyzer& operator=(TPulseAnalyzer&&) noexcept = default;
   virtual ~TPulseAnalyzer();

   void SetData(const TFragment& fragment, double = 0);
   void SetData(const std::vector<Short_t>& wave, double = 0);
   void Clear(Option_t* opt = "");
   bool IsSet() const { return set; }

   inline double Get_wpar_T0() { return cWpar->t0; }
   inline double Get_wpar_baselinefin() { return cWpar->baselinefin; }
   inline double Get_wpar_amplitude() { return cWpar->amplitude; }
   inline double Get_wpar_decay() { return cWpar->tauDecay; }
   inline double Get_wpar_rise() { return cWpar->tauRise; }

   bool        SiliShapePrepare(double tauDecay, double tauRise);
   bool        GetSiliShape(double tauDecay, double tauRise);                                            // Added for Spice, parameters to be found : t0 and Amplitude
   bool        GetSiliShapeTF1(double tauDecay, double tauRise, double baseline, double basefreq = 0);   // Added for Spice, full slow fit to establish parameters, needs initial estimates
   inline bool Get_bflag() { return cWpar->bflag; }
   TF1         Getsilifit();
   double      GetsiliSmirnov();
   void        Drawsilifit();

   static double SiLiFitFunction(double* i, double* p);

   double     fit_newT0();
   double     fit_rf(double = 2 * 8.48409);
   double     get_sig2noise();
   int16_t    good_baseline();
   void       print_WavePar();
   TH1I*      GetWaveHist();
   TGraph*    GetWaveGraph();
   static int fNameIter;
   void       DrawWave();
   void       DrawT0fit();
   void       DrawRFFit();

   // CsI functions:
   double CsIPID();
   double CsIt0();
   void   DrawCsIExclusion();
   void   DrawCsIFit();
   int    GetCsIChiSq();
   int    GetCsIFitType();

private:
   bool         set{false};
   WaveFormPar* cWpar{nullptr};
   int          cN{0};
   // TFragment* frag;
   std::vector<Short_t> cWavebuffer;
   SinPar*              spar{nullptr};
   ShapePar*            shpar{nullptr};
   WaveFormPar*         csiTestWpar[4]{nullptr};    // NOLINT(*-avoid-c-arrays)
   ShapePar*            csiTestShpar[4]{nullptr};   // NOLINT(*-avoid-c-arrays)

   std::string fName;

   // pulse fitting parameters
   int    FILTER;    // integration region for noise reduction (in samples)
   int    T0RANGE;   // tick range over which baseline is calulated
   double LARGECHISQ;

   // linear equation dataholders
   int         lineq_dim;
   long double lineq_matrix[20][20];   // NOLINT(*-avoid-c-arrays)
   long double lineq_vector[20];       // NOLINT(*-avoid-c-arrays)
   long double lineq_solution[20];     // NOLINT(*-avoid-c-arrays)
   long double copy_matrix[20][20];    // NOLINT(*-avoid-c-arrays)

   // CsI functions
   void   GetCsIExclusionZone();
   double GetCsITau(int, ShapePar*);
   double GetCsIt0(ShapePar*, WaveFormPar*);
   int    GetCsIShape();
   int    FitCsIShape(int, ShapePar*, WaveFormPar*);

   bool   CsISet;
   double EPS;

   void SetCsI(bool option = true) { CsISet = option; }
   bool CsIIsSet() const { return CsISet; }

   // internal methods
   int         solve_lin_eq();
   long double determinant(int);

   int    fit_parabola(int, int, ParPar*);
   int    fit_smooth_parabola(int, int, double, ParPar*);
   int    fit_line(int, int, LinePar*);
   double get_linear_T0();
   double get_parabolic_T0();
   double get_smooth_T0();

   void get_baseline();
   void get_baseline_fin();
   void get_tmax();

   double get_tfrac(double, double, double);
   void   get_t10();
   void   get_t30();
   void   get_t50();
   void   get_t90();

   double get_sin_par(double);

   void GetQuickPara();

   // bad chi squares for debugging
   const static int BADCHISQ_SMOOTH_T0   = -1024 - 2;    // smooth_t0 gives bad result
   const static int BADCHISQ_PAR_T0      = -1024 - 3;    // parabolic_t0 gives bad result
   const static int BADCHISQ_LIN_T0      = -1024 - 4;    // linear_t0 gives bad result
   const static int BADCHISQ_MAT         = -1024 - 5;    // matrix for fit is not invertable
   const static int BADCHISQ_FAIL_DIRECT = -1024 - 9;    // generic fit failure
   const static int BAD_EXCLUSION_ZONE   = -1024 - 10;   // fails finding exclusion zone
   const static int BAD_MAX              = -1024 - 11;
   const static int BAD_BASELINE_RANGE   = -1024 - 12;
   const static int BAD_PID              = -1024;   // PID value returned when the fit fails

   // new definitions for Kris' changes to the waveform analyzer
   const static int PIN_BASELINE_RANGE = 16;   // minimum ticks before max for a valid signal
   const static int MAX_SAMPLES        = 4096;

   const static int CSI_BASELINE_RANGE = 50;    // baseline range in channels
   const static int NOISE_LEVEL_CSI    = 100;   // noise level for CsI
   const static int NSHAPE             = 5;     // number of trial functions for waveform fit

   const static int BADCHISQ_T0   = -1024 - 7;
   const static int BADCHISQ_NEG  = -1024 - 1;
   const static int BADCHISQ_AMPL = -1024 - 6;

   /// \cond CLASSIMP
   ClassDef(TPulseAnalyzer, 4)   // NOLINT
   /// \endcond
};
/*! @} */
#endif
