
#include "GRootFunctions.h"


NamespaceImp(GRootFunctions)


#define PI TMATH::Pi()

Double_t GRootFunctions::PolyBg(Double_t *dim, Double_t *par, Int_t order) {
  Double_t result = 0.0;
  int j=0;
  for(Int_t i=0;i<=order;i++) {
    result += *(par+j) *TMath::Power(dim[0],i);
    j++;
  }
    //result += par[i]*TMath::Power(dim[0]-par[order+1],i);
  return result;
}

Double_t GRootFunctions::LinFit(Double_t *dim, Double_t *par) {
  return PolyBg(dim,par,1);
}

Double_t GRootFunctions::QuadFit(Double_t *dim, Double_t *par) {
  return PolyBg(dim,par,2);
}


Double_t GRootFunctions::StepFunction(Double_t *dim, Double_t *par) {
  //  -dim[0]: channels to fit
  //  -par[0]: height of peak
  //  -par[1]: centroid of peak
  //  -par[2]: sigma of peak
  //  -par[3]: size of step in step function.

  Double_t x       = dim[0];
  
  Double_t height  = par[0];
  Double_t cent    = par[1];
  Double_t sigma   = par[2];
  //Double_t R       = par[4];
  Double_t step    = par[3];

  //return TMath::Abs(step)*height/100.0*TMath::Erfc((x-cent)/(TMath::Sqrt(2.0)*sigma));
  return height*(step/100.0) *TMath::Erfc((x-cent)/(TMath::Sqrt(2.0)*sigma));

}

Double_t GRootFunctions::StepBG(Double_t *dim, Double_t *par) {
  return StepFunction(dim,par) + PolyBg(dim,(par+4),0);
}

Double_t GRootFunctions::Gaus(Double_t *dim, Double_t *par) {
  // - dim[0]: channels to fit
  // - par[0]: height of peak
  // - par[1]: cent of peak
  // - par[2]: sigma
  // - par[3]: relative height of skewed gaus to gaus

  Double_t x      = dim[0];
  Double_t height = par[0];
  Double_t cent   = par[1];
  Double_t sigma  = par[2];
  Double_t R      = par[3];

  return height*(1.0-R/100.0)*TMath::Gaus(x,cent,sigma);
}

Double_t GRootFunctions::SkewedGaus(Double_t *dim,Double_t *par) {

  // StepFunction(dim,par) + PolyBg
  // - par[0]: height of peak
  // - par[1]: cent of peak
  // - par[2]: sigma
  // - par[3]: relative height of skewed gaus to gaus
  // - par[4]: "skewedness" of the skewed gaussin

  Double_t x      = dim[0]; //channel number used for fitting
  Double_t height = par[0]; //height of photopeak
  Double_t cent   = par[1]; //Peak Centroid of non skew gaus
  Double_t sigma  = par[2]; //standard deviation of gaussian
  Double_t R      = par[3]; //relative height of skewed gaussian
  Double_t beta   = par[4]; //"skewedness" of the skewed gaussian

  double scaling = R*height/100.0;
  //double x_rel = (x - cent)/sigma;

  double fterm = (x-cent)/(sigma*TMath::Sqrt(2.));
  double sterm = sigma /  (beta *TMath::Sqrt(2.));

  return scaling * TMath::Exp((x-cent)/beta) * TMath::Erfc(fterm + sterm); 
}

Double_t GRootFunctions::PhotoPeak(Double_t *dim,Double_t *par) {
  return Gaus(dim,par) + SkewedGaus(dim,par);
}

Double_t GRootFunctions::PhotoPeakBG(Double_t *dim,Double_t *par) {
  // - dim[0]: channels to fit
  // - par[0]: height of peak
  // - par[1]: cent of peak
  // - par[2]: sigma
  // - par[3]: relative height of skewed gaus to gaus
  // - par[4]: "skewedness" of the skewed gaussin
  // - par[5]: size of stepfunction step.

  // - par[6]: base bg height.
  // - par[7]: slope of bg.
  
  double spar[4];
  spar[0] = par[0];
  spar[1] = par[1];
  spar[2] = par[2];
  spar[3] = par[5];  //stepsize;
  return Gaus(dim,par) + SkewedGaus(dim,par) + StepFunction(dim,spar) + PolyBg(dim,par+6,0);
}


// For fitting Ge detector efficiencies.
Double_t GRootFunctions::Efficiency(Double_t *dim, Double_t *par){
  // - dim[0]: energy.
  // - par[0]: zeroth order
  // - par[1]: first order
  // - par[2]: second order
  // - par[3]: inverse energy squared term.
  // - Formula : 10**(0+1*Log(x)+2*Log(x)**2+3/x**2)

  Double_t x  = dim[0];
  Double_t p0 = par[0];
  Double_t p1 = par[1];
  Double_t p2 = par[2];
  Double_t p3 = par[3];

  if(x!=0)
    return pow(10.0,(p0+p1*TMath::Log10(x)+p2*std::pow(TMath::Log10(x),2.0)+p3/(std::pow(x,2.0))));
  else
    return 0;

}


Double_t GRootFunctions::GausExpo(Double_t *x, Double_t *pars) {

  double result;

  // gaus + step*expo conv with a gaus.

  // par[0] = height
  // par[1] = cent
  // par[2] = sigma
  // par[3] = decay parameter

  result = TMath::Gaus(pars[0],pars[1],pars[2])+(double)(x[0]>pars[1])*pars[0]*TMath::Exp(-pars[3]); 
  return result;
}





Double_t GRootFunctions::LanGaus(Double_t *x, Double_t *pars){
   double dy, y, conv, spec, gaus;
   conv = 0;

   for(int i=0; i<10; i++){
    dy = 5*pars[2]/10.0; // truncate the convolution by decreasing number of evaluation points and decreasing range [2.5 sigma still covers 98.8% of gaussian]
    y = x[0]-2.5*pars[2]+dy*i;
    spec = pars[0]+pars[1]*y; // define background SHOULD THIS BE CONVOLUTED ????? *************************************
    //for( int n=0; n<(int)(pars[0]+0.5); n++) // the implementation of landau function should be done using the landau function
      spec +=pars[3]*TMath::Landau(-y,-pars[4],pars[5])/TMath::Landau(0,0,100); // add peaks, dividing by max height of landau
    gaus = TMath::Gaus(-x[0],-y,pars[2])/sqrt(2*TMath::Pi()*pars[2]*pars[2]); // gaus must be normalisd so there is no sigma weighting
    conv += gaus*spec*dy; // now convolve this [integrate the product] with a gaussian centered at x;
  }

  return conv;
}


Double_t GRootFunctions::LanGausHighRes(Double_t *x, Double_t *pars){ // 5x more convolution points with 1.6x larger range
  double dy, y, conv, spec, gaus;
  conv = 0;

  for(int i=0; i<50; i++){
    dy = 8*pars[2]/50.0; // 4 sigma covers 99.99% of gaussian
    y  = x[0]-4*pars[2]+dy*i;

    spec = pars[0]+pars[1]*y;
    //for( int n=0; n<(int)(pars[0]+0.5); n++)
    spec +=pars[3]*TMath::Landau(-y,-pars[4],pars[5])/TMath::Landau(0,0,100);

    gaus = TMath::Gaus(-x[0],-y,pars[2])/sqrt(2*TMath::Pi()*pars[2]*pars[2]);
    conv += gaus*spec*dy;
  }
  return conv;
}


Double_t GRootFunctions::GammaEff(Double_t *x,Double_t *par) {
  // LOG(EFF) = A0 + A1*LOG(E) + A2*LOG(E)^2 + A3/E^2 

  double logE = TMath::Log10(x[0]);
  double temp =  par[0] + par[1]*logE + par[2]*logE*logE +par[3]/(x[0]*x[0]);
  return pow(10,temp);

}


















