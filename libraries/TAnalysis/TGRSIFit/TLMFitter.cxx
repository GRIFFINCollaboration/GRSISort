#include "TLMFitter.h"
#include <iomanip>
#include <iostream>

/* DEFINITIONS */
/*******************************************************************/
/* alamda = parameter switches between curve and gradient method   */
/*          must begin negative to initialize fit routine          */
/* x[DIM] = x values of the data                                   */
/* y[DIM] = y values of the data (Number of counts)                */
/* sig[i] = sigma in the y value                                   */
/* chisq  = chi-squared definitions defined in mrqcof subroutine   */
/* sig2i  = 1 over sigma squared (Used in computing the chisq)     */
/* a[i]   = fit parameters of the fitting function                 */
/* ia[i]  = set to '1' to free a[i] or '0' for fixed               */
/* beta[j]= extremum vector                                        */
/* alpha[k][k] = curvature matrix                                  */
/* dy     = yfit - ydata                                           */
/* ma     = number of fit parameters in fit function               */
/*******************************************************************/

/* Function Subroutine-***Put fitting function here***-------------*/
void TLMFitter::funcs(const double& x, Vec_IO_double& a, double& y, Vec_O_double& dyda)
{
   for(int i = 0; i < a.size(); ++i) {
      fFunction->SetParameter(i, a[i]);
   }
   for(int i = 0; i < a.size(); ++i) {
      dyda[i] = fFunction->GradientPar(i, &x);
   }
   y = fFunction->Eval(x);
   // std::cout<<"y: "<<y<<" x: "<<x<<std::endl;
}

void TLMFitter::Fit(TH1* hist, TF1* func)
{
   double alamda = 0.;
   double chisq  = 0.;
   double ochisq = 0.;

   int          ma = func->GetNpar(); /* This is the number of parameters in fit function */
   Mat_O_double alpha(ma, ma);
   Mat_O_double covar(ma, ma);
   Vec_BOOL     ia(ma);
   Vec_O_double a(ma);
   Vec_O_double dyda(ma);

   fFunction = func;
   fHist     = hist;

   // Sets whether parameters are fixed or free.
   std::cout << "Setting the parameters..." << std::endl;
   for(int i = 0; i < func->GetNpar(); ++i) {
      a[i] = func->GetParameter(i);

      Double_t min = 0.;
      Double_t max = 0.;
      func->GetParLimits(i, min, max);
      if(min == max) {
         if(min != 0) {
            // Parameter is fixed
            ia[i] = false;
            continue;
         }
      }
      ia[i] = true;
   }
   for(int i = 0; i < ma; ++i) {
      std::cout << "par[" << i << "] = " << fFunction->GetParameter(i) << std::endl;
   }
   double func_range_min = 0.;
   double func_range_max = 0.;
   func->GetRange(func_range_min, func_range_max);
   std::cout << "function range: " << func_range_min << " to " << func_range_max << std::endl;
   SetFitterRange(static_cast<int>(func_range_min), static_cast<int>(func_range_max)); // not sure why SetFitterRange takes int as arguments, is it expecting bins as input?
   int bin_min = hist->FindBin(func_range_min);
   int bin_max = hist->FindBin(func_range_max);

   int        nBins = hist->GetNbinsX();
   Vec_double x(nBins);
   Vec_double y(nBins);
   Vec_double sig(nBins);
   Vec_double yfit(nBins);
   Vec_double W(nBins);
   Vec_double v(nBins);

   std::cout << "Setting bin values..." << std::endl;
   std::cout << "Range is " << bin_min << " to " << bin_max << std::endl;
   for(int i = 0; i < hist->GetNbinsX(); ++i) {
      x[i] = hist->GetXaxis()->GetBinUpEdge(i + 1) * hist->GetXaxis()->GetBinWidth(1);
      y[i] = hist->GetBinContent(i + 1);
      v[i] = hist->GetBinError(i + 1) * hist->GetBinError(i + 1);
      if(v[i] == 0) {
         W[i] = 1.0;
      } else {
         W[i] = y[i] / v[i];
      }
   }

   alamda = -1;
   mrqmin(x, y, sig, a, ia, covar, alpha, chisq, W, alamda);
   std::cout << "out of mrqmin" << std::endl;
   int k    = 1;
   int itst = 0;
   while(true) {
      std::cout << std::endl
                << "Iteration #" << std::setw(3) << k;
      std::cout << std::setw(18) << "chi-squared:" << std::setw(13) << chisq;
      std::cout << std::setw(11) << "alamda:" << std::setw(10) << alamda << std::endl;
      for(int i = 0; i < ma; ++i) {
         std::cout << "par[" << i << "] = " << fFunction->GetParameter(i) << std::endl;
      }
      std::cout << std::endl;
      ++k;
      ochisq = chisq;
      mrqmin(x, y, sig, a, ia, covar, alpha, chisq, W, alamda);
      std::fabs(ochisq - chisq) < 0.0001 ? itst++ : itst = 0;
      if(itst < 10) {
         continue;
      }
      alamda = 0.0;
      mrqmin(x, y, sig, a, ia, covar, alpha, chisq, W, alamda);
      std::cout << "Uncertainties:" << std::endl;
      for(int i = 0; i < ma; ++i) {
         std::cout << " " << std::sqrt(covar[i][i]);
      }
      std::cout << std::endl;
      break;
   }
   std::cout << "Chis2/ndf: " << chisq << std::endl;

   // Feed the parameters back into the function.
   for(int i = 0; i < ma; ++i) {
      fFunction->SetParameter(i, a[i]);
      fFunction->SetParError(i, std::sqrt(covar[i][i]));
   }
}
/*******************************************************************/
/*                                                                 */
/* Subroutines below (in order) -                                  */
/* integrator - integration routine (fit function is an integral)/  */
/* mrqmin - subroutine, which rejects or accepts fit parameters    */
/* mrqcof - calculates the chi squared values                      */
/* covsrt - converts alpha matrix into the error matrix            */
/* gaussj - row reduces alpha and beta to send to covsrt           */
/*                                                                 */
/*******************************************************************/
/*******************************************************************/
/*  Integrator                                                     */
/*******************************************************************/
int TLMFitter::integrator(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_I_double&, Vec_IO_double& a,
                          Vec_double& dyda, int chisqnumber, const double&, Vec_double& yfit, const int& bin)
{
   // Integrates the function within each bin
   int ma = a.size();
   //	int i, j, k;
   double     ynew   = 0;
   double     xstart = x[bin];   // This is the start of the bin
   double     ymod   = 0;
   Vec_double z(fIntegrationSteps);
   Vec_double temp(dyda.size());

   for(int k = 0; k < fIntegrationSteps; ++k) {
      // Find the y value at different integration stpes along the bin
      // z[k] = xstart + (double)(k)*bin_width/(double)fIntegrationSteps;
      z[k] = xstart + static_cast<double>(k) / static_cast<double>(fIntegrationSteps);
      funcs(z[k], a, ymod, dyda);
      for(int i = 0; i < ma; ++i) {
         // Integrates the gradient of the functon within the bin
         temp[i] += 1. / static_cast<double>(fIntegrationSteps) * dyda[i];
         // temp[i] += bin_width/(double)fIntegrationSteps*dyda[i];
      }
      // integrates the y of the function
      // ynew += bin_width/(double)fIntegrationSteps*ymod;
      ynew += 1. / static_cast<double>(fIntegrationSteps) * ymod;
   }
   // Sets the actual variables to the "temporary" variables used for integration
   for(int i = 0; i < ma; ++i) {
      dyda[i] = temp[i];
   }
   yfit[bin] = ynew;

   //	//Change the chisqnumber from what it was defined at the beginning (most likely 3 = Poisson distribution)
   //	//to 2 (= Gaussian distribution with possible bin counts of zero) if yfit < 0.
   if(yfit[bin] <= 0) {
      chisqnumber = 2;
   } else {
      chisqnumber = fInitChi2Number;
   }

   /* Various definitions of sigma used in diff chi squares */
   if(chisqnumber == 0 || chisqnumber == 3) {
      sig[bin] = sqrt(yfit[bin]);
   }
   if(chisqnumber == 1 || chisqnumber == 2) {
      sig[bin] = sqrt(y[bin]);
      if(chisqnumber == 1) {
         if(y[bin] <= 0.5) {
            sig[bin] = sqrt(yfit[bin]);
         }
      }
      if(chisqnumber == 2) {
         if(y[bin] <= 0.5) {
            sig[bin] = 1.0;
         }
      }
   }
   return chisqnumber;
}
///*******************************************************************/
/*  mrqmin                                                         */
/*******************************************************************/

void TLMFitter::mrqmin(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_IO_double& a, Vec_I_BOOL& ia,
                       Mat_O_double& covar, Mat_O_double& alpha, double& chisq, Vec_I_double& W, double& alamda)
{
   static int    mfit;
   static double ochisq;
   static double chisqexp;

   int ma = a.size();
   if(ma <= 0) {
      std::cerr << __PRETTY_FUNCTION__ << ": vector a is empty, not doing anything!" << std::endl;   // NOLINT(cppcoreguidelines-pro-bounds-array-to-pointer-decay)
      return;
   }
   static Mat_double* oneda_p;
   static Vec_double* atry_p;
   static Vec_double* beta_p;
   static Vec_double* da_p;
   if(alamda < 0.0) {   // Initialization
      atry_p = new Vec_double(ma);
      beta_p = new Vec_double(ma);
      da_p   = new Vec_double(ma);
      mfit   = 0;
      for(int j = 0; j < ma; j++) {
         if(ia[j]) {
            mfit++;
         }
         oneda_p = new Mat_double(mfit, 1);
         alamda  = 0.001;
         mrqcof(x, y, sig, a, ia, alpha, *beta_p, chisq, W, chisqexp);
         ochisq = chisq;
         for(j = 0; j < ma; j++) {
            (*atry_p)[j] = a[j];
         }
      }
   }
   Mat_double& oneda = *oneda_p;
   Vec_double& atry  = *atry_p;
   Vec_double& beta  = *beta_p;
   Vec_double& da    = *da_p;
   Mat_double  temp(mfit, mfit);
   // After linearized fitting matrix, by augmenting diagonal elements
   for(int j = 0; j < mfit; j++) {
      for(int k = 0; k < mfit; k++) {
         covar[j][k] = alpha[j][k];
      }
      covar[j][j] = alpha[j][j] * (1.0 + alamda);
      for(int k = 0; k < mfit; k++) {
         temp[j][k] = covar[j][k];
      }
      oneda[j][0] = beta[j];
   }
   gaussj(temp, oneda);   // Matrix solution
   for(int j = 0; j < mfit; j++) {
      for(int k = 0; k < mfit; k++) {
         covar[j][k] = temp[j][k];
      }
      da[j] = oneda[j][0];
   }
   if(alamda == 0.0) {   // Once converged, evaluate covariance matrix
      covsrt(covar, ia, mfit);
      covsrt(alpha, ia, mfit);   // Spread out alpha to its full size too
      chisqexp -= mfit;
      chisq /= chisqexp;
      delete oneda_p;
      delete da_p;
      delete beta_p;
      delete atry_p;
      return;
   }
   for(int j = 0, l = 0; l < ma; l++) {   // Did the trial succeed?
      if(ia[l]) {
         atry[l] = a[l] + da[j++];
      }
   }
   mrqcof(x, y, sig, atry, ia, covar, da, chisq, W, chisqexp);
   if(chisq < ochisq) {   // Success, accept the new solution
      alamda *= 0.1;
      ochisq = chisq;
      for(int j = 0; j < mfit; j++) {
         for(int k = 0; k < mfit; k++) {
            alpha[j][k] = covar[j][k];
         }
         beta[j] = da[j];
      }
      for(int l = 0; l < ma; l++) {
         a[l] = atry[l];
      }
   } else {   // Failure, increase alamda and return
      alamda *= 10.0;
      chisq = ochisq;
   }
}

/*******************************************************************/
/*  mrqcof                                                         */
/*******************************************************************/

void TLMFitter::mrqcof(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_IO_double& a, Vec_I_BOOL& ia,
                       Mat_O_double& alpha, Vec_O_double& beta, double& chisq, Vec_I_double& W, double& chisqexp)
{
   chisqexp = 0.0;

   int        ndata = x.size();
   int        ma    = a.size();
   int        mfit  = 0;
   Vec_double dyda(ma);
   Vec_double yfit(ndata);
   for(int j = 0; j < ma; j++) {
      if(ia[j]) {
         mfit++;
      }
   }
   for(int j = 0; j < mfit; j++) {   // Initialize (symmetric) alpha, beta.
      for(int k = 0; k <= j; k++) {
         alpha[j][k] = 0.0;
      }
      beta[j] = 0.0;
   }
   chisq = 0.0;
   for(int i = fRangeMin; i < fRangeMax; ++i) {   // Summation loop over all data.
      int chisqnumber = integrator(x, y, sig, W, a, dyda, fInitChi2Number, fHist->GetXaxis()->GetBinWidth(1), yfit, i);
      // funcs(x[i],a,ymod,dyda); Integrator does this instead
      double sig2i = 1.0 / (sig[i] * sig[i]);
      double dy    = y[i] - yfit[i];
      for(int j = 0, l = 0; l < ma; l++) {
         if(ia[l]) {
            // sigs are different for different chisqnumbers
            // This is done in integrator
            double wt = dyda[l] * sig2i;
            for(int k = 0, m = 0; m < l + 1; m++) {
               if(ia[m]) {
                  if(chisqnumber == 0) {   // least squares
                     alpha[j][k++] += wt * dyda[m] * (1.0 + dy / yfit[i]) * (1.0 + dy / yfit[i]) * W[i];
                  } else {
                     alpha[j][k++] += wt * dyda[m] * W[i];
                  }
               }
            }
            if(chisqnumber == 0) {   // Least squares
               beta[j++] += dy * wt * (1.0 + dy / (2.0 * yfit[i])) * W[i];
            } else {
               beta[j++] += dy * wt * W[i];
            }
         }
         // Now find the correct chi^2 based on the different versions of chisquared
         if((chisqnumber == 0) || (chisqnumber == 1) || (chisqnumber == 2)) {
            chisq += dy * dy * sig2i * W[i];
            chisqexp += 1;
         }
         if(chisqnumber == 3) {
            if(y[i] < 1.0) {
               chisq += 2.0 * (yfit[i] - y[i]) * W[i];
            } else {
               chisq += 2.0 * (yfit[i] - y[i] + y[i] * log(y[i] / yfit[i])) * W[i];
            }

            // approximation to expectation value of ml chi squared
            if(yfit[i] < 4.2) {
               chisqexp += -2.0 * yfit[i] * std::log(yfit[i]) + std::pow(yfit[i], 2.0) * std::log(4.0) -
                           std::pow(yfit[i], 3.0) * std::log(4.0 / 3.0) +
                           (1.0 / 3.0) * std::pow(yfit[i], 4.0) * std::log(32.0 / 27.0) -
                           (1.0 / 12.0) * std::pow(yfit[i], 5.0) * std::log(4096.0 / 3645.0) +
                           1442.633448 * std::pow(yfit[i], 6.0) / std::pow(10.0, 6.0) -
                           1782.46047 * std::pow(yfit[i], 7.0) / std::pow(10.0, 7.0) +
                           1588.98494 * std::pow(yfit[i], 8.0) / std::pow(10.0, 8.0) -
                           716.19428 * std::pow(yfit[i], 9.0) / std::pow(10.0, 9.0);
            } else {
               chisqexp += 1.0 + 1.0 / (6.0 * yfit[i]) + 1.0 / (6.0 * std::pow(yfit[i], 2.0)) +
                           19.0 / (60.0 * std::pow(yfit[i], 3.0)) + 9.0 / (10.0 * std::pow(yfit[i], 4.0)) -
                           31.9385 / std::pow(yfit[i], 5.0) + 741.3189 / std::pow(yfit[i], 6.0) -
                           3928.1260 / std::pow(yfit[i], 7.0) + 6158.3381 / std::pow(yfit[i], 8.0);
            }
         }   // end of chi2 3
      }
   }   // end of loop over data

   for(int j = 1; j < mfit; j++) {   // Fill in the symmetric side.
      for(int k = 0; k < j; k++) {
         alpha[k][j] = alpha[j][k];
      }
   }
}

/*******************************************************************/
/*  covsrt                                                         */
/*******************************************************************/
void TLMFitter::covsrt(Mat_IO_double& covar, Vec_I_BOOL& ia, const int mfit)
{
   // Rearranges the covariance matrix covar in the order of all ma parameters
   int ma = ia.size();
   for(int i = mfit; i < ma; i++) {
      for(int j = 0; j < i + 1; j++) {
         covar[i][j] = covar[j][i] = 0.0;
      }
   }
   int k = mfit - 1;
   for(int j = ma - 1; j >= 0; j--) {
      if(ia[j]) {
         for(int i = 0; i < ma; i++) {
            SWAP(covar[i][k], covar[i][j]);
         }
         for(int i = 0; i < ma; i++) {
            SWAP(covar[k][i], covar[j][i]);
         }
         k--;
      }
   }
}

/*******************************************************************/
/*  gaussj                                                         */
/*******************************************************************/

void TLMFitter::gaussj(Mat_IO_double& a, Mat_IO_double& b)
{
   // Matrix solver
   int n = a.nrows();
   int m = b.ncols();

   Vec_INT indxc(n);
   Vec_INT indxr(n);
   Vec_INT ipiv(n);
   for(int j = 0; j < n; j++) {
      ipiv[j] = 0;
   }
   int icol = 0;
   int irow = 0;
   for(int i = 0; i < n; i++) {
      double big = 0.0;
      for(int j = 0; j < n; j++) {
         if(ipiv[j] != 1) {
            for(int k = 0; k < n; k++) {
               if(ipiv[k] == 0) {
                  if(std::fabs(a[j][k]) >= big) {
                     big  = std::fabs(a[j][k]);
                     irow = j;
                     icol = k;
                  }
               }
            }
         }
      }
      ++(ipiv[icol]);
      if(irow != icol) {
         for(int l = 0; l < n; l++) {
            SWAP(a[irow][l], a[icol][l]);
         }
         for(int l = 0; l < m; l++) {
            SWAP(b[irow][l], b[icol][l]);
         }
      }
      indxr[i] = irow;
      indxc[i] = icol;
      if(a[icol][icol] == 0.0) {
         nrerror("gaussj: Singular Matrix");
      }
      double pivinv = 1.0 / a[icol][icol];
      a[icol][icol] = 1.0;
      for(int l = 0; l < n; l++) {
         a[icol][l] *= pivinv;
      }
      for(int l = 0; l < m; l++) {
         b[icol][l] *= pivinv;
      }
      for(int ll = 0; ll < n; ll++) {
         if(ll != icol) {
            double dum  = a[ll][icol];
            a[ll][icol] = 0.0;
            for(int l = 0; l < n; l++) {
               a[ll][l] -= a[icol][l] * dum;
            }
            for(int l = 0; l < m; l++) {
               b[ll][l] -= b[icol][l] * dum;
            }
         }
      }
   }
   for(int l = n - 1; l >= 0; l--) {
      if(indxr[l] != indxc[l]) {
         for(int k = 0; k < n; k++) {
            SWAP(a[k][indxr[l]], a[k][indxc[l]]);
         }
      }
   }
}
