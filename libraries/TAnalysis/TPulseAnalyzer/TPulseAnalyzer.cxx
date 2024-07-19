#include <utility>

#include "TPulseAnalyzer.h"

TPulseAnalyzer::TPulseAnalyzer()
{
   Clear();
}

TPulseAnalyzer::TPulseAnalyzer(const TFragment& fragment, double noise_fac)
{
   Clear();
   SetData(fragment, noise_fac);
}

TPulseAnalyzer::TPulseAnalyzer(const std::vector<Short_t>& wave, double noise_fac, std::string name)
   : fName(std::move(name))
{
   Clear();
   SetData(wave, noise_fac);
}

TPulseAnalyzer::~TPulseAnalyzer()
{
	delete cWpar;
	delete spar;
	delete shpar;
}

void TPulseAnalyzer::Clear(Option_t*)
{
   SetCsI(false);
   set        = false;
   cN         = 0;
   FILTER     = 8;
   T0RANGE    = 8;
   LARGECHISQ = 1E111;
   EPS        = 0.001;

   lineq_dim = 0;
   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   memset(lineq_solution, 0, sizeof(lineq_solution));
   memset(copy_matrix, 0, sizeof(copy_matrix));
}

void TPulseAnalyzer::SetData(const TFragment& fragment, double noise_fac)
{
   if(fragment.HasWave()) {
      SetData(*fragment.GetWaveform(), noise_fac);
   }
}

void TPulseAnalyzer::SetData(const std::vector<Short_t>& wave, double noise_fac)
{
   SetCsI(false);
   cN = wave.size();
   if(cN > 0) {
      cWavebuffer = wave;
      set         = true;
      if(noise_fac > 0) {
         FILTER  = 8 * noise_fac;
         T0RANGE = 8 * noise_fac;
      }
   }
}

////////////////////////////////////////
//	Linear Equation Solver
////////////////////////////////////////

// "Very efficient" apparently, written by Kris S,
// Solve the currently stored n dimentional linear eqaution
int TPulseAnalyzer::solve_lin_eq()
{
   memcpy(copy_matrix, lineq_matrix, sizeof(lineq_matrix));
   long double w = determinant(lineq_dim);
   if(w == 0.) {
      return 0;
   }
   for(int i = 0; i < lineq_dim; i++) {
      memcpy(copy_matrix, lineq_matrix, sizeof(lineq_matrix));
      memcpy(copy_matrix[i], lineq_vector, sizeof(lineq_vector));
      lineq_solution[i] = determinant(lineq_dim) / w;
   }
   return 1;
}

// solve the determinant of the currently stored copy_matrix for dimentions m
long double TPulseAnalyzer::determinant(int m)
{
   if(m == 1) {
      return copy_matrix[0][0];
   }
   int sign = 1;
   if(copy_matrix[m - 1][m - 1] == 0.) {
      int j = m - 1;
      while(copy_matrix[m - 1][j] == 0 && j >= 0) {
         j--;
      }
      if(j < 0) {
         return 0.;
      }
      for(int i = 0; i < m; i++) {
         long double s         = copy_matrix[i][m - 1];
         copy_matrix[i][m - 1] = copy_matrix[i][j];
         copy_matrix[i][j]     = s;
      }
      sign *= -1;
   }
   for(int j = m - 2; j >= 0; j--) {
      for(int i = 0; i < m; i++) {
         copy_matrix[i][j] -= copy_matrix[i][m - 1] / copy_matrix[m - 1][m - 1] * copy_matrix[m - 1][j];
      }
   }
   return copy_matrix[m - 1][m - 1] * sign * determinant(m - 1);
}

////////////////////////////////////////
//	Waveform Fits Functions
////////////////////////////////////////

int TPulseAnalyzer::fit_smooth_parabola(int low, int high, double x0, ParPar* pp)
{
   memset(pp, 0, sizeof(ParPar));
   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   lineq_dim = 2;
   double chisq     = 0.;
   int ndf       = 0;
   int k         = static_cast<int>(rint(x0));

   for(int i = low; i < k; i++) {
      lineq_matrix[0][0] += 1;
      lineq_vector[0] += cWavebuffer[i];
      ndf++;
      chisq += cWavebuffer[i] * cWavebuffer[i];
   }

   for(int i = k; i < high; i++) {
      double x = (i - x0) * (i - x0);
      lineq_matrix[0][0] += 1;
      lineq_matrix[0][1] += x;
      lineq_matrix[1][1] += x * x;
      lineq_vector[0] += cWavebuffer[i];
      lineq_vector[1] += cWavebuffer[i] * x;
      ndf++;
      chisq += cWavebuffer[i] * cWavebuffer[i];
   }
   lineq_matrix[1][0] = lineq_matrix[0][1];

   if(solve_lin_eq() == 0) {
      pp->chisq = BADCHISQ_MAT;
      return -1;
   }
   chisq -= lineq_vector[0] * lineq_solution[0];
   chisq -= lineq_vector[1] * lineq_solution[1];

   pp->constant  = lineq_solution[0];
   pp->linear    = 0.;
   pp->quadratic = lineq_solution[1];
   pp->chisq     = chisq;
   pp->ndf       = ndf;

   return 1;
}

////////////////////////////////////////
//	RF Fit Run Functions
////////////////////////////////////////

double TPulseAnalyzer::fit_rf(double T)
{
   if(!set || cN < 10) {
      return -1;
   }
	delete spar;
   spar = new SinPar;

   spar->t0 = -1;

   return 5 * get_sin_par(T);
}

////////////////////////////////////////
//	Waveform Time Fit Run Functions
////////////////////////////////////////

// Overall function which determins limits and fits the 3 trial functions
double TPulseAnalyzer::fit_newT0()
{
   if(!set || cN < 10) {
      return -1;
   }

	delete cWpar;
   cWpar     = new WaveFormPar;
   cWpar->t0 = -1;

   double      chisq[3];
   WaveFormPar w[3];

   cWpar->baseline_range = T0RANGE;   // default only 8 samples!
   get_baseline();
   get_tmax();

   if(good_baseline() == 0) {
      return -1;
   }

   get_t30();
   get_t50();
   cWpar->thigh = cWpar->t50;

   for(double& i : chisq) {
      i = LARGECHISQ;
   }

   size_t swp = sizeof(WaveFormPar);
   chisq[0] = get_smooth_T0();
   memcpy(&w[0], cWpar, swp);
   chisq[1] = get_parabolic_T0();
   memcpy(&w[1], cWpar, swp);
   chisq[2] = get_linear_T0();
   memcpy(&w[2], cWpar, swp);

   double chimin = LARGECHISQ;
   int imin   = 0;

   for(int i = 0; i < 3; i++) {
      if(chisq[i] < chimin && chisq[i] > 0) {
         chimin = chisq[i];
         imin   = i;
      }
   }

   if(imin < 2) {
      memcpy(cWpar, &w[imin], swp);
   }

   get_baseline_fin();
   return cWpar->t0;
}
/*================================================================*/

/*================================================================*/
int TPulseAnalyzer::fit_parabola(int low, int high, ParPar* pp)
{
   memset(pp, 0, sizeof(ParPar));
   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   lineq_dim = 3;
   double chisq     = 0.;
   int ndf       = 0;
   for(int i = low; i < high; i++) {
      lineq_matrix[0][0] += 1;
      lineq_matrix[0][1] += i;
      lineq_matrix[0][2] += i * i;
      lineq_matrix[1][2] += i * i * i;
      lineq_matrix[2][2] += i * i * i * i;
      lineq_vector[0] += cWavebuffer[i];
      lineq_vector[1] += cWavebuffer[i] * i;
      lineq_vector[2] += cWavebuffer[i] * i * i;
      ndf++;
      chisq += cWavebuffer[i] * cWavebuffer[i];
   }
   lineq_matrix[1][0] = lineq_matrix[0][1];
   lineq_matrix[1][1] = lineq_matrix[0][2];
   lineq_matrix[2][0] = lineq_matrix[0][2];
   lineq_matrix[2][1] = lineq_matrix[1][2];

   if(solve_lin_eq() == 0) {
      pp->chisq = BADCHISQ_MAT;
      return -1;
   }
   chisq -= lineq_vector[0] * lineq_solution[0];
   chisq -= lineq_vector[1] * lineq_solution[1];
   chisq -= lineq_vector[2] * lineq_solution[2];
   pp->constant  = lineq_solution[0];
   pp->linear    = lineq_solution[1];
   pp->quadratic = lineq_solution[2];
   pp->chisq     = chisq;
   pp->ndf       = ndf;
   return 1;
}

/*================================================================*/
int TPulseAnalyzer::fit_line(int low, int high, LinePar* lp)
{
   memset(lp, 0, sizeof(LinePar));
   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   lineq_dim = 2;
   double chisq     = 0.;
   int ndf       = 0;
   for(int i = low; i < high; i++) {
      lineq_matrix[0][0] += 1;
      lineq_matrix[0][1] += i;
      lineq_matrix[1][1] += i * i;
      lineq_vector[0] += cWavebuffer[i];
      lineq_vector[1] += cWavebuffer[i] * i;
      ndf++;
      chisq += cWavebuffer[i] * cWavebuffer[i];
   }
   lineq_matrix[1][0] = lineq_matrix[0][1];

   if(solve_lin_eq() == 0) {
      lp->chisq = BADCHISQ_MAT;
      return -1;
   }
   chisq -= lineq_vector[0] * lineq_solution[0];
   chisq -= lineq_vector[1] * lineq_solution[1];
   lp->slope     = lineq_solution[1];
   lp->intercept = lineq_solution[0];
   lp->chisq     = chisq;
   lp->ndf       = ndf;
   return 1;
}
/*================================================================*/

/*======================================================*/

double TPulseAnalyzer::get_linear_T0()
{
   LinePar lp, lpl;
   int     k;
   double  chit, chitmin;
   double  b, c, t;

   chitmin = LARGECHISQ;

   for(k = T0RANGE / 2; k < cWpar->thigh - T0RANGE / 8; k++) {
      // fit line to the baseline
      fit_line(0, k, &lp);

      // fit line to the risetime
      fit_line(k, cWpar->thigh, &lpl);

      chit = lp.chisq + lpl.chisq;

      if(chit < chitmin) {
         chitmin   = chit;
         cWpar->b0 = lp.intercept;
         cWpar->b1 = lp.slope;
         cWpar->s0 = lpl.intercept;
         cWpar->s1 = lpl.slope;
         cWpar->s2 = 0.;
      }
   }   // end of the loop over k
   b = cWpar->s1 - cWpar->b1;
   c = cWpar->s0 - cWpar->b0;
   t = -c / b;

   cWpar->t0    = -1;
   cWpar->temin = 0;
   cWpar->temax = cWpar->thigh;
   if(t < cN && t > 0) {
      cWpar->t0    = t;
      cWpar->temin = static_cast<int>(rint(cWpar->t0)) - 2;
      cWpar->temax = static_cast<int>(rint(cWpar->t0)) + 2;
      return (chitmin / (cWpar->thigh - 5));
   }

   return BADCHISQ_LIN_T0;
}
/*================================================================*/
double TPulseAnalyzer::get_smooth_T0()
{
   ParPar pp, ppmin;
   int    k, kmin;
   double chit, chitmin;
   double c, t;

   memset(&ppmin, 0, sizeof(ParPar));

   chitmin = LARGECHISQ;
   kmin    = 0;
   // corse search first
   for(k = T0RANGE / 2; k < cWpar->thigh - T0RANGE / 2; k++) {
      fit_smooth_parabola(0, cWpar->thigh, static_cast<double>(k), &pp);

      chit = pp.chisq;
      if(chit < chitmin) {
         chitmin = chit;
         kmin    = k;
      }
   }   // end of the corse search loop over k
   c = kmin;

   chitmin = LARGECHISQ;
   // fine search next
   for(t = kmin - 1; t < kmin + 1; t += 0.1) {
      fit_smooth_parabola(0, cWpar->thigh, t, &pp);
      chit = pp.chisq;
      if(chit < chitmin) {
         memcpy(&ppmin, &pp, sizeof(ParPar));
         chitmin = chit;
         c       = t;
      }
   }   // end of the fine search loop over k

   memcpy(&pp, &ppmin, sizeof(ParPar));
   t         = c;
   cWpar->s0 = pp.constant + pp.quadratic * t * t;
   cWpar->s1 = -2. * pp.quadratic * t;
   cWpar->s2 = pp.quadratic;
   cWpar->b0 = pp.constant;
   cWpar->b1 = 0.;

   cWpar->t0    = -1;
   cWpar->temin = 0;
   cWpar->temax = cWpar->thigh;
   if(t < cN && t > 0) {
      cWpar->t0    = t;
      cWpar->temin = static_cast<int>(rint(cWpar->t0)) - 2;
      cWpar->temax = static_cast<int>(rint(cWpar->t0)) + 2;
      return (chitmin / (cWpar->thigh - 2));
   }
   return BADCHISQ_SMOOTH_T0;
}
/*================================================================*/
double TPulseAnalyzer::get_parabolic_T0()
{

   LinePar lp;
   ParPar  pp;
   int     k;
   double  chit, chitmin;
   double  a, b, c, d, t;

   chitmin = LARGECHISQ;
   for(k = T0RANGE / 2; k < cWpar->thigh - T0RANGE / 2; k++) {
      // fit line to the baseline
      fit_line(0, k, &lp);

      // fit parabola to the risetime
      fit_parabola(k, cWpar->thigh, &pp);

      chit = lp.chisq + pp.chisq;

      if(chit < chitmin) {
         chitmin   = chit;
         cWpar->b0 = lp.intercept;
         cWpar->b1 = lp.slope;
         cWpar->s0 = pp.constant;
         cWpar->s1 = pp.linear;
         cWpar->s2 = pp.quadratic;
      }
   }   // end loop through k

   a = cWpar->s2;
   b = cWpar->s1 - cWpar->b1;
   c = cWpar->s0 - cWpar->b0;
   d = b * b - 4 * a * c;

   if(a == 0.) {
      t = -c / b;
   } else {
      if(d >= 0) {
         if(d == 0.) {
            t = -0.5 * b / a;
         } else {
            d = sqrt(d);
            t = 0.5 * (-b + d) / a;
         }
      } else {
         return BADCHISQ_PAR_T0;
      }
   }

   cWpar->t0    = -1;
   cWpar->temin = 0;
   cWpar->temax = cWpar->thigh;
   if(t < cN && t > 0) {
      cWpar->t0    = t;
      cWpar->temin = static_cast<int>(rint(cWpar->t0)) - 2;
      cWpar->temax = static_cast<int>(rint(cWpar->t0)) + 2;
      return (chitmin / (cWpar->thigh - 5));
   }
   return BADCHISQ_PAR_T0;
}

// Measure the baseline and standard deviation of the waveform, over the tick range specified by
// cWpar->baseline_range
void TPulseAnalyzer::get_baseline()
{
   cWpar->baseline      = 0.;
   cWpar->baselineStDev = 0.;

   // error if waveform length cN is shorter than baseline range
   if(cN < cWpar->baseline_range) {
      std::cout << "Baseline range (" << cWpar->baseline_range << ") larger than waveform length!" << std::endl;
      std::cout << "Terminating program" << std::endl;
      exit(0);
   }

   for(int i = 0; i < cWpar->baseline_range; i++) {
      cWpar->baseline += cWavebuffer[i];
      cWpar->baselineStDev += cWavebuffer[i] * cWavebuffer[i];
   }

   cWpar->baselineStDev /= cWpar->baseline_range;
   cWpar->baseline /= cWpar->baseline_range;
   cWpar->baselineStDev -= cWpar->baseline * cWpar->baseline;
   cWpar->baselineStDev = sqrt(cWpar->baselineStDev);
   cWpar->bflag         = 1;   // flag after establishing baseline
}

/*======================================================*/

// Measure the baseline and standard deviation up to tick cWpar->t0 after a fit
void TPulseAnalyzer::get_baseline_fin()
{
   cWpar->baselinefin      = 0.;
   cWpar->baselineStDevfin = 0.;
   int tb                  = cWpar->t0;   // t0 non integer, result always too small before.
   if(tb > T0RANGE + 10) {
      tb -= 10;
   }

   // error if waveform length cN is shorter than baseline range
   if(cN > tb && tb > 0) {
      for(int i = 0; i < tb; i++) {
         cWpar->baselinefin += cWavebuffer[i];
         cWpar->baselineStDevfin += cWavebuffer[i] * cWavebuffer[i];
      }

      cWpar->baselineStDevfin /= tb;
      cWpar->baselinefin /= tb;
      cWpar->baselineStDevfin -= cWpar->baselinefin * cWpar->baselinefin;
      cWpar->baselineStDevfin = sqrt(std::abs(cWpar->baselineStDevfin));
   }
}

/*======================================================*/

// Find the maximum of the wavefunction, smoothed with a moving average filter
void TPulseAnalyzer::get_tmax()
{
   int i, j, sum;
   int D = FILTER / 2;

   cWpar->max  = cWavebuffer[0];
   cWpar->tmax = 0;

   // applies the filter to the waveform
   //     cout<<" "<<cWpar->tmax<<" "<< cWpar->max<<flush;
   for(i = D; i < cN - D; i++) {
      sum = 0;
      for(j = i - D; j < i + D; j++) {
         sum += cWavebuffer[j];
      }
      sum /= FILTER;   // the value of the filtered waveform at i
      if(sum > cWpar->max) {
         // if the value of the filtered waveform at i is larger than the current maximum, max=value and tmax = i
         cWpar->max  = sum;
         cWpar->tmax = i;
      }
   }
   cWpar->mflag = 1;   // flag after finding tmax
}

/*===========================================================*/
double TPulseAnalyzer::get_tfrac(double frac, double fraclow, double frachigh)
{
   if(cWpar->bflag != 1) {
      std::cout << "Baseline not deterimned for the tfraction" << std::endl;
      exit(1);
   }

   if(cWpar->mflag != 1) {
      std::cout << "Maximum not deterimned for the tfraction" << std::endl;
      exit(1);
   }

   int t = cWpar->tmax;

   double f     = cWpar->baseline + frac * (cWpar->max - cWpar->baseline);
   double flow  = cWpar->baseline + fraclow * (cWpar->max - cWpar->baseline);
   double fhigh = cWpar->baseline + frachigh * (cWpar->max - cWpar->baseline);

   while(cWavebuffer[t] > f) {
      t--;
      if(t <= 4) {
         break;
      }
   }
   int imin = t;
   while(cWavebuffer[imin] > flow) {
      imin--;
      if(imin <= 1) {
         break;
      }
   }

   int imax = t;

   while(cWavebuffer[imax] < fhigh) {
      imax++;
      if(imax >= cN - 1) {
         break;
      }
   }

   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   lineq_dim = 3;

   int i                  = imax - imin;
   int64_t a                  = i;
   lineq_matrix[0][0] = a + 1;
   lineq_matrix[0][1] = 0.5 * a;
   lineq_matrix[2][0] = a / 6.;
   lineq_matrix[2][2] = -a / 30.;
   a *= i;
   lineq_matrix[0][1] += 0.5 * a;
   lineq_matrix[2][0] += 0.5 * a;
   lineq_matrix[2][1] = 0.25 * a;
   a *= i;
   lineq_matrix[2][0] += a / 3.;
   lineq_matrix[2][1] += 0.5 * a;
   lineq_matrix[2][2] += a / 3.;
   a *= i;
   lineq_matrix[2][1] += 0.25 * a;
   lineq_matrix[2][2] += 0.5 * a;
   a *= i;
   lineq_matrix[2][2] += 0.2 * a;

   lineq_matrix[1][0] = lineq_matrix[0][1];
   lineq_matrix[1][1] = lineq_matrix[2][0];
   lineq_matrix[0][2] = lineq_matrix[2][0];
   lineq_matrix[1][2] = lineq_matrix[2][1];

   for(i = 0; i < lineq_dim; i++) {
      lineq_vector[i] = 0;
   }

   for(i = imin; i < imax + 1; i++) {
      a = i - imin;
      lineq_vector[0] += cWavebuffer[i];
      lineq_vector[1] += cWavebuffer[i] * a;
      lineq_vector[2] += cWavebuffer[i] * a * a;
   }

   if(solve_lin_eq() == 0) {
      return -4;
   }
   double p = lineq_solution[0] - f;
   double q = lineq_solution[1];
   double r = lineq_solution[2];

   if(r != 0) {
      double d = q * q - 4 * r * p;
      if(d < 0) {
         return -5;
      }
      f = -q + sqrt(d);
      f *= 0.5;
      f /= r;
      f += imin;
      return f;
   }
   if(q != 0) {
      f = -p / q;
      return f;
   }
   return -6;

   return -7;
}

/* ==================================================== */
void TPulseAnalyzer::get_t50()
{
   int t;

   t = get_tfrac(0.5, 0.3, 0.8);
   if((t > 0) && (t < MAX_SAMPLES)) {
      cWpar->t50_flag = 1;
      cWpar->t50      = t;
   } else {
      cWpar->t50_flag = -1;
      cWpar->t50      = -1;
   }
}
/* ==================================================== */
void TPulseAnalyzer::get_t90()
{
   int t;

   t = get_tfrac(0.9, 0.8, 0.98);

   if((t > 0) && (t < MAX_SAMPLES)) {
      cWpar->t90_flag = 1;
      cWpar->t90      = t;
   } else {
      cWpar->t90_flag = -1;
      cWpar->t90      = -1;
   }
}
/*===========================================================*/
void TPulseAnalyzer::get_t10()
{
   int t;

   t = get_tfrac(0.1, 0.05, 0.2);

   if((t > 0) && (t < MAX_SAMPLES)) {
      cWpar->t10_flag = 1;
      cWpar->t10      = t;
   } else {
      cWpar->t10_flag = -1;
      cWpar->t10      = -1;
   }
}
/*===========================================================*/
void TPulseAnalyzer::get_t30()
{
   int t;

   t = get_tfrac(0.3, 0.15, 0.45);
   if((t > 0) && (t < MAX_SAMPLES)) {
      cWpar->t30_flag = 1;
      cWpar->t30      = t;
   } else {
      cWpar->t30_flag = -1;
      cWpar->t30      = -1;
   }
}

double TPulseAnalyzer::get_sin_par(double T)
{
   int    i;
   double s, sn, snm, s2, s2n, s2nm, c, cn, cnm, c2, c2n, c2nm, w;
   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   lineq_dim = 3;

   w = 2 * TMath::Pi() / T;

   s    = sin(w);
   sn   = sin(cN * w);
   snm  = sin((cN - 1) * w);
   s2   = sin(2 * w);
   s2n  = sin(2 * cN * w);
   s2nm = sin(2 * (cN - 1) * w);

   c    = cos(w);
   cn   = cos(cN * w);
   cnm  = cos((cN - 1) * w);
   c2   = cos(2 * w);
   c2n  = cos(2 * cN * w);
   c2nm = cos(2 * (cN - 1) * w);

   lineq_matrix[0][0] = 0.5 * cN - 0.25 * (1 - c2 - c2n + c2nm) / (1 - c2);
   lineq_matrix[0][1] = 0.25 * (s2 + s2nm - s2n) / (1 - c2);
   lineq_matrix[1][0] = lineq_matrix[0][1];
   lineq_matrix[0][2] = 0.5 * (s + snm - sn) / (1 - c);
   lineq_matrix[2][0] = lineq_matrix[0][2];
   lineq_matrix[1][1] = 0.5 * cN + 0.25 * (1 - c2 - c2n + c2nm) / (1 - c2);
   lineq_matrix[1][2] = 0.5 * (1 - c - cn + cnm) / (1 - c);
   lineq_matrix[2][1] = lineq_matrix[1][2];
   lineq_matrix[2][2] = cN;

   for(i = 0; i < lineq_dim; i++) {
      lineq_vector[i] = 0;
   }

   for(i = 0; i < cN; i++) {
      lineq_vector[0] += cWavebuffer[i] * sin(w * i);
      lineq_vector[1] += cWavebuffer[i] * cos(w * i);
      lineq_vector[2] += cWavebuffer[i];
   }
   if(solve_lin_eq() == 0) {
      return 0;
   }
   spar->A = sqrt(lineq_solution[0] * lineq_solution[0] + lineq_solution[1] * lineq_solution[1]);
   spar->C = lineq_solution[2];

   s = -lineq_solution[1] / spar->A;
   c = lineq_solution[0] / spar->A;

   if(s >= 0) {
      spar->t0 = acos(c) * T / (2 * TMath::Pi());
   } else {
      spar->t0 = (1 - acos(c) / (2 * TMath::Pi())) * T;
   }

   return spar->t0;
}

/*======================================================*/
// void TPulseAnalyzer::get_sig2noise(){ if(cN==0)return;
// 	if(set){
// 		get_baseline();
// 		get_tmax();
// 		cWpar->sig2noise=(cWpar->max-cWpar->baseline)/cWpar->baselineStDev;
// 	}
// 	return;
// }
/*======================================================*/
double TPulseAnalyzer::get_sig2noise()
{
   if(set && (cWpar != nullptr)) {
      if(cWpar->t0 > 0) {
         return (cWpar->max - cWpar->baselinefin) / cWpar->baselineStDevfin;
      }
   }
   return -1;
}

int16_t TPulseAnalyzer::good_baseline()
{
   if(set && (cWpar != nullptr)) {
      if(cWpar->tmax < T0RANGE) {
         return 0;
      }
      if((cWpar->max - cWpar->baseline) < (cWpar->baseline - cWavebuffer[0]) * 10) {
         return 0;
      }
      if((cWpar->max - cWavebuffer[T0RANGE]) < (cWavebuffer[T0RANGE] - cWpar->baseline) * 4) {
         return 0;
      }
      return 1;
   }
   return 0;
}

//=====================================================//
//	CsI functions:
//=====================================================//

int TPulseAnalyzer::GetCsIChiSq()
{
   if(CsIIsSet()) {
      return shpar->chisq;
   }
   return -1;
}

int TPulseAnalyzer::GetCsIFitType()
{
   if(CsIIsSet()) {
      return shpar->type;
   }
   return -1;
}

double TPulseAnalyzer::CsIt0()
{

   if(CsIIsSet()) {
      return shpar->t[0];
   }
   if(!set || cN < 10) {
      return -1.;
   }

   if(cWpar != nullptr) {
      delete cWpar;
   }
   cWpar = new WaveFormPar;
   if(shpar != nullptr) {
      delete shpar;
   }
   shpar = new ShapePar;

   GetCsIExclusionZone();
   if(cWpar->teflag == 1) {
      // good exclusion zone
      int tmpchisq = GetCsIShape();
      if(tmpchisq >= 0) {
         SetCsI();
         return shpar->t[0];
      }
   }

   return -1.0;
}

double TPulseAnalyzer::CsIPID()
{
   // std::cout<<"Fitting CsI PID"<<std::endl;
   if(CsIIsSet()) {
      double f = shpar->am[2];
      double s = shpar->am[3];
      double r = s / f * 100;

      return r;
   }
   if(!set || cN < 10) {
      return -1.;
   }

   if(cWpar != nullptr) {
      delete cWpar;
   }
   cWpar = new WaveFormPar;
   if(shpar != nullptr) {
      delete shpar;
   }
   shpar = new ShapePar;

   shpar->t[1] = 4510;
   shpar->t[2] = 64.3;
   shpar->t[3] = 380.0;

   GetCsIExclusionZone();

   if(cWpar->bflag == 0) {
      shpar->type = -2;   // type for failed exclusion zone determination
      return BAD_BASELINE_RANGE;
   }
   if(cWpar->mflag == 0) {
      shpar->type = -2;   // type for failed exclusion zone determination
      return BAD_MAX;
   }
   if(cWpar->teflag == 0) {
      shpar->type = -2;   // type for failed exclusion zone determination
      return BAD_EXCLUSION_ZONE;
   }

   int tmpchisq = GetCsIShape();
   if(tmpchisq >= 0) {
      double f;
      double s;
      double r;

      f = shpar->am[2];
      s = shpar->am[3];
      r = s / f * 100;

      SetCsI();

      return r;
   }

   return BAD_PID;
}

int TPulseAnalyzer::GetCsIShape()
{

   double chisq[4], chimin;   // chisq array for fit types: 0=2 comp, 1=fast, 2=slow, 3=gamma on PIN

   int ndf;
   int i, imin;

   // calculate ndf assuming two component (4 parameter) fit
   ndf = -4;
   for(i = 0; i < cWpar->temin; i++) {
      ndf++;
   }
   for(i = cWpar->temax; i < cN; i++) {
      ndf++;
   }

   // std::cout<<"ndf 4 parameters: "<<ndf<<std::endl;

   // initialize chisq to large value and set up waveform parameters
   for(i = 0; i < 4; i++) {
      chisq[i]        = LARGECHISQ;
      csiTestShpar[i] = new ShapePar;
      csiTestWpar[i]  = new WaveFormPar;
      memcpy(csiTestShpar[i], shpar, sizeof(ShapePar));
      memcpy(csiTestWpar[i], cWpar, sizeof(WaveFormPar));
   }

   // two component fit
   FitCsIShape(4, csiTestShpar[0], csiTestWpar[0]);
   chisq[0] = csiTestShpar[0]->chisq / ndf;

   // for 3 parameter fits, ndf is one larger
   ndf++;

   // std::cout<<"ndf 3 parameters: "<<ndf<<std::endl;

   // fast only for high Z recoils
   FitCsIShape(3, csiTestShpar[1], csiTestWpar[1]);
   chisq[1] = csiTestShpar[1]->chisq / ndf;

   // slow only for gamma on CsI
   csiTestShpar[2]->t[2] = shpar->t[3];
   csiTestShpar[2]->t[3] = shpar->t[2];
   FitCsIShape(3, csiTestShpar[2], csiTestWpar[2]);
   chisq[2] = csiTestShpar[2]->chisq / ndf;

   // gamma on PIN
   csiTestShpar[3]->t[2] = shpar->t[4];
   csiTestShpar[3]->t[4] = shpar->t[2];
   FitCsIShape(3, csiTestShpar[3], csiTestWpar[3]);
   chisq[3] = csiTestShpar[3]->chisq / ndf;

   /*std::cout<<"0=two comp 1=fast 2=slow 3=gamma on PIN"<<std::endl;
   for(i = 0; i < 4; i++)
      std::cout<<"chisq "<<csiTestShpar[i]->chisq<<", chisq["<<i<<"]/ndf "<<chisq[i]<<std::endl;*/

   // find minimum chisq
   imin   = -1;
   chimin = LARGECHISQ;
   for(i = 0; i < 4; i++) {
      if((chisq[i] < chimin) && (chisq[i] > 0)) {
         chimin = chisq[i];
         imin   = i;
      }
	}

   // std::cout<<"minimum chisq["<<imin<<"]/ndf "<<chisq[imin]<<", t0min "<<csiTestShpar[imin]->t[0]<<std::endl;

   switch(imin) {
   case 3:
      // gamma on PIN fit type
      memcpy(shpar, csiTestShpar[imin], sizeof(ShapePar));
      memcpy(cWpar, csiTestWpar[imin], sizeof(WaveFormPar));
      shpar->t[2]  = csiTestShpar[imin]->t[4];
      shpar->am[2] = csiTestShpar[imin]->am[4];
      shpar->t[4]  = csiTestShpar[imin]->t[2];
      shpar->am[4] = csiTestShpar[imin]->am[2];
      shpar->type  = imin + 1;   // gamma on PIN type
      break;
   case 2:
      // slow component only fit type
      memcpy(shpar, csiTestShpar[imin], sizeof(ShapePar));
      memcpy(cWpar, csiTestWpar[imin], sizeof(WaveFormPar));
      shpar->t[2]  = csiTestShpar[imin]->t[3];
      shpar->am[2] = csiTestShpar[imin]->am[3];
      shpar->t[3]  = csiTestShpar[imin]->t[2];
      shpar->am[3] = csiTestShpar[imin]->am[2];
      shpar->type  = imin + 1;   // slow only type
      break;
   case 1:
      // fast component only fit type
      memcpy(shpar, csiTestShpar[imin], sizeof(ShapePar));
      memcpy(cWpar, csiTestWpar[imin], sizeof(WaveFormPar));
      shpar->type = imin + 1;   // fast only type
      break;
   case 0:
      // two component fit type
      memcpy(shpar, csiTestShpar[imin], sizeof(ShapePar));
      memcpy(cWpar, csiTestWpar[imin], sizeof(WaveFormPar));
      shpar->type = imin + 1;   // two component type
      break;
   default:
      // failed fit
      shpar->type  = -1;                     // fit failure type
      shpar->chisq = BADCHISQ_FAIL_DIRECT;   // set it here so it still frees memory on bad fits
      break;
   }

   // free memory allocated for the fit
   for(i = 0; i < 4; i++) {
      delete csiTestShpar[i];
      delete csiTestWpar[i];
   }

   return shpar->chisq;   // generic chisq return statement for all types
}

int TPulseAnalyzer::FitCsIShape(int dim, ShapePar* par, WaveFormPar* wpar)
{
   long double sum, tau, tau_i, tau_j;
   int         p, q, d;

   memset(lineq_matrix, 0, sizeof(lineq_matrix));
   memset(lineq_vector, 0, sizeof(lineq_vector));
   memset(lineq_solution, 0, sizeof(lineq_solution));

   /* q is the low limit of the signal section */
   q = wpar->temax;
   if(q >= cN || q <= 0) {
      par->chisq = -1;
      return -1.;
   }

   /* p is the high limit of the baseline section */
   p = wpar->temin;
   if(p >= cN || p <= 0) {
      par->chisq = -1;
      return -1.;
   }
   lineq_dim = dim;

   // initialize amplitudes to 0
   for(long double& i : par->am) {
      i = 0.;
   }

   d = cN;

   // initialize chi square 0 and ndf = n-k to -k where k=dim
   par->chisq = 0.;
   par->ndf   = -lineq_dim;

   /**************************************************************************
     linearized chi square fit is Mu = v where M is a data matrix
     u, v are vectors; u is the parameter vector (solution)
     note that in this formulation, chisq_min = y_i^2-sum(u_iv_i)
    **************************************************************************/

   // create matrix for linearized fit
   for(int i = 1; i < lineq_dim; i++) {
      tau   = GetCsITau(i, par);
      tau_i = tau;
      sum   = -(static_cast<double>(q)) / tau + log(1. - exp(-(static_cast<double>(d - q)) / tau));
      sum -= log(1. - exp(-1. / tau));
      lineq_matrix[i][0] = exp(sum);
      lineq_matrix[0][i] = exp(sum);

      tau /= 2.;
      sum = -(static_cast<double>(q)) / tau + log(1. - exp(-(static_cast<double>(d - q)) / tau));
      sum -= log(1. - exp(-1. / tau));
      lineq_matrix[i][i] = exp(sum);

      for(int j = i + 1; j < lineq_dim; j++) {
         tau_j = GetCsITau(j, par);
         tau   = (tau_i * tau_j) / (tau_i + tau_j);
         sum   = -(static_cast<double>(q)) / tau + log(1. - exp(-(static_cast<double>(d - q)) / tau));
         sum -= log(1. - exp(-1. / tau));
         lineq_matrix[i][j] = exp(sum);
         lineq_matrix[j][i] = exp(sum);
      }
   }

   lineq_vector[0]    = 0;
   lineq_matrix[0][0] = 0;

   for(int j = q; j < cN; j++) {
      lineq_vector[0] += cWavebuffer[j];
      lineq_matrix[0][0] += 1;
      par->chisq += cWavebuffer[j] * cWavebuffer[j];
      par->ndf += 1;
   }

   if(lineq_dim >= cN) {
      par->chisq = -1;
      return -1.;
   }

   for(int i = 1; i < lineq_dim; i++) {
      tau             = GetCsITau(i, par);
      lineq_vector[i] = 0;
      for(int j = q; j < cN; j++) {
         lineq_vector[i] += cWavebuffer[j] * exp(-(static_cast<double>(j)) / tau);
      }
   }

   for(int j = 0; j < p; j++) {
      lineq_vector[0] += cWavebuffer[j];
      lineq_matrix[0][0] += 1;
      par->chisq += cWavebuffer[j] * cWavebuffer[j];
      par->ndf += 1;
   }

   // solve the matrix equation Mu = v -> u = M^(-1)v where M^(-1) is the inverse
   // of M. note this has no solution if M is not invertable!

   // error if the matrix cannot be inverted
   if(solve_lin_eq() == 0) {
      par->chisq = BADCHISQ_MAT;
      par->ndf   = 1;

      return BADCHISQ_MAT;
   }   // else try and find t0 and calculate amplitudes

   // see the function comments for find_t0 for details

   par->t[0] = GetCsIt0(par, wpar);

   // if t0 is less than 0, return a T0FAIL
   if(par->t[0] <= 0) {
      par->chisq = BADCHISQ_T0;
      par->ndf   = 1;
      return BADCHISQ_T0;
   }

   // calculate amplitudes
   par->am[0] = lineq_solution[0];

   for(int i = 1; i < lineq_dim; i++) {
      tau        = GetCsITau(i, par);
      par->am[i] = lineq_solution[i] * exp(-par->t[0] / tau);
   }
   // done calculating amplitudes

   for(int i = 0; i < lineq_dim; i++) {
      par->chisq -= lineq_solution[i] * lineq_vector[i];
   }

   if(par->chisq < 0) {
      par->chisq = BADCHISQ_NEG;
      par->ndf   = 1;
      return BADCHISQ_NEG;
   }

   for(int i = 2; i < lineq_dim; i++) {
      par->am[i] *= -1;
   }

   par->type = dim - 2;

   // return BADCHISQ_AMPL if a component amplitude is less than 0
   //(apart from the baseline which can be negative)
   for(int i = 1; i < lineq_dim; i++) {
      if(par->am[i] < 0) {
         par->chisq = BADCHISQ_AMPL;
         par->ndf   = 1;
         return BADCHISQ_AMPL;
      }
   }

   // std::cout<<"chisq from FitCsIShape: "<<par->chisq<<std::endl;

   return par->chisq;
}

void TPulseAnalyzer::GetCsIExclusionZone()
{
   int    i, j;
   int    D = FILTER / 2;   // filter half width
   double sum;              // sum of waveform across filter

   // initilize the fit parameters for the risetime to 0 for safety
   cWpar->afit = 0.;
   cWpar->bfit = 0.;

   // make sure the baseline is established prior to finding the exclusion zone
   cWpar->baseline_range = CSI_BASELINE_RANGE;
   get_baseline();

   // Here we determine the x position temax of the upper limit for the exclusion zone.
   // find tmax and define baselineMax
   get_tmax();

   // If tmax is established, continue.
   if(cWpar->mflag == 1) {
      cWpar->baselineMax = cWpar->baseline + NOISE_LEVEL_CSI;

      // Starting at tmax and working backwards along the waveform get the value of the filtered waveform at i and
      // when
      // the value of the filtered waveform goes below baselineMax, set the upper limit of the exclusion zone temax =
      // i.
      // The exclusion zone cannot be defined in the area of the waveform used to calculate the baseline.
      for(i = cWpar->tmax; i > cWpar->baseline_range; i--) {
         sum = 0.;
         for(j = i - D; j < i + D; j++) {
            sum += cWavebuffer[j];
         }
         sum /= FILTER;
         if(sum < cWpar->baselineMax) {
            cWpar->temax  = i;
            cWpar->teflag = 1;
            break;
         }
      }

      if(cWpar->temax > cWpar->tmax || cWpar->temax < cWpar->baseline_range || cWpar->temax < 0) {
         cWpar->teflag = 0;
      }
      // End of the determination of the upper limit of the exclusion zone.

      // Here we determine the x position of the lower limit for the exclusion zone
      /***** Fitting the risetime *****/
      if(cWpar->teflag == 1) {
         // Set baselineMin
         cWpar->baselineMin = cWpar->baseline - NOISE_LEVEL_CSI;

         // Here we fit a line y=ax+b from temax to temax + 3*FILTER and find the intersection with baselineMin. The
         // x
         // coordinate of this intersection becomes temin.
         // Matrix for the fit
         memset(lineq_matrix, 0, sizeof(lineq_matrix));
         memset(lineq_vector, 0, sizeof(lineq_vector));
         memset(lineq_solution, 0, sizeof(lineq_solution));

         lineq_dim = 2;
         for(i = cWpar->temax; i <= cWpar->temax + 3 * FILTER; i++) {
            lineq_matrix[0][0] += 1;
            lineq_matrix[0][1] += i;
            lineq_matrix[1][1] += i * i;

            lineq_vector[0] += cWavebuffer[i];

            lineq_vector[1] += cWavebuffer[i] * i;
         }
         lineq_matrix[1][0] = lineq_matrix[0][1];

         // solve_lin_eq returns 0 if the determinant of the matrix is 0 the system is unsolvable. If there is no
         // solution, set temin to the upper limit of the baseline range.
         if(solve_lin_eq() == 0) {
            cWpar->temin = cWpar->baseline_range;
         } else {
            cWpar->bfit = lineq_solution[0];
            cWpar->afit = lineq_solution[1];
            // solve baselineMin = afit*x + bfit for x to find the crossing point. If the crossing point is outside
            // the
            // acceptable range, set temin to the upper limit of the baseline range.
            cWpar->temin = static_cast<int>(floor((cWpar->baselineMin - cWpar->bfit) / cWpar->afit));
            if(cWpar->temin < cWpar->baseline_range) {
               cWpar->temin = cWpar->baseline_range;
            }
            if(cWpar->temin > cWpar->temax) {
               cWpar->temin = cWpar->baseline_range;
            }
         }
      }
      // End of the determination of the lower limit of the exclusion zone.
   }
}

double TPulseAnalyzer::GetCsITau(int i, ShapePar* par)
{

   if(i == 1) {
      return par->t[1];
   }

   if(i >= 2) {
      if(i < NSHAPE) {
         return par->t[i] * par->t[1] / (par->t[i] + par->t[1]);
      }
   }

   return -1.;
}

double TPulseAnalyzer::GetCsIt0(ShapePar* par, WaveFormPar* wpar)
{
   /*************************************************************************
   This function calculates t0 given the matrix solution from FitCsIShape.
   In this case, the fit function is written as follows:

   f(t) = C + (Af+As)*exp(t0/tRC)*exp(-t/tRC) - Af*exp(t0/tF')*exp(-t/tF')
   - As*exp(t0/tS')*exp(-t/tS')

   This can be re-written as:

   f(t) = C' + alpha*exp(-t/tRC) + beta*exp(-t/tF') + gamma*exp(-t/tS')

   Where:
   C = C'
   alpha = (Af+As)*exp(t0/tRC)
   beta  = -Af*exp(t0/tF')
   gamma = -As*exp(t0/tS')

   Ignoring the constant, we have:

   f'(t0) = alpha*exp(-t0/tRC) + beta*exp(-t0/tF') + gamma*exp(-t0/tS') = 0

   For t<t0, f'(t)< 0, and for t>t0, f'(t)>0. This function finds the
   intersection of f'(t) and 0 by linear interpolation from these endpoints.
   *************************************************************************/

   double fa, fb, fc;       // value of the fit function at points a,b,c
   double ta, tb, tc = 0;   // corresponding time (x-)axis values
   double slope;            // linear interpolation slope
   double delta;            // checks how close is the interpolated f(t0) is to 0
   double tau;
   int    i;

   ta = wpar->baseline_range;
   // ta=wpar->temin;
   fa = 0.;

   // t0 must be between the baseline and the max
   // calculates fit value (no constant) at the end of the baseline range
   // this is the t<t0 point
   for(i = 1; i < lineq_dim; i++) {
      tau = GetCsITau(i, par);
      // getc(stdin);
      fa += lineq_solution[i] * exp(-ta / tau);
   }

   tb = wpar->tmax;
   // tb=wpar->temax;
   fb = 0.;

   // calculates fit value (no constant) at tmax
   // this is the t>t0 point
   for(i = 1; i < lineq_dim; i++) {
      tau = GetCsITau(i, par);
      fb += lineq_solution[i] * exp(-tb / tau);
   }

   delta = 1;

   if((fa < 0) && (fb > 0)) {
      // keep the interpolation going until you get below epsilon
      /* |f(t0) - 0| = |f(t0)|< epsilon */
      while(delta > EPS) {
         slope = -fa / (fb - fa);   // interpolation slope for dependent variable t

         //"reasonable" interpolation slopes
         if(slope > 0.99) {
            slope = 0.99;
         }

         if(slope < 0.01) {
            slope = 0.01;
         }
         // its pretty harmless computationally

         // tc is the estimate for t0
         tc = ta + slope * (tb - ta);
         fc = 0.;
         for(i = 1; i < lineq_dim; i++) {
            tau = GetCsITau(i, par);
            fc += lineq_solution[i] * exp(-tc / tau);
         }

         // really should have this, just to be safe
         if(fc == 0) {
            return tc;
         }

         if(fc > 0) {
            tb = tc;
            fb = fc;
         } else {
            ta = tc;
            fa = fc;
         }
         delta = std::abs(fc);
      }
   } else {
      return -1;
   }

   // set wpar->t0 here
   wpar->t0 = tc;

   return tc;
}

void TPulseAnalyzer::GetQuickPara()
{
   if(!IsSet()) return;
   if(cWpar) delete cWpar;
   cWpar                 = new WaveFormPar;
   cWpar->baseline_range = T0RANGE;   // default only 8 samples! but can be increased with a multiplier in TPulseAnalyzer constructor
   cWpar->t90_flag       = 0;
   cWpar->t50_flag       = 0;
   cWpar->t10_flag       = 0;
   cWpar->mflag          = 0;
   cWpar->bflag          = 0;
   cWpar->t0             = 0;
   cWpar->baselinefin    = 0;

   get_baseline();   // Takes a small sample baseline
   get_tmax();       // Does a filtered max search

   if(!(cWpar->mflag && cWpar->bflag)) return;

   if(cWpar->tmax > cN) cWpar->tmax = cN - 1;

   double amp = cWpar->max - cWpar->baseline;
   double y9  = cWpar->baseline + amp * .9;
   double y5  = cWpar->baseline + amp * .5;
   double y1  = cWpar->baseline + amp * .1;

   for(int t = cWpar->tmax; t > 0; t--) {
      if(cWavebuffer[t] < y5) {
         cWpar->t50_flag = 1;
         cWpar->t50      = t + 0.5;
         break;
      }
   }

   if(!cWpar->t50_flag) return;

   for(int t = cWpar->t50; t < cWpar->tmax; t++) {
      if(cWavebuffer[t] > y9) {
         cWpar->t90_flag = 1;
         cWpar->t90      = t - 0.5;
         break;
      }
   }

   for(int t = cWpar->t50; t > 0; t--) {
      if(cWavebuffer[t] < y1) {
         cWpar->t10_flag = 1;
         cWpar->t10      = t + 0.5;
         break;
      }
   }

   if(!cWpar->t10_flag) { return; }

   double t0 = cWpar->t50 - ((cWpar->t50 - cWpar->t10) * 1.2);
   if(cWpar->t90_flag) {
      t0 += cWpar->t90 - ((cWpar->t90 - cWpar->t10) * 1.125);
      t0 *= 0.5;
   }
   if(t0 < 0) { t0 = 0; }

   // 	std::cout<<std::endl<<t0<<std::flush;
   cWpar->t0 = t0;
   get_baseline_fin();   // baseline all the way up to t0
}

bool TPulseAnalyzer::SiliShapePrepare(double tauDecay, double tauRise)
{
   if(IsSet()) {
      // double t0=fit_newT0();//fits the T0 in the SFU way with my added bit at the end for a nice baseline calc
      // double t0=cWpar->t0;
      // int exclusion=t0+3;
      // 	if(t0<1){//if the fit_newT0() failed
      // 		exclusion=10;
      // 		if(abs(baseline-cWavebuffer[0])>100)baseline=cWavebuffer[0];
      // 	}

      // New simplified guesses because fit_newT0 was taking 1000% longer
      GetQuickPara();
      cWpar->amplitude = 0;
      cWpar->tauDecay  = tauDecay;
      cWpar->tauRise   = tauRise;
      cWpar->bflag     = 0;   // baseline
      cWpar->baseamp   = 0;
      cWpar->basefreq  = 0;
      cWpar->basephase = 0;
      cWpar->osciflag  = 0;

      if(!(cWpar->t10_flag)) { return 0; }

      // GetQuickPara() Returns values that are spurious if the baseline is missing or <<T0RANGE
      if(cWpar->t0 < cWpar->baseline_range) {                                           // Is there is no clear baseline baseline
         if(!(cWpar->baselineStDevfin / (cWpar->max - cWpar->baselinefin) < 0.035)) {   // Strict (previously 0.05) limit determined from data
            return false;
         }
      }
      cWpar->bflag = 1;
      return true;
   }
   return false;
}

bool TPulseAnalyzer::GetSiliShape(double tauDecay, double tauRise)
{
   if(IsSet()) {

      if(!SiliShapePrepare(tauDecay, tauRise)) { return false; }

      int    exclusion = cWpar->t10;
      double baseline  = cWpar->baselinefin;

      /**************************************************************************
      // Parametes for this function
      //fShpar->t0 t0         (time where signal starts, calculated)
      //fShpar->tau[0]) decay     (provided)
      //fShpar->tau[1]) rise      (provided)
      //fShpar->tau[2]) slow      (not used)
      //fShpar->tau[3]) diode     (not used)
      //fShpar->am[0]) baseline (provided)
      //fShpar->am[1]) fast     (amplitude, calculated )
      //fShpar->am[2]) slow     (not used)
      //fShpar->am[3]) diode    (not used)

      linearized chi square fit is Mu = v where M is a data matrix
      u, v are vectors; u is the parameter vector (solution)
      note that in this formulation, chisq_min = y_i^2-sum(u_iv_i)
      **************************************************************************/

      // cout<<baseline<<"  "<<exclusion<<endl ;// ResetShapeAmplitudes();/ fShpar->am[0] = baseline ;

      lineq_dim = 2;
      memset(lineq_matrix, 0, sizeof(lineq_matrix));
      memset(lineq_vector, 0, sizeof(lineq_vector));
      memset(lineq_solution, 0, sizeof(lineq_solution));

      if(exclusion >= cN) { return false; }
      if(lineq_dim >= cN) { return false; }

      // setting  M[0,0] V[0] V[1]
      for(int j = exclusion; j < cN; j++) {
         // vector
         if((cWavebuffer[j] - baseline) < 0) {
            exclusion++;
            continue;
         }   // this is crucial for oscillations
         // if (j%10==0) cout<<j<<" "<<  cWavebuffer[j]<<" "<<cWavebuffer[j]-baseline<<endl ;
         double signal = log(cWavebuffer[j] - baseline) + j / tauDecay;   // sum of Y_i where  Y_i = (y_i - baseline)*exp(-t_i/tauDecay)
         lineq_vector[0] += exp(signal);
         lineq_vector[1] -= exp(signal - j / tauRise);   // sum of Y_i*X_i
         // Matrix
         lineq_matrix[0][0] += 1;
      }

      // create matrix for linearized fit
      // setting elements M[0,1] M[1,0] M[1,1]
      long double sum;
      sum = -static_cast<double>(exclusion) / tauRise + log(1. - exp(-(static_cast<double>(cN - exclusion)) / tauRise));
      sum -= log(1. - exp(-1. / tauRise));   // finishing the geometric sequence sum
      lineq_matrix[1][0] = -exp(sum);
      lineq_matrix[0][1] = -exp(sum);

      double tauRise_2 = tauRise / 2.;
      sum              = -static_cast<double>(exclusion) / tauRise_2 + log(1. - exp(-(static_cast<double>(cN - exclusion)) / tauRise_2));
      sum -= log(1. - exp(-1. / tauRise_2));
      lineq_matrix[1][1] = exp(sum);

      // cout<<lineq_matrix[0][0]<<"  "<<lineq_matrix[0][1]<<" ---------- "<<lineq_vector[0] <<endl ;
      // cout<<lineq_matrix[1][0]<<"  "<<lineq_matrix[1][1]<<" ---------- "<<lineq_vector[1] <<endl ;

      // solve the matrix equation Mu = v -> u = M^(-1)v where M^(-1) is the inverse
      // of M. note this has no solution if M is not invertable!

      // error if the matrix cannot be inverted
      if(solve_lin_eq() == 0) {
         return false;
      } else {   // else calculate amplitudes
         // calculate amplitudes
         double beta  = lineq_solution[0];
         double alpha = lineq_solution[1];

         double dom = exp(((log(alpha) - log(beta)) * tauRise) / tauDecay);
         if(dom > 0 || dom < 0) { cWpar->amplitude = beta / dom; }

         double tt = (log(alpha) - log(beta)) * tauRise;
         if(tt > 0) { cWpar->t0 = tt; }
      }
      return true;
   }
   return false;
}

// Significantly slower and should only be used in non-sorting analysis of poor waveform
// Needs initial estimates even if fitting those parameters
// Setting basefreq>0 opens a very experimental/bad mode
bool TPulseAnalyzer::GetSiliShapeTF1(double tauDecay, double tauRise, double baseline, double basefreq)
{
   TGraph* h = GetWaveGraph();
   if(h) {   // Graph better than hist for stats and simplicity

      SiliShapePrepare(tauDecay, tauRise);
      TF1 g = Getsilifit();

      // 	g.SetParameter(0,cWpar->t0);
      // 	g.SetParameter(1,cWpar->tauDecay);
      // 	g.SetParameter(2,cWpar->tauRise);
      // 	g.SetParameter(3,cWpar->baselinefin);
      // 	g.SetParameter(4,cWpar->amplitude);

      // Currently constrained for positive waveforms
      double r = cWpar->max - cWavebuffer[0];
      g.SetParameter(4, r * 1.05);

      if(cWpar->bflag) {   // Have reasonable T0 & base, fit the shape
         g.SetParLimits(0, cWpar->t0 * 0.5, cWpar->t0 * 1.5);
         g.SetParLimits(1, tauDecay * 0.3, tauDecay * 3.0);
         g.SetParLimits(2, tauRise * 0.3, tauRise * 1.5);
         g.FixParameter(3, cWpar->baselinefin);
         g.SetParLimits(4, r * 0.5, r * 2.0);
      } else {   // Have no T0 or base, FIX the shape
         g.SetParLimits(0, -cN, cWpar->baseline_range);
         g.FixParameter(1, tauDecay);
         g.FixParameter(2, tauRise);
         g.FixParameter(3, baseline);
         // 		g.SetParameter(3,baseline);
         // 		g.SetParLimits(3,baseline-300,baseline+300);
         g.SetParLimits(4, r * 0.1, r * 10.0);
      }

      if(basefreq > 0) {
         cWpar->osciflag = 1;
         g.ReleaseParameter(5);
         g.ReleaseParameter(6);
         g.ReleaseParameter(7);
         g.SetParameter(5, basefreq);
         g.SetParameter(6, 0.5);
         g.SetParameter(7, r * 0.1);
         g.SetParLimits(5, basefreq * 0.5, basefreq * 2.0);
         g.SetParLimits(6, 0, 1);
         g.SetParLimits(7, 0, r * 10.0);
      }

      int res = h->Fit(&g, "QN");
      delete h;

      if(!res) {
         cWpar->t0          = g.GetParameter(0);
         cWpar->tauDecay    = g.GetParameter(1);
         cWpar->tauRise     = g.GetParameter(2);
         cWpar->baselinefin = g.GetParameter(3);
         cWpar->amplitude   = g.GetParameter(4);
         cWpar->basefreq    = g.GetParameter(5);
         cWpar->basephase   = g.GetParameter(6);
         cWpar->baseamp     = g.GetParameter(7);

         return true;
      }
   }
   return false;
}

double TPulseAnalyzer::SiLiFitFunction(double* i, double* p)
{
   // p[0]-p[2] are t0, RC, Tau
   // p[3]-p[4] are baseline, A0
   // p[5]-p[7] are osci freq,phase,amp

   double x = i[0] - p[0];

   double s = p[3];
   if(x > 0) s += p[4] * (1 - exp(-x / p[2])) * exp(-x / p[1]);
   if(p[7] > 0) s += p[7] * sin((p[6] + i[0] / p[5]) * 2 * TMath::Pi());

   return s;
}

TF1 TPulseAnalyzer::Getsilifit()
{
   if(set && cWpar) {
      std::stringstream ss;
      ss << "Fit" << nameiter;
      ++nameiter;
      TF1 g(ss.str().c_str(), SiLiFitFunction, 0, cN, 8);

      g.SetParameter(0, cWpar->t0);
      g.SetParameter(1, cWpar->tauDecay);
      g.SetParameter(2, cWpar->tauRise);
      g.SetParameter(3, cWpar->baselinefin);
      g.SetParameter(4, cWpar->amplitude);
      g.FixParameter(5, cWpar->basefreq);
      g.FixParameter(6, cWpar->basephase);
      g.FixParameter(7, cWpar->baseamp);

      g.SetLineColor(kRed);

      return g;
   }

   return TF1();
}

double TPulseAnalyzer::GetsiliSmirnov()
{
   double Smirnov = 0;
   if(set && cWpar) {
      double gsum = 0, wsum = 0;
      TF1    g = Getsilifit();
      for(Int_t i = 0; i < cN; i++) {
         wsum += abs(cWavebuffer[i]);
         gsum += abs(g.Eval(i + 0.5));
         Smirnov += abs(wsum - gsum);
      }
   }
   return (Smirnov);
}

void TPulseAnalyzer::Drawsilifit()
{
   if(!set) return;
   DrawWave();
   if(cWpar) {
      Getsilifit().DrawCopy("same");
      std::cout << "t0:\t" << cWpar->t0 << ", A:\t" << cWpar->amplitude << std::endl;
   }
   return;
}

void TPulseAnalyzer::DrawWave()
{
   TH1I* h = GetWaveHist();
   if(h != nullptr) {
      h->DrawCopy();
      delete h;
   }
}

int   TPulseAnalyzer::nameiter = 0;
TH1I* TPulseAnalyzer::GetWaveHist()
{
   if(cN == 0 || !set) {
      return nullptr;
   }
   std::stringstream ss;
   ss << "WaveformHist" << nameiter;
   ++nameiter;   // Avoid naming conflicts with TNamed
   TH1I* h = new TH1I(ss.str().c_str(), ss.str().c_str(), cN, -0.5,
                      cN - 0.5);   // midpoint should be the value, else time is off
   for(Int_t i = 0; i < cN; i++) {
      h->SetBinContent(i + 1, cWavebuffer[i]);
   }
   return h;
}

TGraph* TPulseAnalyzer::GetWaveGraph()
{
   if(cN == 0 || !set) {
      return nullptr;
   }
   auto* g = new TGraph();
   for(Int_t i = 0; i < cN; i++) {
      g->SetPoint(i, i, cWavebuffer[i]);
   }
   return g;
}

void TPulseAnalyzer::DrawRFFit()
{

   if(cN == 0 || !set) {
      return;
   }

   DrawWave();

   if(spar != nullptr) {
      TF1 f("fit", "[2] + [0]*sin(6.283185307 * (x - [1])/(2*8.48409))", 0, cN);

      f.SetParameter(0, spar->A);
      f.SetParameter(1, spar->t0);
      f.SetParameter(2, spar->C);

      f.DrawCopy("same");

      std::cout << "t0:\t" << spar->t0 << ", A:\t" << spar->A << ", O:\t" << spar->C << std::endl;
   }
   return;
}

void TPulseAnalyzer::DrawT0fit()
{

   if(cN == 0 || !set) {
      return;
   }

   DrawWave();

   if(cWpar != nullptr) {
      TF1 g("fit", "[0]+[1]*x", 0, cWpar->temax);
      TF1 f("fit", "[0]+[1]*x+[2]*x*x", cWpar->temin, cWpar->thigh);

      g.SetParameter(0, cWpar->b0);
      g.SetParameter(1, cWpar->b1);
      g.SetLineColor(kRed);

      f.SetParameter(0, cWpar->s0);
      f.SetParameter(1, cWpar->s1);
      f.SetParameter(2, cWpar->s2);
      f.SetLineColor(8);

      f.DrawCopy("same");
      g.DrawCopy("same");

      std::cout << "t0:\t" << cWpar->t0 << std::endl;
   }
   return;
}

void TPulseAnalyzer::DrawCsIExclusion()
{

   if(cN == 0 || !set) {
      return;
   }

   DrawWave();
   if(cWpar != nullptr) {
      TF1 f("base", "[0]", 0, cWpar->baseline_range);
      TF1 g("basemin", "[0]", cWpar->temin, cWpar->temax);
      TF1 h("basemax", "[0]", cWpar->temin, cWpar->temax);
      TF1 r("risetime", "[0]*x+[1]", cWpar->temin, cWpar->temax + 3 * FILTER);

      std::cout << "Baseline:\t" << cWpar->baseline << std::endl;
      std::cout << "Zero crossing:\t" << cWpar->t0 << std::endl;

      f.SetParameter(0, cWpar->baseline);
      f.SetLineColor(kGreen);

      g.SetParameter(0, cWpar->baselineMin);
      g.SetLineColor(kBlue);

      h.SetParameter(0, cWpar->baselineMax);
      h.SetLineColor(kBlack);

      r.SetParameter(0, cWpar->afit);
      r.SetParameter(1, cWpar->bfit);
      r.SetLineColor(kRed);

      f.DrawCopy("same");
      g.DrawCopy("same");
      h.DrawCopy("same");
      r.DrawCopy("same");
   }
   return;
}

void TPulseAnalyzer::DrawCsIFit()
{

   if(cN == 0 || !set || !CsIIsSet()) {
      return;
   }

   DrawWave();
   if(cWpar != nullptr) {
      TF1 shape("shape", TGRSIFunctions::CsIFitFunction, 0, cN, 9);

      shape.SetParameter(0, shpar->t[0]);
      shape.SetParameter(1, shpar->t[1]);
      shape.SetParameter(2, shpar->t[2]);
      shape.SetParameter(3, shpar->t[3]);
      shape.SetParameter(4, shpar->t[4]);
      shape.SetParameter(5, shpar->am[0]);
      shape.SetParameter(6, shpar->am[2]);
      shape.SetParameter(7, shpar->am[3]);
      shape.SetParameter(8, shpar->am[4]);
      shape.SetLineColor(kRed);

      std::cout << "t0:\t" << shpar->t[0] << ",\ttRC:\t" << shpar->t[1] << ",\ttF:\t" << shpar->t[2] << ",\ttS:\t" << shpar->t[3] << ",\tTGamma:\t" << shpar->t[4] << std::endl;
      std::cout << "Baseline:\t" << shpar->am[0] << ",\tFast:\t" << shpar->am[2] << ",\tSlow:\t" << shpar->am[3] << ",\tGamma:\t" << shpar->am[4] << std::endl;

      shape.DrawCopy("same");
   }
   return;
}

/*======================================================*/
void TPulseAnalyzer::print_WavePar()
{
   std::cout << "== Currently established waveform parameters ============" << std::endl;
   std::cout << "baseline         : " << std::setw(10) << cWpar->baseline << std::endl;
   std::cout << "baseline st. dev.: " << std::setw(10) << cWpar->baselineStDev << std::endl;
   std::cout << "max              : " << std::setw(10) << cWpar->max << std::endl;
   std::cout << "tmax             : " << std::setw(10) << cWpar->tmax << std::endl;
   std::cout << "temin            : " << std::setw(10) << static_cast<double>(cWpar->temin) << std::endl;
   std::cout << "temax            : " << std::setw(10) << static_cast<double>(cWpar->temax) << std::endl;
   std::cout << "t0               : " << std::setw(10) << cWpar->t0 << std::endl;
}
