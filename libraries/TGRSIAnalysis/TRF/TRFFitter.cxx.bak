#include <iostream>
#include <vector>

#include <TDecompLU.h>
#include <TMath.h>
#include <TMatrixT.h>
#include <TH1.h>
#include <TF1.h>
#include <TVectorT.h>


#include "TFragment.h"
#include "TRFFitter.h"

using std::cout;
using std::endl;
using std::vector;

using TMath::Abs;
using TMath::ATan2;
using TMath::Power;
using TMath::QuietNaN;
using TMath::Sqrt;
using TMath::TwoPi;
using TMath::Pi;

ClassImp(TRFFitter)

static const Double_t defaultRfPeriod = 84.8e-9; // [seconds]
static const Double_t defaultSamplingPeriod = 10e-9; // [seconds]

TRFFitter::TRFFitter()  {
   Clear();
}

TRFFitter::~TRFFitter() {   }

void TRFFitter::Clear(Option_t *opt)  {
   //rf_omega = TwoPi() / (defaultRfPeriod / defaultSamplingPeriod);
   rf_omega = Pi() / (defaultRfPeriod / defaultSamplingPeriod);
   phase = QuietNaN();
   rftime = QuietNaN();
   midastime = 0;
   timestamp = 0;
   scale_factor = 1.0;
   signal_noise = 10.0;
   set = false;
}


void TRFFitter::FindPhase(const TFragment &frag, Bool_t calcErrors) {
  if(!frag.HasWave()) {
    printf("{TRFFitter} Error: fragment does not contain waveform.\n");
    return;
  }
  timestamp = frag.GetTimeStamp();
  midastime = frag.MidasTimeStamp;
  FindPhase(frag.wavebuffer,calcErrors);
}

void TRFFitter::DrawWave(const TFragment &frag) {
  if(!frag.HasWave()) {
    printf("{TRFFitter} Error: fragment does not contain waveform.\n");
    return;
  }
  if(!IsSet())
     FindPhase(frag,true);

  //g(t) = A sin (wt + phi) + c
  int bins = frag.wavebuffer.size();
  TF1 myfit("myfit","[0]*sin([1]*x+[2])+[3]",0,bins);
  myfit.SetParameter(0,amplitude);
  myfit.SetParameter(1,rf_omega);
  myfit.SetParameter(2,phase);
  myfit.SetParameter(3,offset);
  TH1I h("waveform",frag.GetName(),bins,0,bins-1);
  for(int x=0;x<bins;x++) 
    h.SetBinContent(x,frag.wavebuffer.at(x));
  Print();
  h.DrawCopy();
  myfit.DrawCopy("same");
  return;
}


void TRFFitter::FindPhase(const vector<TRFFitter::vector_element_t> &waveform, Bool_t calcErrors)  {
   if (waveform.empty()) {
      phase = rftime = QuietNaN();
      return;
   }
   CalculateDerivative(waveform);
   // CalculateDoubleDerivative(waveform);

   residuals.resize(waveform.size());
   scaledResiduals.resize(waveform.size());
   // Solve the chi-square equation for f(t) = a sin(wt) + b cos(wt) + c
   //   This is related to g(t) = A sin (wt + phi) + c
   //   via the transform: A = sqrt(a^2+b^2), phi = atan2(a,b)
   TMatrixD m(3,3);
   TMatrixD inv(3,3);

   TVectorD v(3);

   // populate the matrix and vector from the waveform data
   for (size_t i=0; i < waveform.size(); i++) {
      double s = sin(rf_omega * i);
      double c = cos(rf_omega * i);
      double w;
      if (scale_factor == 0) {
	     w = signal_noise;
      } else {
	     // double w = derivative[i]; // weight for this sample
	     w = Sqrt(Power(derivative[i] / scale_factor, 2) + Power(signal_noise,2)); 
      }

      // row for del(chi^2)/del a
      m(0,0) += s * s / w;
      m(0,1) += s * c / w;
      m(0,2) += s / w;
      // row for del(chi^2)/del b
      m(1,0) += s * c / w;
      m(1,1) += c * c / w;
      m(1,2) += c / w;
      // row for del(chi^2)/del c
      m(2,0) += s / w;
      m(2,1) += c / w;
      m(2,2) += 1 / w;
      // column data
      v(0) += waveform[i] * s / w;
      v(1) += waveform[i] * c / w;
      v(2) += waveform[i] / w;
   }

   TDecompLU lu(m);
   if (calcErrors) {
      // only do the full inverse if we want the errors
      lu.Invert(inv);

      v = inv * v;
   } else {
      lu.Solve(v);
   }
   // calculate the chi-square
   chisquare = 0;
   for (size_t i=0; i < waveform.size(); i++) {
      double s = sin(rf_omega * i);
      double c = cos(rf_omega * i);

      residuals[i] = waveform[i] - v(0) * s - v(1) * c - v(2);
      // scaledResiduals[i] = residuals[i] / derivative[i] * scale_factor;
      if (scale_factor == 0) {
	      scaledResiduals[i] = residuals[i] / signal_noise;
      } else {
	      scaledResiduals[i] = residuals[i] / Sqrt( Power(derivative[i] / scale_factor,2) + Power(signal_noise,2) ) * derivative[i] / Abs(derivative[i]);
      }
      /*
      if (Abs(doubleDerivative[i] / scale_factor) > Abs(derivative[i])) {
	 scaledResiduals[i] = residuals[i] / (0.5 * doubleDerivative[i] / scale_factor / scale_factor);
      } else {
	 scaledResiduals[i] = residuals[i] / (derivative[i] / scale_factor + 0.5 * doubleDerivative[i] / scale_factor / scale_factor);
      }
      */
      chisquare += Power(scaledResiduals[i], 2);
   }
   chisquare /= waveform.size() - 3;

   phase = ATan2(v(1),v(0));		
   amplitude = Sqrt(v(0)*v(0) + v(1)*v(1));
   offset = v(2);

   if (calcErrors) {
      error_phase     = Sqrt(v(1) * v(1) * inv(0,0) - 2 * v(0) * v(1) * inv(0,1) + v(0) * v(0) * inv(1,1)) / Power(amplitude, 2);
      error_amplitude = Sqrt(v(0) * v(0) * inv(0,0) + 2 * v(0) * v(1) * inv(0,1) + v(1) * v(1) * inv(1,1)) / amplitude;
      correlation     = (-v(0) * v(1) * inv(0,0) + (v(0) * v(0) - v(1) * v(1)) * inv(0,1) + v(0) * v(1) * inv(1,1)) / Power(amplitude, 3);
      correlation    /= error_phase * error_amplitude; // normalize
      error_offset    = Sqrt(inv(2,2));
      // inflate by chisquare
      error_phase     *= Sqrt(chisquare);
      error_amplitude *= Sqrt(chisquare);
      error_offset    *= Sqrt(chisquare);
   } else {
      error_phase     = 0;
      error_amplitude = 0;
      error_offset    = 0;
      correlation     = 0;
   }
   rftime = phase/rf_omega; //TwoPi()/(rf_omega - phase);
   set = true;
}

void TRFFitter::Print(Option_t *opt) const  {
   cout << "Fit with RF period = " << TwoPi() / rf_omega << endl;
   cout << "-----------------------------" << endl;
   cout << "  chisquare = " << chisquare << endl;
   cout << "  phi = " << phase << " +/- " << error_phase << endl;
   cout << "  amp = " << amplitude << " +/- " << error_amplitude << endl;
   cout << "  off = " << offset << " +/- " << error_offset << endl;
   cout << "  correlation coeff. = " << correlation << endl;
}

void TRFFitter::CalculateDerivative(const vector<TRFFitter::vector_element_t> &waveform)  {
   if (waveform.size() < 2) {
      return;
   }
   derivative.resize(waveform.size());
   if (waveform.size() == 2) {
      derivative[0] = waveform.at(1) - waveform.at(0);
      derivative[1] = -derivative[0];
      return;
   }
   // otherwise we calculate 3-point derivatives on the ends
   derivative[0] = -1.5 * waveform[0] + 2.0 * waveform[1] - 0.5 * waveform[2];
   derivative[1] = -0.5 * waveform[0] + 0.5 * waveform[2];
   derivative[waveform.size() - 2] = -0.5 * waveform[waveform.size() - 3] + 0.5 * waveform[waveform.size() - 1];
   derivative[waveform.size() - 1] = 0.5 * waveform[waveform.size() - 3] - 2.0 * waveform[waveform.size() - 2] + 1.5 * waveform[waveform.size() - 1];
   // and calculate 5-point derivatives in the middle
   for(size_t i=2; i < waveform.size() - 2; i++) {
      derivative[i] = 1.0 / 12.0 * (1 * waveform[i-2] - 8 * waveform[i-1] + 8 * waveform[i+1] - 1 * waveform[i+2]);
   }
}

void TRFFitter::CalculateDoubleDerivative(const vector<TRFFitter::vector_element_t> &waveform)  {
   if (waveform.size() < 3) {
      return;
   }
   doubleDerivative.resize(waveform.size());
   if (waveform.size() == 3) {
      doubleDerivative[0] = waveform.at(0) - 2 * waveform.at(1) + waveform.at(2);
      doubleDerivative[2] = doubleDerivative[1] = doubleDerivative[0];
      return;
   }
   // otherwise we calculate 3-point derivatives on the ends
   doubleDerivative[0] = waveform[0] - 2 * waveform[1] + waveform[2];
   doubleDerivative[1] = doubleDerivative[0];
   doubleDerivative[waveform.size() - 2] = waveform[waveform.size() - 3] - 2 * waveform[waveform.size() - 2] + waveform[waveform.size() - 1];
   doubleDerivative[waveform.size() - 1] = doubleDerivative[waveform.size() - 2];
   // and calculate 5-point double derivatives in the middle
   for(size_t i=2; i < waveform.size() - 2; i++) {
      doubleDerivative[i] = 1.0 / 3.0 * (waveform[i-2] - waveform[i-1] - waveform[i+1] + waveform[i+2]);
      // doubleDerivative[i] = 1.0 / 12.0 * (-waveform[i-2] + 16.0 * waveform[i-1] - 30.0 * waveform[i] + 16 * waveform[i+1] - waveform[i+2]);
   }
}
