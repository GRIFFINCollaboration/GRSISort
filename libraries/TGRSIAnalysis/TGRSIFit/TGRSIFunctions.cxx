#include "TGRSIFunctions.h"

// Without this macro the THtml doc for TGRSIFunctions can't be generated
NamespaceImp(TGRSIFunctions)

//////////////////////////////////////////////////////////////////////
//
// TGRSIFunctions
//
// This namespace is where we store all of our commonly used functions.
// This makes it easier to create fits etc.
//
///////////////////////////////////////////////////////////////////////

Double_t TGRSIFunctions::CsIFitFunction(Double_t* i, Double_t* p)
{
   Double_t x, s, e;

   /*
      p[0]-p[4] are t0, tRC, tF, TS, TGamma
      p[5]-p[8] are baseline, AF, AS, AGamma
      */

   x = i[0] - p[0];
   e = exp(-x / p[1]);
   if(x <= 0) {
      return p[5];
   }
   s = p[5];
   s += p[6] * (1 - exp(-x / p[2])) * e;
   s += p[7] * (1 - exp(-x / p[3])) * e;
   s += p[8] * (1 - exp(-x / p[4])) * e;
   return s;
}

Double_t TGRSIFunctions::PolyBg(Double_t* x, Double_t* par, Int_t order)
{
   // Polynomial function of the form SUM(par[i]*(x - shift)^i). The shift is done to match parameters with Radware
   // output.

   /*   if((Double_t)(sizeof(par))/(Double_t)(sizeof(par[0])) < (order + 2)){ //This doesn't work with the current
      method
        std::cout<<"not enough parameters passed to function"<<std::endl;
        std::cout<<"sizeof par = "<<sizeof(par)<<std::endl;
        std::cout<<"size of par[0] = "<<sizeof(par[0])<<std::endl;
        return 0;
        }   */

   Double_t result = 0.0;
   for(Int_t i = 0; i <= order; i++) {
      result += par[i] * TMath::Power(x[0] - par[order + 1], i);
   }
   return result;
}

Double_t TGRSIFunctions::StepFunction(Double_t* dim, Double_t* par)
{
   // This function uses the same parameters as the photopeak and gaussian. This is because in the photopeak,
   // the shapes are correlated.
   // Requires the following parameters:
   //   - dim[0]:  channels being fit
   //   - par[0]:  height of photopeak
   //   - par[1]:  centroid of gaussian
   //   - par[2]:  standard deviation of gaussian
   //   - par[5]:  Size of the step in the step function

   Double_t x      = dim[0];
   Double_t height = par[0];
   Double_t c      = par[1];
   Double_t sigma  = par[2];
   Double_t step   = par[5];

   return TMath::Abs(step) * height / 100.0 * TMath::Erfc((x - c) / (TMath::Sqrt(2.0) * sigma));
}

Double_t TGRSIFunctions::StepBG(Double_t* dim, Double_t* par)
{
   return StepFunction(dim, par) + PolyBg(dim, &par[6], 2);
}

Double_t TGRSIFunctions::PhotoPeak(Double_t* dim, Double_t* par)
{
   // Returns the combination of a TGRSIFunctions::Gaus + a TGRSIFunctions::SkewedGaus
   return Gaus(dim, par) + SkewedGaus(dim, par);
}

Double_t TGRSIFunctions::PhotoPeakBG(Double_t* dim, Double_t* par)
{
   // Returns a single RadWare style peak
   return Gaus(dim, par) + SkewedGaus(dim, par) + StepFunction(dim, par) + PolyBg(dim, &par[6], 2);
}

Double_t TGRSIFunctions::MultiPhotoPeakBG(Double_t* dim, Double_t* par)
{
   //
   // Limits need to be imposed or error states may occour.
   //
   // General background.
   int    nPeaks = static_cast<int>(par[0] + 0.5);
   double result = PolyBg(dim, &par[1], 2); // polynomial background. uses par[1->4]
   for(int i = 0; i < nPeaks; i++) {        // par[0] is number of peaks
      Double_t tmp_par[6];
      tmp_par[0] = par[6 * i + 5];  // height of photopeak
      tmp_par[1] = par[6 * i + 6];  // Peak Centroid of non skew gaus
      tmp_par[2] = par[6 * i + 7];  // standard deviation  of gaussian
      tmp_par[3] = par[6 * i + 8];  //"skewedness" of the skewed gaussian
      tmp_par[4] = par[6 * i + 9];  // relative height of skewed gaussian
      tmp_par[5] = par[6 * i + 10]; // Size of step in background
      result += PhotoPeak(dim, tmp_par) + StepFunction(dim, tmp_par);
   }
   return result;
}

Double_t TGRSIFunctions::Gaus(Double_t* dim, Double_t* par)
{
   // This is a gaussian that has been scaled to match up with Radware photopeak results.
   // It contains a scaling factor for the relative height of the skewed gaussian to the
   // normal gaussian. Requires the following parameters:
   //   - dim[0]:  channels being fit
   //   - par[0]:  height of photopeak
   //   - par[1]:  centroid of gaussian
   //   - par[2]:  standard deviation of gaussian
   //   - par[4]:  relative height of skewed gaus to gaus

   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma  = par[2]; // standard deviation of gaussian
   Double_t R      = par[4]; // relative height of skewed gaussian

   return height * (1.0 - R / 100.0) * TMath::Gaus(x, c, sigma);
}

Double_t TGRSIFunctions::SkewedGaus(Double_t* dim, Double_t* par)
{
   // This function uses the same parameters as the photopeak and gaussian. This is because in the photopeak,
   // the shapes are correlated.
   // Requires the following parameters:
   //   - dim[0]:  channels being fit
   //   - par[0]:  height of photopeak
   //   - par[1]:  centroid of gaussian
   //   - par[2]:  standard deviation of gaussian
   //   - par[3]:  "skewedness" of the skewed gaussin
   //   - par[4]:  relative height of skewed gaus to gaus

   Double_t x      = dim[0]; // channel number used for fitting
   Double_t height = par[0]; // height of photopeak
   Double_t c      = par[1]; // Peak Centroid of non skew gaus
   Double_t sigma  = par[2]; // standard deviation  of gaussian
   Double_t beta   = par[3]; //"skewedness" of the skewed gaussian
   Double_t R      = par[4]; // relative height of skewed gaussian
   if(beta == 0.0) {
      return 0.0;
   }

   return R * height / 100.0 * (TMath::Exp((x - c) / beta)) *
          (TMath::Erfc(((x - c) / (TMath::Sqrt(2.0) * sigma)) + sigma / (TMath::Sqrt(2.0) * beta)));
}
Double_t TGRSIFunctions::MultiSkewedGausWithBG(Double_t* dim, Double_t* par)
{
   // TGRSIFunctions::Set(int num);
   //
   // Limits need to be imposed or error states may occour.
   //
   double result = par[1] + dim[0] * par[2]; // background.
   for(int i = 0; i < par[0]; i++) {         // par[0] is number of peaks
      Double_t tmp_par[5];
      tmp_par[0] = par[5 * i + 3]; // height of photopeak
      tmp_par[1] = par[5 * i + 4]; // Peak Centroid of non skew gaus
      tmp_par[2] = par[5 * i + 5]; // standard deviation  of gaussian
      tmp_par[3] = par[5 * i + 6]; //"skewedness" of the skewed gaussian
      tmp_par[4] = par[5 * i + 7]; // relative height of skewed gaussian
      result += SkewedGaus(dim, tmp_par);
   }
   return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Double_t TGRSIFunctions::SkewedGaus2(Double_t* x, Double_t* par)
{
   // This function is derived from the convolution of a gaussian with an exponential
   // Requires the following parameters:
   //   - par[0]:  height of photopeak
   //   - par[1]:  centroid of gaussian
   //   - par[2]:  standard deviation of gaussian
   //   - par[3]:  "skewedness" of the skewed gaussin

   return par[0] * (TMath::Exp((x[0] - par[1]) / par[3])) *
          (TMath::Erfc(((x[0] - par[1]) / (TMath::Sqrt(2.0) * par[2])) + par[2] / (TMath::Sqrt(2.0) * par[3])));
}

Double_t TGRSIFunctions::MultiSkewedGausWithBG2(Double_t* dim, Double_t* par)
{
   // TGRSIFunctions::Set(int num);
   //
   // Limits need to be impossed or error states may occour.
   //
   double result = par[1] + dim[0] * par[2];                 // background.
   for(int i = 0; i < static_cast<int>(par[0] + 0.5); i++) { // par[0] is number of peaks
      Double_t tmp_par[4];
      tmp_par[0] = par[4 * i + 3]; // height of photopeak
      tmp_par[1] = par[4 * i + 4]; // Peak Centroid of non skew gaus
      tmp_par[2] = par[4 * i + 5]; // standard deviation  of gaussian
      tmp_par[3] = par[4 * i + 6]; //"skewedness" of the skewed gaussian //
                                   //&$^@(T&U@(FUJIQ#HF@Q#YG)*!*$()#%*$@)%(*#$%()*#)(*%#$()*%(@$)#%*)#%*$$#
      result += SkewedGaus2(dim, tmp_par);
   }
   return result;
}

Double_t TGRSIFunctions::LanGaus(Double_t* x, Double_t* pars)
{
   double dy, y, conv, spec, gaus;
   conv = 0;

   for(int i = 0; i < 10; i++) {
      dy = 5 * pars[3] / 10.0; // truncate the convolution by decreasing number of evaluation points and decreasing
                               // range [2.5 sigma still covers 98.8% of gaussian]
      y = x[0] - 2.5 * pars[3] + dy * i;

      spec = pars[1] +
             pars[2] * y; // define background SHOULD THIS BE CONVOLUTED ????? *************************************
      for(int n = 0; n < static_cast<int>(pars[0] + 0.5);
          n++) { // the implementation of landau function should be done using the landau function
         spec += pars[3 * n + 4] * TMath::Landau(-y, -pars[3 * n + 5], pars[3 * n + 6]) /
                 TMath::Landau(0, 0, 100); // add peaks, dividing by max height of landau
      }

      gaus = TMath::Gaus(-x[0], -y, pars[3]) /
             sqrt(2 * TMath::Pi() * pars[3] * pars[3]); // gaus must be normalisd so there is no sigma weighting
      conv += gaus * spec * dy; // now convolve this [integrate the product] with a gaussian centered at x;
   }

   return conv;
}

Double_t TGRSIFunctions::LanGausHighRes(Double_t* x, Double_t* pars)
{ // 5x more convolution points with 1.6x larger range
   double dy, y, conv, spec, gaus;
   conv = 0;

   for(int i = 0; i < 50; i++) {
      dy = 8 * pars[3] / 50.0; // 4 sigma covers 99.99% of gaussian
      y  = x[0] - 4 * pars[3] + dy * i;

      spec = pars[1] + pars[2] * y;
      for(int n = 0; n < static_cast<int>(pars[0] + 0.5); n++) {
         spec += pars[3 * n + 4] * TMath::Landau(-y, -pars[3 * n + 5], pars[3 * n + 6]) / TMath::Landau(0, 0, 100);
      }

      gaus = TMath::Gaus(-x[0], -y, pars[3]) / sqrt(2 * TMath::Pi() * pars[3] * pars[3]);
      conv += gaus * spec * dy;
   }
   return conv;
}

Double_t TGRSIFunctions::MultiGausWithBG(Double_t* dim, Double_t* par)
{
   // TGRSIFunctions::Set(int num);
   //
   // Limits need to be impossed or error states may occour.
   //
   double amp, mean, sigma;
   double result = par[1] + dim[0] * par[2]; // background.
   for(int i = 0; i < static_cast<int>(par[0] + 0.5); i++) {
      amp   = par[3 * i + 3];
      mean  = par[3 * i + 4];
      sigma = par[3 * i + 5];
      result += amp * TMath::Gaus(dim[0], mean, sigma);
   }
   return result;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Double_t TGRSIFunctions::Bateman(Double_t* dim, Double_t* par, UInt_t nChain, Double_t)
{
   //****NOT TESTED****The Bateman equation is the general closed form for a decay chain of nuclei. This functions
   // returns
   // the total activity from a given chain of nuclei.
   // Requires the following parameters:
   //   - dim[0]:  channels being fit
   //   - par[0+3*i]:  Initial Activity s^(-1)
   //   - par[1+3*i]:  Decay Rate in seconds^(-1)
   //   - par[2+3*i]:  Activity at time t s^(-1)
   // NOTE: The lowest paramters correspond to the most 'senior' nuclei

   if(sizeof(par) / sizeof(par[0]) < (nChain * 3)) {
      std::cout<<"not enough parameters passed to function"<<std::endl;
      return 0;
   }

   Double_t totalActivity = 0.0;

   // LOOP OVER ALL NUCLEI

   for(UInt_t n = 0; n < nChain; n++) {
      // Calculate this equation for the nth nucleus.
      Double_t firstterm = 1.0;
      // Compute the first multiplication
      for(UInt_t j = 0; j < n - 1; j++) {
         firstterm *= par[1 + 3 * j];
      }
      Double_t secondterm = 0.0;
      for(UInt_t i = 0; i < n; i++) {
         Double_t sum = 0.0;
         for(UInt_t j = i; j < n; j++) {
            Double_t denom = 1.0;
            for(UInt_t p = i; p < n; p++) {
               if(p != j) {
                  denom *= par[1 + 3 * p] - par[1 + 3 * j];
               }
            }
            sum += par[0 + 3 * i] / par[1 + 3 * i] * TMath::Exp(-par[1 + 3 * j] * dim[0]) / denom;
         }
         secondterm += sum;
      }
      par[2 + 3 * n] = par[1 + 3 * n] * firstterm * secondterm;
      totalActivity += par[2 + 3 * n];
   }
   return totalActivity;
}

Double_t TGRSIFunctions::DeadTimeCorrect(Double_t* dim, Double_t deadtime, Double_t binWidth)
{
   // This function deadtime corrects data. Not to be confused with dead time affecting of fit functions
   // Dead time is in us.
   // binWidth is in s/bin.

   return dim[0] / (1.0 - dim[0] * deadtime / (binWidth * 1000000.0));
}

Double_t TGRSIFunctions::DeadTimeAffect(Double_t function, Double_t deadtime, Double_t binWidth)
{
   // This function deadtime affects fitting functions. This is useful for counting the number of decays.
   // Dead time is in us.
   // binWidth is in s/bin.

   return function / (1.0 + function * deadtime / (binWidth * 1000000.0));
}

#ifdef HAS_MATHMORE
Double_t TGRSIFunctions::LegendrePolynomial(Double_t* x, Double_t* p)
{
   Double_t val;
   val = p[0] * (1 + p[1] * ::ROOT::Math::legendre(2, x[0]) + p[2] * ::ROOT::Math::legendre(4, x[0]));
   return val;
}
#endif

Double_t TGRSIFunctions::PhotoEfficiency(Double_t* dim, Double_t* par)
{

   double sum = 0.0;
   for(int i = 0; i < 9; ++i) {
      sum += par[i] * TMath::Power(TMath::Log(dim[0]), i);
   }

   return TMath::Exp(sum);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Double_t TGRSIFunctions::ConvolutedDecay(Double_t *x, Double_t *par)
{
///This function is derived from the convolution of a gaussian with an exponential decay, to fit TAC spectra of long half-lives (above 100 ps)
///Requires the following parameters:
///   - par[0]:  Normalization factor
///   - par[1]:  Centroid of gaussian
///   - par[2]:  Sigma of gaussian 
///   - par[3]:  Lambda of the level

  Double_t val;
  val = TMath::Sqrt(TMath::Pi())*par[0]*par[3]/2.*TMath::Exp(par[3]/2.*(2.*par[1]+par[3]*pow(par[2],2.)-2.*x[0]))*TMath::Erfc((par[1]+par[3]*pow(par[2],2.)-x[0])/(TMath::Sqrt(2)*par[2]));
  return val;
}
