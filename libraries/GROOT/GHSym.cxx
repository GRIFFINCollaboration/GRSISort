#include "GHSym.h"

#include "TROOT.h"
#include "THashList.h"
#include "THLimitsFinder.h"
#include "TVirtualHistPainter.h"
#include "TObjString.h"
#include "TVirtualPad.h"
#include "TMath.h"
#include "TPad.h"
#include "TF1.h"
#include "TF2.h"
#include "TRandom.h"
#include "TClass.h"

#include <iostream>

// Internal exceptions for the CheckConsistency method
class DifferentDimension : public std::exception {
};
class DifferentNumberOfBins : public std::exception {
};
class DifferentAxisLimits : public std::exception {
};
class DifferentBinLimits : public std::exception {
};
class DifferentLabels : public std::exception {
};

GHSym::GHSym()
   : fTsumwy(0), fTsumwy2(0), fTsumwxy(0), fMatrix(nullptr)
{
	fDimension = 2; 
}

// we have to repeat the code from the default constructor here, because calling the TH1 constructor and the GHSym
// default constructor gives an error
GHSym::GHSym(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : TH1(name, title, nbins, low, up), fTsumwy(0), fTsumwy2(0), fTsumwxy(0), fMatrix(nullptr)
{
	fDimension = 2; 
   fYaxis.Set(nbins, low, up);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)/2 cells
   fNcells = (fNcells * (nbins + 3)) / 2;
}

GHSym::GHSym(const char* name, const char* title, Int_t nbins, const Double_t* bins)
	: TH1(name, title, nbins, bins), fTsumwy(0), fTsumwy2(0), fTsumwxy(0), fMatrix(nullptr)
{
	fDimension = 2; 
   fYaxis.Set(nbins, bins);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)/2 cells
   fNcells = (fNcells * (nbins + 3)) / 2;
}

GHSym::GHSym(const char* name, const char* title, Int_t nbins, const Float_t* bins)
	: TH1(name, title, nbins, bins), fTsumwy(0), fTsumwy2(0), fTsumwxy(0), fMatrix(nullptr)
{
	fDimension = 2; 
   fYaxis.Set(nbins, bins);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)/2 cells
   fNcells = (fNcells * (nbins + 3)) / 2;
}

GHSym::GHSym(const GHSym& rhs) : TH1()
{
   rhs.Copy(*this);
}

GHSym::~GHSym() = default;

Int_t GHSym::BufferEmpty(Int_t action)
{
   /// Fill histogram with all entries in the buffer.
   /// action = -1 histogram is reset and refilled from the buffer (called by THistPainter::Paint)
   /// action =  0 histogram is filled from the buffer
   /// action =  1 histogram is filled and buffer is deleted
   ///             The buffer is automatically deleted when the number of entries
   ///             in the buffer is greater than the number of entries in the histogram
   if(fBuffer == nullptr) {
      return 0;
   }

   auto nbEntries = static_cast<Int_t>(fBuffer[0]);
   if(nbEntries == 0) {
      return 0;
   }
   if(nbEntries < 0 && action == 0) {
      return 0;   // histogram has been already filled from the buffer
   }
   Double_t* buffer = fBuffer;
   if(nbEntries < 0) {
      nbEntries = -nbEntries;
      fBuffer   = nullptr;
      Reset("ICES");
      fBuffer = buffer;
   }

   if(CanExtendAllAxes() || fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin()) {
      // find min, max of entries in buffer
      // for the symmetric matrix x- and y-range are the same
      Double_t min = fBuffer[2];
      Double_t max = min;
      if(fBuffer[3] < min) {
         min = fBuffer[3];
      }
      if(fBuffer[3] > max) {
         max = fBuffer[3];
      }
      for(Int_t i = 1; i < nbEntries; ++i) {
         Double_t x = fBuffer[3 * i + 2];
         if(x < min) {
            min = x;
         }
         if(x > max) {
            max = x;
         }
         Double_t y = fBuffer[3 * i + 3];
         if(y < min) {
            min = y;
         }
         if(y > max) {
            max = y;
         }
      }
      if(fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin()) {
         THLimitsFinder::GetLimitsFinder()->FindGoodLimits(this, min, max, min, max);
      } else {
         fBuffer     = nullptr;
         Int_t keep  = fBufferSize;
         fBufferSize = 0;
         if(min < fXaxis.GetXmin()) {
            RebinAxis(min, &fXaxis);
         }
         if(max >= fXaxis.GetXmax()) {
            RebinAxis(max, &fXaxis);
         }
         if(min < fYaxis.GetXmin()) {
            RebinAxis(min, &fYaxis);
         }
         if(max >= fYaxis.GetXmax()) {
            RebinAxis(max, &fYaxis);
         }
         fBuffer     = buffer;
         fBufferSize = keep;
      }
   }

   fBuffer = nullptr;
   for(Int_t i = 0; i < nbEntries; ++i) {
      Fill(buffer[3 * i + 2], buffer[3 * i + 3], buffer[3 * i + 1]);
   }
   fBuffer = buffer;

   if(action > 0) {
      delete[] fBuffer;
      fBuffer     = nullptr;
      fBufferSize = 0;
   } else {
      if(nbEntries == static_cast<Int_t>(fEntries)) {
         fBuffer[0] = -nbEntries;
      } else {
         fBuffer[0] = 0;
      }
   }
   return nbEntries;
}

Int_t GHSym::BufferFill(Double_t x, Double_t y, Double_t w)
{
   if(fBuffer == nullptr) {
      return -3;
   }

   auto nbEntries = static_cast<Int_t>(fBuffer[0]);
   if(nbEntries < 0) {
      nbEntries  = -nbEntries;
      fBuffer[0] = nbEntries;
      if(fEntries > 0) {
         Double_t* buffer = fBuffer;
         fBuffer          = nullptr;
         Reset("ICES");
         fBuffer = buffer;
      }
   }
   if(3 * nbEntries + 3 >= fBufferSize) {
      BufferEmpty(1);
      return Fill(x, y, w);
   }
   fBuffer[3 * nbEntries + 1] = w;
   fBuffer[3 * nbEntries + 2] = x;
   fBuffer[3 * nbEntries + 3] = y;
   fBuffer[0] += 1;

   return -3;
}

void GHSym::Copy(TObject& obj) const
{
   // Copy.

   TH1::Copy(obj);
   static_cast<GHSym&>(obj).fTsumwy  = fTsumwy;
   static_cast<GHSym&>(obj).fTsumwy2 = fTsumwy2;
   static_cast<GHSym&>(obj).fTsumwxy = fTsumwxy;
   static_cast<GHSym&>(obj).fMatrix  = nullptr;
}

Double_t GHSym::DoIntegral(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Double_t& error, Option_t* option, Bool_t doError) const
{
   // internal function compute integral and optionally the error  between the limits
   // specified by the bin number values working for all histograms (1D, 2D and 3D)

   Int_t nbinsx = GetNbinsX();
   if(binx1 < 0) {
      binx1 = 0;
   }
   if(binx2 > nbinsx + 1 || binx2 < binx1) {
      binx2 = nbinsx + 1;
   }
   if(GetDimension() > 1) {
      Int_t nbinsy = GetNbinsY();
      if(biny1 < 0) {
         biny1 = 0;
      }
      if(biny2 > nbinsy + 1 || biny2 < biny1) {
         biny2 = nbinsy + 1;
      }
   } else {
      biny1 = 0;
      biny2 = 0;
   }

   //   - Loop on bins in specified range
   TString opt = option;
   opt.ToLower();
   Bool_t width = kFALSE;
   if(opt.Contains("width")) {
      width = kTRUE;
   }

   Double_t dx       = 1.;
   Double_t dy       = 1.;
   Double_t integral = 0;
   Double_t igerr2   = 0;
   for(Int_t binx = binx1; binx <= binx2; ++binx) {
      if(width) {
         dx = fXaxis.GetBinWidth(binx);
      }
      for(Int_t biny = biny1; biny <= biny2; ++biny) {
         if(width) {
            dy = fYaxis.GetBinWidth(biny);
         }
         Int_t bin = GetBin(binx, biny);
         if(width) {
            integral += GetBinContent(bin) * dx * dy;
         } else {
            integral += GetBinContent(bin);
         }
         if(doError) {
            if(width) {
               igerr2 += GetBinError(bin) * GetBinError(bin) * dx * dx * dy * dy;
            } else {
               igerr2 += GetBinError(bin) * GetBinError(bin);
            }
         }
      }
   }

   if(doError) {
      error = TMath::Sqrt(igerr2);
   }
   return integral;
}

Int_t GHSym::Fill(Double_t)
{
   // Invalid Fill method
   Error("Fill", "Invalid signature - do nothing");
   return -1;
}

Int_t GHSym::Fill(Double_t x, Double_t y)
{
   /// Increment cell defined by x,y by 1.
   if(fBuffer != nullptr) {
      return BufferFill(x, y, 1);
   }

   Int_t binx = 0;
	Int_t biny = 0;
   fEntries++;
   if(y <= x) {
      binx = fXaxis.FindBin(x);
      biny = fYaxis.FindBin(y);
   } else {
      binx = fXaxis.FindBin(y);
      biny = fYaxis.FindBin(x);
   }
   if(binx < 0 || biny < 0) {
      return -1;
   }
   Int_t bin = biny * (2 * fXaxis.GetNbins() - biny + 3) / 2 + binx;
   AddBinContent(bin);
   if(fSumw2.fN != 0) {
      ++fSumw2.fArray[bin];
   }
   if(binx == 0 || binx > fXaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   if(biny == 0 || biny > fYaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   // not sure if these summed weights are calculated correct
   // as of now this is the method used in TH2
   ++fTsumw;
   ++fTsumw2;
   fTsumwx += x;
   fTsumwx2 += x * x;
   fTsumwy += y;
   fTsumwy2 += y * y;
   fTsumwxy += x * y;
   return bin;
}

Int_t GHSym::Fill(Double_t x, Double_t y, Double_t w)
{
   /// Increment cell defined by x,y by 1.
   if(fBuffer != nullptr) {
      return BufferFill(x, y, 1);
   }

   Int_t binx = 0;
	Int_t biny = 0;
   fEntries++;
   if(y <= x) {
      binx = fXaxis.FindBin(x);
      biny = fYaxis.FindBin(y);
   } else {
      binx = fXaxis.FindBin(y);
      biny = fYaxis.FindBin(x);
   }
   if(binx < 0 || biny < 0) {
      return -1;
   }
   Int_t bin = biny * (2 * fXaxis.GetNbins() - biny + 3) / 2 + binx;
   AddBinContent(bin, w);
   if(fSumw2.fN != 0) {
      fSumw2.fArray[bin] += w * w;
   }
   if(binx == 0 || binx > fXaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   if(biny == 0 || biny > fYaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   // not sure if these summed weights are calculated correct
   // as of now this is the method used in TH2
   fTsumw += w;
   fTsumw2 += w * w;
   fTsumwx += w * x;
   fTsumwx2 += w * x * x;
   fTsumwy += w * y;
   fTsumwy2 += w * y * y;
   fTsumwxy += w * x * y;
   return bin;
}

Int_t GHSym::Fill(const char* namex, const char* namey, Double_t w)
{
   // Increment cell defined by namex,namey by a weight w
   //
   // if x or/and y is less than the low-edge of the corresponding axis first bin,
   //   the Underflow cell is incremented.
   // if x or/and y is greater than the upper edge of corresponding axis last bin,
   //   the Overflow cell is incremented.
   //
   // If the storage of the sum of squares of weights has been triggered,
   // via the function Sumw2, then the sum of the squares of weights is incremented
   // by w^2 in the cell corresponding to x,y.
   //

   Int_t binx = 0;
	Int_t biny = 0;
   fEntries++;
   binx = fXaxis.FindBin(namex);
   biny = fYaxis.FindBin(namey);
   if(binx < 0 || biny < 0) {
      return -1;
   }
   if(biny >= binx) {
		std::swap(binx, biny);
   }
   Int_t bin = biny * (2 * fXaxis.GetNbins() - biny + 3) / 2 + binx;
   AddBinContent(bin, w);
   if(fSumw2.fN != 0) {
      fSumw2.fArray[bin] += w * w;
   }
   if(binx == 0 || binx > fXaxis.GetNbins()) {
      return -1;
   }
   if(biny == 0 || biny > fYaxis.GetNbins()) {
      return -1;
   }
   Double_t x = fXaxis.GetBinCenter(binx);
   Double_t y = fYaxis.GetBinCenter(biny);
   fTsumw += w;
   fTsumw2 += w * w;
   fTsumwx += w * x;
   fTsumwx2 += w * x * x;
   fTsumwy += w * y;
   fTsumwy2 += w * y * y;
   fTsumwxy += w * x * y;
   return bin;
}

void GHSym::FillN(Int_t ntimes, const Double_t* x, const Double_t* y, const Double_t* w, Int_t stride)
{
   //*-*-*-*-*-*-*Fill a 2-D histogram with an array of values and weights*-*-*-*
   //*-*          ========================================================
   //*-*
   //*-* ntimes:  number of entries in arrays x and w (array size must be ntimes*stride)
   //*-* x:       array of x values to be histogrammed
   //*-* y:       array of y values to be histogrammed
   //*-* w:       array of weights
   //*-* stride:  step size through arrays x, y and w
   //*-*
   //*-* If the storage of the sum of squares of weights has been triggered,
   //*-* via the function Sumw2, then the sum of the squares of weights is incremented
   //*-* by w[i]^2 in the cell corresponding to x[i],y[i].
   //*-* if w is NULL each entry is assumed a weight=1
   //*-*
   //*-* NB: function only valid for a TH2x object
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*
   ntimes *= stride;
   for(int i = 0; i < ntimes; i += stride) {
      if(w != nullptr) {
         Fill(x[i], y[i], w[i]);
      } else {
         Fill(x[i], y[i]);
      }
   }
}

void GHSym::FillRandom(const char* fname, Int_t ntimes, TRandom* rng)
{
   //*-*-*-*-*-*-*Fill histogram following distribution in function fname*-*-*-*
   //*-*          =======================================================
   //*-*
   //*-*   The distribution contained in the function fname (TF2) is integrated
   //*-*   over the channel contents.
   //*-*   It is normalized to 1.
   //*-*   Getting one random number implies:
   //*-*     - Generating a random number between 0 and 1 (say r1)
   //*-*     - Look in which bin in the normalized integral r1 corresponds to
   //*-*     - Fill histogram channel
   //*-*   ntimes random numbers are generated
   //*-*
   //*-*  One can also call TF2::GetRandom2 to get a random variate from a function.
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   //*-*- Search for fname in the list of ROOT defined functions
   TObject* fobj = gROOT->GetFunction(fname);
   if(fobj == nullptr) {
      Error("FillRandom", "Unknown function: %s", fname);
      return;
   }
   TF2* f1 = static_cast<TF2*>(fobj);
   if(f1 == nullptr) {
      Error("FillRandom", "Function: %s is not a TF2", fname);
      return;
   }

   //*-*- Allocate temporary space to store the integral and compute integral
   Int_t nbinsx = GetNbinsX();
   Int_t nbinsy = GetNbinsY();
   Int_t nbins  = nbinsx * nbinsy;

   auto* integral = new Double_t[nbins + 1];
   Int_t ibin           = 0;
   integral[ibin] = 0;
   for(Int_t biny = 1; biny <= nbinsy; ++biny) {
      for(Int_t binx = 1; binx <= nbinsx; ++binx) {
         ++ibin;
         Double_t fint  = f1->Integral(fXaxis.GetBinLowEdge(binx), fXaxis.GetBinUpEdge(binx), fYaxis.GetBinLowEdge(biny),
                                       fYaxis.GetBinUpEdge(biny));
         integral[ibin] = integral[ibin - 1] + fint;
      }
   }

   //*-*- Normalize integral to 1
   if(integral[nbins] == 0) {
      delete[] integral;
      Error("FillRandom", "Integral = zero");
      return;
   }
   for(Int_t bin = 1; bin <= nbins; ++bin) {
      integral[bin] /= integral[nbins];
   }

   //*-*--------------Start main loop ntimes
   for(int loop = 0; loop < ntimes; ++loop) {
      Double_t r1   = (rng != nullptr) ? rng->Rndm(loop) : gRandom->Rndm(loop);
      ibin = TMath::BinarySearch(nbins, &integral[0], r1);
      Int_t biny = ibin / nbinsx;
      Int_t binx = 1 + ibin - nbinsx * biny;
      ++biny;
      Fill(fXaxis.GetBinCenter(binx), fYaxis.GetBinCenter(biny));
   }
   delete[] integral;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
void GHSym::FillRandom(TH1* h, Int_t ntimes, TRandom*)
#else
void GHSym::FillRandom(TH1* hist, Int_t ntimes, TRandom* rng)
#endif
{
   //*-*-*-*-*-*-*Fill histogram following distribution in histogram h*-*-*-*
   //*-*          ====================================================
   //*-*
   //*-*   The distribution contained in the histogram h (TH2) is integrated
   //*-*   over the channel contents.
   //*-*   It is normalized to 1.
   //*-*   Getting one random number implies:
   //*-*     - Generating a random number between 0 and 1 (say r1)
   //*-*     - Look in which bin in the normalized integral r1 corresponds to
   //*-*     - Fill histogram channel
   //*-*   ntimes random numbers are generated
   //*-*
   //*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   if(hist == nullptr) {
      Error("FillRandom", "Null histogram");
      return;
   }
   if(fDimension != hist->GetDimension()) {
      Error("FillRandom", "Histograms with different dimensions");
      return;
   }

   if(hist->ComputeIntegral() == 0) {
      return;
   }

   Double_t x = 0.;
	Double_t y = 0.;
   TH2*     h2 = static_cast<TH2*>(hist);
   for(int loop = 0; loop < ntimes; ++loop) {
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
      h2->GetRandom2(x, y);
#else
      h2->GetRandom2(x, y, rng);
#endif
      Fill(x, y);
   }
}

Int_t GHSym::FindFirstBinAbove(Double_t threshold, Int_t axis, Int_t firstBin, Int_t lastBin) const
{
   // find first bin with content > threshold for axis (1=x, 2=y, 3=z)
   // if no bins with content > threshold is found the function returns -1.

   if(axis < 1 || axis > 2) {
      Warning("FindFirstBinAbove", "Invalid axis number : %d, axis x assumed\n", axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
   if(lastBin > firstBin && lastBin < nbinsx) nbinsx = lastBin;
   Int_t nbinsy = fYaxis.GetNbins();
   if(lastBin > firstBin && lastBin < nbinsy) nbinsy = lastBin;
   if(axis == 1) {
      for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
         for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
            if(GetBinContent(binx, biny) > threshold) {
               return binx;
            }
         }
      }
   } else {
      for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            if(GetBinContent(binx, biny) > threshold) {
               return biny;
            }
         }
      }
   }
   return -1;
}

Int_t GHSym::FindLastBinAbove(Double_t threshold, Int_t axis, Int_t firstBin, Int_t lastBin) const
{
   // find last bin with content > threshold for axis (1=x, 2=y, 3=z)
   // if no bins with content > threshold is found the function returns -1.

   if(axis < 1 || axis > 2) {
      Warning("FindLastBinAbove", "Invalid axis number : %d, axis x assumed\n", axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
   if(lastBin > firstBin && lastBin < nbinsx) nbinsx = lastBin;
   Int_t nbinsy = fYaxis.GetNbins();
   if(lastBin > firstBin && lastBin < nbinsy) nbinsy = lastBin;
   if(axis == 1) {
      for(Int_t binx = nbinsx; binx >= firstBin; --binx) {
         for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
            if(GetBinContent(binx, biny) > threshold) {
               return binx;
            }
         }
      }
   } else {
      for(Int_t biny = nbinsy; biny >= firstBin; --biny) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            if(GetBinContent(binx, biny) > threshold) {
               return biny;
            }
         }
      }
   }
   return -1;
}

void GHSym::FitSlices(TF1* f1, Int_t firstbin, Int_t lastbin, Int_t cut, Option_t* option, TObjArray* arr)
{
   // Project slices along X in case of a 2-D histogram, then fit each slice
   // with function f1 and make a histogram for each fit parameter
   // Only bins along Y between firstbin and lastbin are considered.
   // By default (firstbin == 0, lastbin == -1), all bins in y including
   // over- and underflows are taken into account.
   // If f1=0, a gaussian is assumed
   // Before invoking this function, one can set a subrange to be fitted along X
   // via f1->SetRange(xmin,xmax)
   // The argument option (default="QNR") can be used to change the fit options.
   //     "Q"  means Quiet mode
   //     "N"  means do not show the result of the fit
   //     "R"  means fit the function in the specified function range
   //     "G2" merge 2 consecutive bins along X
   //     "G3" merge 3 consecutive bins along X
   //     "G4" merge 4 consecutive bins along X
   //     "G5" merge 5 consecutive bins along X
   //     "S"  sliding merge: merge n consecutive bins along X accordingly to what Gn is given.
   //          It makes sense when used together with a Gn option
   //
   // The generated histograms are returned by adding them to arr, if arr is not NULL.
   // arr's SetOwner() is called, to signal that it is the user's respponsability to
   // delete the histograms, possibly by deleting the arrary.
   //    TObjArray aSlices;
   //    h2->FitSlicesX(func, 0, -1, 0, "QNR", &aSlices);
   // will already delete the histograms once aSlice goes out of scope. aSlices will
   // contain the histogram for the i-th parameter of the fit function at aSlices[i];
   // aSlices[n] (n being the number of parameters) contains the chi2 distribution of
   // the fits.
   //
   // If arr is NULL, the generated histograms are added to the list of objects
   // in the current directory. It is the user's responsability to delete
   // these histograms.
   //
   //  Example: Assume a 2-d histogram h2
   //   Root > h2->FitSlicesX(); produces 4 TH1D histograms
   //          with h2_0 containing parameter 0(Constant) for a Gaus fit
   //                    of each bin in Y projected along X
   //          with h2_1 containing parameter 1(Mean) for a gaus fit
   //          with h2_2 containing parameter 2(RMS)  for a gaus fit
   //          with h2_chi2 containing the chisquare/number of degrees of freedom for a gaus fit
   //
   //   Root > h2->FitSlicesX(0,15,22,10);
   //          same as above, but only for bins 15 to 22 along Y
   //          and only for bins in Y for which the corresponding projection
   //          along X has more than cut bins filled.
   //
   //  NOTE: To access the generated histograms in the current directory, do eg:
   //     TH1D *h2_1 = (TH1D*)gDirectory->Get("h2_1");

   Int_t nbins = fYaxis.GetNbins();
   if(firstbin < 0) {
      firstbin = 0;
   }
   if(lastbin < 0 || lastbin > nbins + 1) {
      lastbin = nbins + 1;
   }
   if(lastbin < firstbin) {
      firstbin = 0;
      lastbin  = nbins + 1;
   }
   TString opt = option;
   opt.ToLower();
   Int_t ngroup = 1;
   if(opt.Contains("g2")) {
      ngroup = 2;
      opt.ReplaceAll("g2", "");
   }
   if(opt.Contains("g3")) {
      ngroup = 3;
      opt.ReplaceAll("g3", "");
   }
   if(opt.Contains("g4")) {
      ngroup = 4;
      opt.ReplaceAll("g4", "");
   }
   if(opt.Contains("g5")) {
      ngroup = 5;
      opt.ReplaceAll("g5", "");
   }

   // implement option S sliding merge for each bin using in conjunction with a given Gn
   Int_t nstep = ngroup;
   if(opt.Contains("s")) {
      nstep = 1;
   }

   // default is to fit with a gaussian
   if(f1 == nullptr) {
      f1 = static_cast<TF1*>(gROOT->GetFunction("gaus"));
      if(f1 == nullptr) {
         f1 = new TF1("gaus", "gaus", fXaxis.GetXmin(), fXaxis.GetXmax());
      } else {
         f1->SetRange(fXaxis.GetXmin(), fXaxis.GetXmax());
      }
   }
   Int_t npar = f1->GetNpar();
   if(npar <= 0) {
      return;
   }
   auto* parsave = new Double_t[npar];
   f1->GetParameters(parsave);

   if(arr != nullptr) {
      arr->SetOwner();
      arr->Expand(npar + 1);
   }

   // Create one histogram for each function parameter
   auto**         hlist = new TH1D*[npar];
   auto*          name  = new char[2000];
   auto*          title = new char[2000];
   const TArrayD* bins  = fYaxis.GetXbins();
   for(Int_t ipar = 0; ipar < npar; ++ipar) {
      snprintf(name, 2000, "%s_%d", GetName(), ipar);
      snprintf(title, 2000, "Fitted value of par[%d]=%s", ipar, f1->GetParName(ipar));
      delete gDirectory->FindObject(name);
      if(bins->fN == 0) {
         hlist[ipar] = new TH1D(name, title, nbins, fYaxis.GetXmin(), fYaxis.GetXmax());
      } else {
         hlist[ipar] = new TH1D(name, title, nbins, bins->fArray);
      }
      hlist[ipar]->GetXaxis()->SetTitle(fYaxis.GetTitle());
      if(arr != nullptr) {
         (*arr)[ipar] = hlist[ipar];
      }
   }
   snprintf(name, 2000, "%s_chi2", GetName());
   delete gDirectory->FindObject(name);
   TH1D* hchi2 = nullptr;
   if(bins->fN == 0) {
      hchi2 = new TH1D(name, "chisquare", nbins, fYaxis.GetXmin(), fYaxis.GetXmax());
   } else {
      hchi2 = new TH1D(name, "chisquare", nbins, bins->fArray);
   }
   hchi2->GetXaxis()->SetTitle(fYaxis.GetTitle());
   if(arr != nullptr) {
      (*arr)[npar] = hchi2;
   }

   // Loop on all bins in Y, generate a projection along X
   // in case of sliding merge nstep=1, i.e. do slices starting for every bin
   // now do not slices case with overflow (makes more sense)
   for(Int_t bin = firstbin; bin + ngroup - 1 <= lastbin; bin += nstep) {
      TH1D* proj = Projection("_temp", bin, bin + ngroup - 1, "e");
      if(proj == nullptr) {
         continue;
      }
      auto nentries = static_cast<Long64_t>(proj->GetEntries());
      if(nentries == 0 || nentries < cut) {
         delete proj;
         continue;
      }
      f1->SetParameters(parsave);
      proj->Fit(f1, opt.Data());
      Int_t npfits = f1->GetNumberFitPoints();
      if(npfits > npar && npfits >= cut) {
         Int_t binOn = bin + ngroup / 2;
         for(Int_t ipar = 0; ipar < npar; ++ipar) {
            hlist[ipar]->Fill(fYaxis.GetBinCenter(binOn), f1->GetParameter(ipar));
            hlist[ipar]->SetBinError(binOn, f1->GetParError(ipar));
         }
         hchi2->Fill(fYaxis.GetBinCenter(binOn), f1->GetChisquare() / (npfits - npar));
      }
      delete proj;
   }
   delete[] parsave;
   delete[] name;
   delete[] title;
   delete[] hlist;
}

Int_t GHSym::GetBin(Int_t binx, Int_t biny, Int_t) const
{
   Int_t nBin = fXaxis.GetNbins() + 2;
   if(binx < 0) {
      binx = 0;
   }
   if(binx >= nBin) {
      binx = nBin - 1;
   }
   if(biny < 0) {
      biny = 0;
   }
   if(biny >= nBin) {
      biny = nBin - 1;
   }
   if(biny <= binx) {
      return binx + biny * (2 * fXaxis.GetNbins() - biny + 3) / 2;
   }
   return biny + binx * (2 * fXaxis.GetNbins() - binx + 3) / 2;
}

Double_t GHSym::GetBinWithContent2(Double_t content, Int_t& binx, Int_t& biny, Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Double_t maxdiff) const
{
   // compute first cell (binx,biny) in the range [firstxbin,lastxbin][firstybin,lastybin] for which
   // diff = abs(cell_content-c) <= maxdiff
   // In case several cells in the specified range with diff=0 are found
   // the first cell found is returned in binx,biny.
   // In case several cells in the specified range satisfy diff <=maxdiff
   // the cell with the smallest difference is returned in binx,biny.
   // In all cases the function returns the smallest difference.
   //
   // NOTE1: if firstxbin < 0, firstxbin is set to 1
   //        if(lastxbin < firstxbin then lastxbin is set to the number of bins in X
   //          ie if firstxbin=1 and lastxbin=0 (default) the search is on all bins in X except
   //          for X's under- and overflow bins.
   //        if firstybin < 0, firstybin is set to 1
   //        if(lastybin < firstybin then lastybin is set to the number of bins in Y
   //          ie if firstybin=1 and lastybin=0 (default) the search is on all bins in Y except
   //          for Y's under- and overflow bins.
   // NOTE2: if maxdiff=0 (default), the first cell with content=c is returned.

   if(fDimension != 2) {
      binx = -1;
      biny = -1;
      Error("GetBinWithContent2", "function is only valid for 2-D histograms");
      return 0;
   }
   if(firstxbin < 0) {
      firstxbin = 1;
   }
   if(lastxbin < firstxbin) {
      lastxbin = fXaxis.GetNbins();
   }
   if(firstybin < 0) {
      firstybin = 1;
   }
   if(lastybin < firstybin) {
      lastybin = fYaxis.GetNbins();
   }
   Double_t curmax = 1e240;
   for(Int_t j = firstybin; j <= lastybin; j++) {
      for(Int_t i = firstxbin; i <= lastxbin; i++) {
         Double_t diff = TMath::Abs(GetBinContent(i, j) - content);
         if(diff <= 0) {
            binx = i;
            biny = j;
            return diff;
         }
         if(diff < curmax && diff <= maxdiff) {
            curmax = diff, binx = i;
            biny = j;
         }
      }
   }
   return curmax;
}

Double_t GHSym::GetCellContent(Int_t binx, Int_t biny) const
{
   return GetBinContent(GetBin(binx, biny));
}

Double_t GHSym::GetCellError(Int_t binx, Int_t biny) const
{
   return GetBinError(GetBin(binx, biny));
}

Double_t GHSym::GetCorrelationFactor(Int_t axis1, Int_t axis2) const
{
   //*-*-*-*-*-*-*-*Return correlation factor between axis1 and axis2*-*-*-*-*
   //*-*            ====================================================
   if(axis1 < 1 || axis2 < 1 || axis1 > 2 || axis2 > 2) {
      Error("GetCorrelationFactor", "Wrong parameters");
      return 0;
   }
   if(axis1 == axis2) {
      return 1;
   }
   Double_t rms1 = GetRMS(axis1);
   if(rms1 == 0) {
      return 0;
   }
   Double_t rms2 = GetRMS(axis2);
   if(rms2 == 0) {
      return 0;
   }
   return GetCovariance(axis1, axis2) / rms1 / rms2;
}

Double_t GHSym::GetCovariance(Int_t axis1, Int_t axis2) const
{
   //*-*-*-*-*-*-*-*Return covariance between axis1 and axis2*-*-*-*-*
   //*-*            ====================================================

   if(axis1 < 1 || axis2 < 1 || axis1 > 2 || axis2 > 2) {
      Error("GetCovariance", "Wrong parameters");
      return 0;
   }
	std::array<Double_t, kNstat> stats;
   GetStats(stats.data());
   Double_t sumw = stats[0];
   // Double_t sumw2  = stats[1];
   Double_t sumwx  = stats[2];
   Double_t sumwx2 = stats[3];
   Double_t sumwy  = stats[4];
   Double_t sumwy2 = stats[5];
   Double_t sumwxy = stats[6];

   if(sumw == 0) {
      return 0;
   }
   if(axis1 == 1 && axis2 == 1) {
      return TMath::Abs(sumwx2 / sumw - sumwx / sumw * sumwx / sumw);
   }
   if(axis1 == 2 && axis2 == 2) {
      return TMath::Abs(sumwy2 / sumw - sumwy / sumw * sumwy / sumw);
   }
   return sumwxy / sumw - sumwx / sumw * sumwy / sumw;
}

void GHSym::GetRandom2(Double_t& x, Double_t& y)
{
   // return 2 random numbers along axis x and y distributed according
   // the cellcontents of a 2-dim histogram
   // return a NaN if the histogram has a bin with negative content

   Int_t    nbinsx = GetNbinsX();
   Int_t    nbinsy = GetNbinsY();
   Int_t    nbins  = nbinsx * nbinsy;
   Double_t integral;
   // compute integral checking that all bins have positive content (see ROOT-5894)
   if(fIntegral != nullptr) {
      if(fIntegral[nbins + 1] != fEntries) {
         integral = ComputeIntegral(true);
      } else {
         integral = fIntegral[nbins];
      }
   } else {
      integral = ComputeIntegral(true);
   }
   if(integral == 0) {
      x = 0;
      y = 0;
      return;
   }
   // case histogram has negative bins
   if(integral == TMath::QuietNaN()) {
      x = TMath::QuietNaN();
      y = TMath::QuietNaN();
      return;
   }

   Double_t r1   = gRandom->Rndm();
   Int_t    ibin = TMath::BinarySearch(nbins, fIntegral, r1);
   Int_t    biny = ibin / nbinsx;
   Int_t    binx = ibin - nbinsx * biny;
   x             = fXaxis.GetBinLowEdge(binx + 1);
   if(r1 > fIntegral[ibin]) {
      x += fXaxis.GetBinWidth(binx + 1) * (r1 - fIntegral[ibin]) / (fIntegral[ibin + 1] - fIntegral[ibin]);
   }
   y = fYaxis.GetBinLowEdge(biny + 1) + fYaxis.GetBinWidth(biny + 1) * gRandom->Rndm();
}

void GHSym::GetStats(Double_t* stats) const
{
   // fill the array stats from the contents of this histogram
   // The array stats must be correctly dimensionned in the calling program.
   // stats[0] = sumw
   // stats[1] = sumw2
   // stats[2] = sumwx
   // stats[3] = sumwx2
   // stats[4] = sumwy
   // stats[5] = sumwy2
   // stats[6] = sumwxy
   //
   // If no axis-subranges are specified (via TAxis::SetRange), the array stats
   // is simply a copy of the statistics quantities computed at filling time.
   // If sub-ranges are specified, the function recomputes these quantities
   // from the bin contents in the current axis ranges.
   //
   //  Note that the mean value/RMS is computed using the bins in the currently
   //  defined ranges (see TAxis::SetRange). By default the ranges include
   //  all bins from 1 to nbins included, excluding underflows and overflows.
   //  To force the underflows and overflows in the computation, one must
   //  call the static function TH1::StatOverflows(kTRUE) before filling
   //  the histogram.

   if(fBuffer != nullptr) {
      const_cast<GHSym*>(this)->BufferEmpty();
   }

   if((fTsumw == 0 && fEntries > 0) || fXaxis.TestBit(TAxis::kAxisRange) || fYaxis.TestBit(TAxis::kAxisRange)) {
      for(Int_t bin = 0; bin < 7; ++bin) {
         stats[bin] = 0;
      }

      Int_t firstBinX = fXaxis.GetFirst();
      Int_t lastBinX  = fXaxis.GetLast();
      Int_t firstBinY = fYaxis.GetFirst();
      Int_t lastBinY  = fYaxis.GetLast();
      // include underflow/overflow if TH1::StatOverflows(kTRUE) in case no range is set on the axis
      if(fgStatOverflows) {
         if(!fXaxis.TestBit(TAxis::kAxisRange)) {
            if(firstBinX == 1) {
               firstBinX = 0;
            }
            if(lastBinX == fXaxis.GetNbins()) {
               lastBinX += 1;
            }
         }
         if(!fYaxis.TestBit(TAxis::kAxisRange)) {
            if(firstBinY == 1) {
               firstBinY = 0;
            }
            if(lastBinY == fYaxis.GetNbins()) {
               lastBinY += 1;
            }
         }
      }
      for(Int_t biny = firstBinY; biny <= lastBinY; ++biny) {
         Double_t y = fYaxis.GetBinCenter(biny);
         for(Int_t binx = firstBinX; binx <= lastBinX; ++binx) {
            Int_t    bin = GetBin(binx, biny);
            Double_t x   = fXaxis.GetBinCenter(binx);
            Double_t w   = GetBinContent(bin);
            Double_t err = TMath::Abs(GetBinError(bin));
            stats[0] += w;
            stats[1] += err * err;
            stats[2] += w * x;
            stats[3] += w * x * x;
            stats[4] += w * y;
            stats[5] += w * y * y;
            stats[6] += w * x * y;
         }
      }
   } else {
      stats[0] = fTsumw;
      stats[1] = fTsumw2;
      stats[2] = fTsumwx;
      stats[3] = fTsumwx2;
      stats[4] = fTsumwy;
      stats[5] = fTsumwy2;
      stats[6] = fTsumwxy;
   }
}

Double_t GHSym::Integral(Option_t* option) const
{
   // Return integral of bin contents. Only bins in the bins range are considered.
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x and in y.

   return Integral(fXaxis.GetFirst(), fXaxis.GetLast(), fYaxis.GetFirst(), fYaxis.GetLast(), option);
}

Double_t GHSym::Integral(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Option_t* option) const
{
   // Return integral of bin contents in range [firstxbin,lastxbin],[firstybin,lastybin]
   // for a 2-D histogram
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x and in y.
   double err = 0;
   return DoIntegral(firstxbin, lastxbin, firstybin, lastybin, err, option);
}

Double_t GHSym::IntegralAndError(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Double_t& error,
                                 Option_t* option) const
{
   // Return integral of bin contents in range [firstxbin,lastxbin],[firstybin,lastybin]
   // for a 2-D histogram. Calculates also the integral error using error propagation
   // from the bin errors assumming that all the bins are uncorrelated.
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x and in y.

   return DoIntegral(firstxbin, lastxbin, firstybin, lastybin, error, option, kTRUE);
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GHSym::Interpolate(Double_t)
#else
Double_t GHSym::Interpolate(Double_t) const
#endif
{
   // illegal for a TH2
   Error("Interpolate", "This function must be called with 2 arguments for a TH2");
   return 0;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GHSym::Interpolate(Double_t x, Double_t y)
#else
Double_t GHSym::Interpolate(Double_t x, Double_t y) const
#endif
{
   // Given a point P(x,y), Interpolate approximates the value via bilinear
   // interpolation based on the four nearest bin centers
   // see Wikipedia, Bilinear Interpolation
   // Andy Mastbaum 10/8/2008
   // vaguely based on R.Raja 6-Sep-2008

   Double_t f  = 0;
   Double_t x1 = 0;
   Double_t x2 = 0;
   Double_t y1 = 0;
   Double_t y2 = 0;
   Double_t dx = 0.;
	Double_t dy = 0.;
   Int_t    bin_x = fXaxis.FindBin(x);
   Int_t    bin_y = fYaxis.FindBin(y);
   if(bin_x < 1 || bin_x > GetNbinsX() || bin_y < 1 || bin_y > GetNbinsY()) {
      Error("Interpolate", "Cannot interpolate outside histogram domain.");
      return 0;
   }
   Int_t quadrant = 0;   // CCW from UR 1,2,3,4
   // which quadrant of the bin (bin_P) are we in?
   dx = fXaxis.GetBinUpEdge(bin_x) - x;
   dy = fYaxis.GetBinUpEdge(bin_y) - y;
   if(dx <= fXaxis.GetBinWidth(bin_x) / 2 && dy <= fYaxis.GetBinWidth(bin_y) / 2) {
      quadrant = 1;   // upper right
   }
   if(dx > fXaxis.GetBinWidth(bin_x) / 2 && dy <= fYaxis.GetBinWidth(bin_y) / 2) {
      quadrant = 2;   // upper left
   }
   if(dx > fXaxis.GetBinWidth(bin_x) / 2 && dy > fYaxis.GetBinWidth(bin_y) / 2) {
      quadrant = 3;   // lower left
   }
   if(dx <= fXaxis.GetBinWidth(bin_x) / 2 && dy > fYaxis.GetBinWidth(bin_y) / 2) {
      quadrant = 4;   // lower right
   }
   switch(quadrant) {
   case 1:
      x1 = fXaxis.GetBinCenter(bin_x);
      y1 = fYaxis.GetBinCenter(bin_y);
      x2 = fXaxis.GetBinCenter(bin_x + 1);
      y2 = fYaxis.GetBinCenter(bin_y + 1);
      break;
   case 2:
      x1 = fXaxis.GetBinCenter(bin_x - 1);
      y1 = fYaxis.GetBinCenter(bin_y);
      x2 = fXaxis.GetBinCenter(bin_x);
      y2 = fYaxis.GetBinCenter(bin_y + 1);
      break;
   case 3:
      x1 = fXaxis.GetBinCenter(bin_x - 1);
      y1 = fYaxis.GetBinCenter(bin_y - 1);
      x2 = fXaxis.GetBinCenter(bin_x);
      y2 = fYaxis.GetBinCenter(bin_y);
      break;
   case 4:
      x1 = fXaxis.GetBinCenter(bin_x);
      y1 = fYaxis.GetBinCenter(bin_y - 1);
      x2 = fXaxis.GetBinCenter(bin_x + 1);
      y2 = fYaxis.GetBinCenter(bin_y);
      break;
   }
   Int_t bin_x1 = fXaxis.FindBin(x1);
   if(bin_x1 < 1) {
      bin_x1 = 1;
   }
   Int_t bin_x2 = fXaxis.FindBin(x2);
   if(bin_x2 > GetNbinsX()) {
      bin_x2 = GetNbinsX();
   }
   Int_t bin_y1 = fYaxis.FindBin(y1);
   if(bin_y1 < 1) {
      bin_y1 = 1;
   }
   Int_t bin_y2 = fYaxis.FindBin(y2);
   if(bin_y2 > GetNbinsY()) {
      bin_y2 = GetNbinsY();
   }
   Int_t    bin_q22 = GetBin(bin_x2, bin_y2);
   Int_t    bin_q12 = GetBin(bin_x1, bin_y2);
   Int_t    bin_q11 = GetBin(bin_x1, bin_y1);
   Int_t    bin_q21 = GetBin(bin_x2, bin_y1);
   Double_t q11     = GetBinContent(bin_q11);
   Double_t q12     = GetBinContent(bin_q12);
   Double_t q21     = GetBinContent(bin_q21);
   Double_t q22     = GetBinContent(bin_q22);
   Double_t d       = 1.0 * (x2 - x1) * (y2 - y1);
   f                = 1.0 * q11 / d * (x2 - x) * (y2 - y) + 1.0 * q21 / d * (x - x1) * (y2 - y) + 1.0 * q12 / d * (x2 - x) * (y - y1) +
       1.0 * q22 / d * (x - x1) * (y - y1);
   return f;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GHSym::Interpolate(Double_t, Double_t, Double_t)
#else
Double_t GHSym::Interpolate(Double_t, Double_t, Double_t) const
#endif
{
   // illegal for a TH2
   Error("Interpolate", "This function must be called with 2 arguments for a TH2");
   return 0;
}

Double_t GHSym::KolmogorovTest(const TH1* h2, Option_t* option) const
{
   //  Statistical test of compatibility in shape between
   //  THIS histogram and h2, using Kolmogorov test.
   //     Default: Ignore under- and overflow bins in comparison
   //
   //     option is a character string to specify options
   //         "U" include Underflows in test
   //         "O" include Overflows
   //         "N" include comparison of normalizations
   //         "D" Put out a line of "Debug" printout
   //         "M" Return the Maximum Kolmogorov distance instead of prob
   //
   //   The returned function value is the probability of test
   //       (much less than one means NOT compatible)
   //
   //   The KS test uses the distance between the pseudo-CDF's obtained
   //   from the histogram. Since in 2D the order for generating the pseudo-CDF is
   //   arbitrary, two pairs of pseudo-CDF are used, one starting from the x axis the
   //   other from the y axis and the maximum distance is the average of the two maximum
   //   distances obtained.
   //
   //  Code adapted by Rene Brun from original HBOOK routine HDIFF

   TString opt = option;
   opt.ToUpper();

   Double_t prb = 0;
   TH1*     h1  = const_cast<TH1*>(static_cast<const TH1*>(this));
   if(h2 == nullptr) {
      return 0;
   }
   TAxis* xaxis1 = h1->GetXaxis();
   auto*  xaxis2 = const_cast<TAxis*>(h2->GetXaxis());
   TAxis* yaxis1 = h1->GetYaxis();
   auto*  yaxis2 = const_cast<TAxis*>(h2->GetYaxis());
   Int_t  ncx1   = xaxis1->GetNbins();
   Int_t  ncx2   = xaxis2->GetNbins();
   Int_t  ncy1   = yaxis1->GetNbins();
   Int_t  ncy2   = yaxis2->GetNbins();

   // Check consistency of dimensions
   if(h1->GetDimension() != 2 || h2->GetDimension() != 2) {
      Error("KolmogorovTest", "Histograms must be 2-D\n");
      return 0;
   }

   // Check consistency in number of channels
   if(ncx1 != ncx2) {
      Error("KolmogorovTest", "Number of channels in X is different, %d and %d\n", ncx1, ncx2);
      return 0;
   }
   if(ncy1 != ncy2) {
      Error("KolmogorovTest", "Number of channels in Y is different, %d and %d\n", ncy1, ncy2);
      return 0;
   }

   // Check consistency in channel edges
   Bool_t   afunc1  = kFALSE;
   Bool_t   afunc2  = kFALSE;
   Double_t difprec = 1e-5;
   Double_t diff1   = TMath::Abs(xaxis1->GetXmin() - xaxis2->GetXmin());
   Double_t diff2   = TMath::Abs(xaxis1->GetXmax() - xaxis2->GetXmax());
   if(diff1 > difprec || diff2 > difprec) {
      Error("KolmogorovTest", "histograms with different binning along X");
      return 0;
   }
   diff1 = TMath::Abs(yaxis1->GetXmin() - yaxis2->GetXmin());
   diff2 = TMath::Abs(yaxis1->GetXmax() - yaxis2->GetXmax());
   if(diff1 > difprec || diff2 > difprec) {
      Error("KolmogorovTest", "histograms with different binning along Y");
      return 0;
   }

   //   Should we include Uflows, Oflows?
   Int_t ibeg = 1;
   Int_t jbeg = 1;
   Int_t iend = ncx1;
   Int_t jend = ncy1;
   if(opt.Contains("U")) {
      ibeg = 0;
      jbeg = 0;
   }
   if(opt.Contains("O")) {
      iend = ncx1 + 1;
      jend = ncy1 + 1;
   }

   Double_t sum1 = 0;
   Double_t sum2 = 0;
   Double_t w1   = 0;
   Double_t w2   = 0;
   for(Int_t i = ibeg; i <= iend; ++i) {
      for(Int_t j = jbeg; j <= jend; ++j) {
         sum1 += h1->GetBinContent(i, j);
         sum2 += h2->GetBinContent(i, j);
         Double_t ew1 = h1->GetBinError(i, j);
         Double_t ew2 = h2->GetBinError(i, j);
         w1 += ew1 * ew1;
         w2 += ew2 * ew2;
      }
   }

   // Check that both scatterplots contain events
   if(sum1 == 0) {
      Error("KolmogorovTest", "Integral is zero for h1=%s\n", h1->GetName());
      return 0;
   }
   if(sum2 == 0) {
      Error("KolmogorovTest", "Integral is zero for h2=%s\n", h2->GetName());
      return 0;
   }
   // calculate the effective entries.
   // the case when errors are zero (w1 == 0 or w2 ==0) are equivalent to
   // compare to a function. In that case the rescaling is done only on sqrt(esum2) or sqrt(esum1)
   Double_t esum1 = 0.;
	Double_t esum2 = 0.;
   if(w1 > 0) {
      esum1 = sum1 * sum1 / w1;
   } else {
      afunc1 = kTRUE;   // use later for calculating z
   }

   if(w2 > 0) {
      esum2 = sum2 * sum2 / w2;
   } else {
      afunc2 = kTRUE;   // use later for calculating z
   }

   if(afunc2 && afunc1) {
      Error("KolmogorovTest", "Errors are zero for both histograms\n");
      return 0;
   }

   // Find first Kolmogorov distance
   Double_t s1     = 1 / sum1;
   Double_t s2     = 1 / sum2;
   Double_t dfmax1 = 0;
   Double_t rsum1 = 0.;
	Double_t rsum2 = 0.;
   for(Int_t i = ibeg; i <= iend; ++i) {
      for(Int_t j = jbeg; j <= jend; ++j) {
         rsum1 += s1 * h1->GetCellContent(i, j);
         rsum2 += s2 * h2->GetCellContent(i, j);
         dfmax1 = TMath::Max(dfmax1, TMath::Abs(rsum1 - rsum2));
      }
   }

   //   Find second Kolmogorov distance
   Double_t dfmax2 = 0;
   rsum1 = 0.;
	rsum2 = 0.;
   for(Int_t j = jbeg; j <= jend; ++j) {
      for(Int_t i = ibeg; i <= iend; ++i) {
         rsum1 += s1 * h1->GetCellContent(i, j);
         rsum2 += s2 * h2->GetCellContent(i, j);
         dfmax2 = TMath::Max(dfmax2, TMath::Abs(rsum1 - rsum2));
      }
   }

   //   Get Kolmogorov probability: use effective entries, esum1 or esum2,  for normalizing it
   Double_t factnm = 0.;
   if(afunc1) {
      factnm = TMath::Sqrt(esum2);
   } else if(afunc2) {
      factnm = TMath::Sqrt(esum1);
   } else {
      factnm = TMath::Sqrt(esum1 * sum2 / (esum1 + esum2));
   }

   // take average of the two distances
   Double_t dfmax = 0.5 * (dfmax1 + dfmax2);
   Double_t z     = dfmax * factnm;

   prb = TMath::KolmogorovProb(z);

   Double_t prb1 = 0;
   Double_t prb2 = 0;
   // option N to combine normalization makes sense if both afunc1 and afunc2 are false
   if(opt.Contains("N") && !(afunc1 || afunc2)) {
      // Combine probabilities for shape and normalization
      prb1          = prb;
      Double_t d12  = esum1 - esum2;
      Double_t chi2 = d12 * d12 / (esum1 + esum2);
      prb2          = TMath::Prob(chi2, 1);
      //   see Eadie et al., section 11.6.2
      if(prb > 0 && prb2 > 0) {
         prb = prb * prb2 * (1 - TMath::Log(prb * prb2));
      } else {
         prb = 0;
      }
   }
   //   debug printout
   if(opt.Contains("D")) {
      std::cout << " Kolmo Prob  h1 = " << h1->GetName() << ", sum1 = " << sum1 << std::endl;
      std::cout << " Kolmo Prob  h2 = " << h2->GetName() << ", sum2 = " << sum2 << std::endl;
      std::cout << " Kolmo Probabil = " << prb << ", Max dist = " << dfmax << std::endl;
      if(opt.Contains("N")) {
         std::cout << " Kolmo Probabil = " << prb1 << " for shape alone, " << prb2 << " for normalisation alone" << std::endl;
      }
   }
   // This numerical error condition should never occur:
   if(TMath::Abs(rsum1 - 1) > 0.002) {
      Warning("KolmogorovTest", "Numerical problems with h1=%s\n", h1->GetName());
   }
   if(TMath::Abs(rsum2 - 1) > 0.002) {
      Warning("KolmogorovTest", "Numerical problems with h2=%s\n", h2->GetName());
   }

   if(opt.Contains("M")) {
      return dfmax;   // return avergae of max distance
   }

   return prb;
}

Long64_t GHSym::Merge(TCollection* list)
{
   // Add all histograms in the collection to this histogram.
   // This function computes the min/max for the axes,
   // compute a new number of bins, if necessary,
   // add bin contents, errors and statistics.
   // If overflows are present and limits are different the function will fail.
   // The function returns the total number of entries in the result histogram
   // if the merge is successfull, -1 otherwise.
   //
   // IMPORTANT remark. The 2 axis x and y may have different number
   // of bins and different limits, BUT the largest bin width must be
   // a multiple of the smallest bin width and the upper limit must also
   // be a multiple of the bin width.

   if(list == nullptr) {
      return 0;
   }
   if(list->IsEmpty()) {
      return static_cast<Long64_t>(GetEntries());
   }

   TList inlist;
   inlist.AddAll(list);

   TAxis  newXAxis;
   TAxis  newYAxis;
   Bool_t initialLimitsFound  = kFALSE;
   Bool_t allSameLimits       = kTRUE;
   Bool_t sameLimitsX         = kTRUE;
   Bool_t sameLimitsY         = kTRUE;
   Bool_t allHaveLimits       = kTRUE;
   Bool_t firstHistWithLimits = kTRUE;

   TIter  next(&inlist);
   GHSym* h = this;
   do {
      Bool_t hasLimits = h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax();
      allHaveLimits    = allHaveLimits && hasLimits;

      if(hasLimits) {
         h->BufferEmpty();

         // this is done in case the first histograms are empty and
         // the histogram have different limits
         if(firstHistWithLimits) {
            // set axis limits in the case the first histogram did not have limits
            if(h != this) {
               if(!SameLimitsAndNBins(fXaxis, *(h->GetXaxis()))) {
                  if(h->GetXaxis()->GetXbins()->GetSize() != 0) {
                     fXaxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXbins()->GetArray());
                  } else {
                     fXaxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
                  }
               }
               if(!SameLimitsAndNBins(fYaxis, *(h->GetYaxis()))) {
                  if(h->GetYaxis()->GetXbins()->GetSize() != 0) {
                     fYaxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXbins()->GetArray());
                  } else {
                     fYaxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
                  }
               }
            }
            firstHistWithLimits = kFALSE;
         }

         if(!initialLimitsFound) {
            // this is executed the first time an histogram with limits is found
            // to set some initial values on the new axes
            initialLimitsFound = kTRUE;
            if(h->GetXaxis()->GetXbins()->GetSize() != 0) {
               newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXbins()->GetArray());
            } else {
               newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
            }
            if(h->GetYaxis()->GetXbins()->GetSize() != 0) {
               newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXbins()->GetArray());
            } else {
               newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
            }
         } else {
            // check first if histograms have same bins in X
            if(!SameLimitsAndNBins(newXAxis, *(h->GetXaxis()))) {
               sameLimitsX = kFALSE;
               // recompute in this case the optimal limits
               // The condition to works is that the histogram have same bin with
               // and one common bin edge
               if(!RecomputeAxisLimits(newXAxis, *(h->GetXaxis()))) {
                  Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                                 "first: (%d, %f, %f), second: (%d, %f, %f)",
                        newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax(), h->GetXaxis()->GetNbins(),
                        h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
                  return -1;
               }
            }

            // check first if histograms have same bins in Y
            if(!SameLimitsAndNBins(newYAxis, *(h->GetYaxis()))) {
               sameLimitsY = kFALSE;
               // recompute in this case the optimal limits
               // The condition to works is that the histogram have same bin with
               // and one common bin edge
               if(!RecomputeAxisLimits(newYAxis, *(h->GetYaxis()))) {
                  Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                                 "first: (%d, %f, %f), second: (%d, %f, %f)",
                        newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax(), h->GetYaxis()->GetNbins(),
                        h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
                  return -1;
               }
            }
            allSameLimits = sameLimitsY && sameLimitsX;
         }
      }
   } while((h = static_cast<GHSym*>(next())) != nullptr);
   if(h == nullptr && ((*next) != nullptr)) {
      Error("Merge", "Attempt to merge object of class: %s to a %s", (*next)->ClassName(), ClassName());
      return -1;
   }
   next.Reset();

   // In the case of histogram with different limits
   // newX(Y)Axis will now have the new found limits
   // but one needs first to clone this histogram to perform the merge
   // The clone is not needed when all histograms have the same limits
   TH2* hclone = nullptr;
   if(!allSameLimits) {
      // We don't want to add the clone to gDirectory,
      // so remove our kMustCleanup bit temporarily
      Bool_t mustCleanup = TestBit(kMustCleanup);
      if(mustCleanup) {
         ResetBit(kMustCleanup);
      }
      hclone = static_cast<TH2*>(IsA()->New());
      hclone->SetDirectory(nullptr);
      Copy(*hclone);
      if(mustCleanup) {
         SetBit(kMustCleanup);
      }
      BufferEmpty(1);   // To remove buffer.
      Reset();          // BufferEmpty sets limits so we can't use it later.
      SetEntries(0);
      inlist.AddFirst(hclone);
   }

   if(!allSameLimits && initialLimitsFound) {
      if(!sameLimitsX) {
         fXaxis.SetRange(0, 0);
         if(newXAxis.GetXbins()->GetSize() != 0) {
            fXaxis.Set(newXAxis.GetNbins(), newXAxis.GetXbins()->GetArray());
         } else {
            fXaxis.Set(newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax());
         }
      }
      if(!sameLimitsY) {
         fYaxis.SetRange(0, 0);
         if(newYAxis.GetXbins()->GetSize() != 0) {
            fYaxis.Set(newYAxis.GetNbins(), newYAxis.GetXbins()->GetArray());
         } else {
            fYaxis.Set(newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax());
         }
      }
      fZaxis.Set(1, 0, 1);
      fNcells = (fXaxis.GetNbins() + 2) * (fYaxis.GetNbins() + 2);
      SetBinsLength(fNcells);
      if(fSumw2.fN != 0) {
         fSumw2.Set(fNcells);
      }
   }

   if(!allHaveLimits) {
      // fill this histogram with all the data from buffers of histograms without limits
      while((h = static_cast<GHSym*>(next())) != nullptr) {
         if(h->GetXaxis()->GetXmin() >= h->GetXaxis()->GetXmax() && (h->fBuffer != nullptr)) {
            // no limits
            Int_t nbentries = static_cast<Int_t>(h->fBuffer[0]);
            for(Int_t i = 0; i < nbentries; i++) {
               Fill(h->fBuffer[3 * i + 2], h->fBuffer[3 * i + 3], h->fBuffer[3 * i + 1]);
            }
            // Entries from buffers have to be filled one by one
            // because FillN doesn't resize histograms.
         }
      }
      if(!initialLimitsFound) {
         if(hclone != nullptr) {
            inlist.Remove(hclone);
            delete hclone;
         }
         return static_cast<Long64_t>(GetEntries());   // all histograms have been processed
      }
      next.Reset();
   }

   // merge bin contents and errors
	std::array<Double_t, kNstat> stats;
	std::array<Double_t, kNstat> totstats;
   for(Int_t i = 0; i < kNstat; ++i) {
      totstats[i] = stats[i] = 0;
   }
   GetStats(totstats.data());
   Double_t nentries = GetEntries();
	Int_t    ix = 0;
	Int_t    iy = 0;
   Double_t cu = 0.;
   Bool_t   canExtend = CanExtendAllAxes();
   SetCanExtend(TH1::kNoAxis);   // reset, otherwise setting the under/overflow will extend the axis

   while((h = static_cast<GHSym*>(next())) != nullptr) {
      // skip empty histograms
      Double_t histEntries = h->GetEntries();
      if(h->fTsumw == 0 && histEntries == 0) {
         continue;
      }

      // process only if the histogram has limits; otherwise it was processed before
      if(h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax()) {
         // import statistics
         h->GetStats(stats.data());
         for(Int_t i = 0; i < kNstat; ++i) {
            totstats[i] += stats[i];
         }
         nentries += histEntries;

         Int_t nx = h->GetXaxis()->GetNbins();
         Int_t ny = h->GetYaxis()->GetNbins();

         for(Int_t biny = 0; biny <= ny + 1; ++biny) {
            if(!allSameLimits) {
               iy = fYaxis.FindBin(h->GetYaxis()->GetBinCenter(biny));
            } else {
               iy = biny;
            }
            for(Int_t binx = 0; binx <= nx + 1; ++binx) {
               cu = h->GetBinContent(binx, biny);
               if(!allSameLimits) {
                  if(cu != 0 && ((!sameLimitsX && (binx == 0 || binx == nx + 1)) ||
                                 (!sameLimitsY && (biny == 0 || biny == ny + 1)))) {
                     Error("Merge", "Cannot merge histograms - the histograms have"
                                    " different limits and undeflows/overflows are present."
                                    " The initial histogram is now broken!");
                     return -1;
                  }
                  ix = fXaxis.FindBin(h->GetXaxis()->GetBinCenter(binx));
               } else {
                  // case histograms with the same limits
                  ix = binx;
               }
               Int_t ibin = GetBin(ix, iy);

               if(ibin < 0) {
                  continue;
               }
               AddBinContent(ibin, cu);
               if(fSumw2.fN != 0) {
                  Double_t error1 = h->GetBinError(GetBin(binx, biny));
                  fSumw2.fArray[ibin] += error1 * error1;
               }
            }
         }
      }
   }
   SetCanExtend(static_cast<UInt_t>(canExtend));

   // copy merged stats
   PutStats(totstats.data());
   SetEntries(nentries);
   if(hclone != nullptr) {
      inlist.Remove(hclone);
      delete hclone;
   }
   return static_cast<Long64_t>(nentries);
}

TProfile* GHSym::Profile(const char* name, Int_t firstbin, Int_t lastbin, Option_t* option) const
{
   // *-*-*-*-*Project a 2-D histogram into a profile histogram along X*-*-*-*-*-*
   // *-*      ========================================================
   //
   //   The projection is made from the channels along the Y axis
   //   ranging from firstybin to lastybin included.
   //   By default, bins 1 to ny are included
   //   When all bins are included, the number of entries in the projection
   //   is set to the number of entries of the 2-D histogram, otherwise
   //   the number of entries is incremented by 1 for all non empty cells.
   //
   //   if option "d" is specified, the profile is drawn in the current pad.
   //
   //   if option "o" original axis range of the target axes will be
   //   kept, but only bins inside the selected range will be filled.
   //
   //   The option can also be used to specify the projected profile error type.
   //   Values which can be used are 's', 'i', or 'g'. See TProfile::BuildOptions for details
   //
   //   Using a TCutG object, it is possible to select a sub-range of a 2-D histogram.
   //   One must create a graphical cut (mouse or C++) and specify the name
   //   of the cut between [] in the option.
   //   For example, with a TCutG named "cutg", one can call:
   //      myhist->ProfileX(" ",firstybin,lastybin,"[cutg]");
   //   To invert the cut, it is enough to put a "-" in front of its name:
   //      myhist->ProfileX(" ",firstybin,lastybin,"[-cutg]");
   //   It is possible to apply several cuts ("," means logical AND):
   //      myhist->ProfileX(" ",firstybin,lastybin,"[cutg1,cutg2]");
   //
   //   NOTE that if a TProfile named "name" exists in the current directory or pad with
   //   a compatible axis the profile is reset and filled again with the projected contents of the TH2.
   //   In the case of axis incompatibility an error is reported and a NULL pointer is returned.
   //
   //   NOTE that the X axis attributes of the TH2 are copied to the X axis of the profile.
   //
   //   NOTE that the default under- / overflow behavior differs from what ProjectionX
   //   does! Profiles take the bin center into account, so here the under- and overflow
   //   bins are ignored by default.

   TString opt = option;
   // extract cut infor
   TString cut;
   Int_t   i1 = opt.Index("[");
   if(i1 >= 0) {
      Int_t i2 = opt.Index("]");
      cut      = opt(i1, i2 - i1 + 1);
   }
   opt.ToLower();
   bool originalRange = opt.Contains("o");

   Int_t       inN          = fYaxis.GetNbins();
   const char* expectedName = "_pf";

   Int_t firstOutBin = fXaxis.GetFirst();
   Int_t lastOutBin  = fXaxis.GetLast();
   if(firstOutBin == 0 && lastOutBin == 0) {
      firstOutBin = 1;
      lastOutBin  = fXaxis.GetNbins();
   }

   if(lastbin < firstbin && fYaxis.TestBit(TAxis::kAxisRange)) {
      firstbin = fYaxis.GetFirst();
      lastbin  = fYaxis.GetLast();
      // For special case of TAxis::SetRange, when first == 1 and last
      // = N and the range bit has been set, the TAxis will return 0
      // for both.
      if(firstbin == 0 && lastbin == 0) {
         firstbin = 1;
         lastbin  = fYaxis.GetNbins();
      }
   }
   if(firstbin < 0) {
      firstbin = 1;
   }
   if(lastbin < 0) {
      lastbin = inN;
   }
   if(lastbin > inN + 1) {
      lastbin = inN;
   }

   // Create the profile histogram
   char* pname = const_cast<char*>(name);
   if((name != nullptr) && strcmp(name, expectedName) == 0) {
      auto nch = strlen(GetName()) + 5;
      pname    = new char[nch];
      snprintf(pname, nch, "%s%s", GetName(), name);
   }
   TProfile* h1 = nullptr;
   // check if a profile with identical name exist
   // if compatible reset and re-use previous histogram
   TObject* h1obj = gROOT->FindObject(pname);
   if((h1obj != nullptr) && h1obj->InheritsFrom(TH1::Class())) {
      if(h1obj->IsA() != TProfile::Class()) {
         Error("DoProfile", "Histogram with name %s must be a TProfile and is a %s", name, h1obj->ClassName());
         return nullptr;
      }
      h1 = static_cast<TProfile*>(h1obj);
      // reset the existing histogram and set always the new binning for the axis
      // This avoid problems when the histogram already exists and the histograms is rebinned or its range has changed
      // (see https://savannah.cern.ch/bugs/?94101 or https://savannah.cern.ch/bugs/?95808 )
      h1->Reset();
      const TArrayD* xbins = fXaxis.GetXbins();
      if(xbins->fN == 0) {
         if(originalRange) {
            h1->SetBins(fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
         } else {
            h1->SetBins(lastOutBin - firstOutBin + 1, fXaxis.GetBinLowEdge(firstOutBin),
                        fXaxis.GetBinUpEdge(lastOutBin));
         }
      } else {
         // case variable bins
         if(originalRange) {
            h1->SetBins(fXaxis.GetNbins(), xbins->fArray);
         } else {
            h1->SetBins(lastOutBin - firstOutBin + 1, &xbins->fArray[firstOutBin - 1]);
         }
      }
   }

   Int_t ncuts = 0;
   if(opt.Contains("[")) {
      const_cast<GHSym*>(this)->GetPainter();
      if(fPainter != nullptr) {
         ncuts = fPainter->MakeCuts(const_cast<char*>(cut.Data()));
      }
   }

   if(h1 == nullptr) {
      const TArrayD* bins = fXaxis.GetXbins();
      if(bins->fN == 0) {
         if(originalRange) {
            h1 = new TProfile(pname, GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax(), opt);
         } else {
            h1 = new TProfile(pname, GetTitle(), lastOutBin - firstOutBin + 1, fXaxis.GetBinLowEdge(firstOutBin),
                              fXaxis.GetBinUpEdge(lastOutBin), opt);
         }
      } else {
         // case variable bins
         if(originalRange) {
            h1 = new TProfile(pname, GetTitle(), fXaxis.GetNbins(), bins->fArray, opt);
         } else {
            h1 = new TProfile(pname, GetTitle(), lastOutBin - firstOutBin + 1, &bins->fArray[firstOutBin - 1], opt);
         }
      }
   }
   if(pname != name) {
      delete[] pname;
   }

   // Copy attributes
   h1->GetXaxis()->ImportAttributes(&fXaxis);
   h1->SetLineColor(GetLineColor());
   h1->SetFillColor(GetFillColor());
   h1->SetMarkerColor(GetMarkerColor());
   h1->SetMarkerStyle(GetMarkerStyle());

   // check if histogram is weighted
   // in case need to store sum of weight square/bin for the profile
   bool useWeights = (GetSumw2N() > 0);
   if(useWeights) {
      h1->Sumw2();
   }

   // Fill the profile histogram
   // no entries/bin is available so can fill only using bin content as weight
   TArrayD& binSumw2 = *(h1->GetBinSumw2());

   // implement filling of projected histogram
   // outbin is bin number of fXaxis (the projected axis). Loop is done on all bin of TH2 histograms
   // inbin is the axis being integrated. Loop is done only on the selected bins
   for(Int_t outbin = 0; outbin <= fXaxis.GetNbins() + 1; ++outbin) {
      if(fXaxis.TestBit(TAxis::kAxisRange) && (outbin < firstOutBin || outbin > lastOutBin)) {
         continue;
      }

      // find corresponding bin number in h1 for outbin (binOut)
      Double_t xOut   = fXaxis.GetBinCenter(outbin);
      Int_t    binOut = h1->GetXaxis()->FindBin(xOut);
      if(binOut < 0) {
         continue;
      }

      for(Int_t inbin = firstbin; inbin <= lastbin; ++inbin) {
         Int_t binx = outbin;
         Int_t biny = inbin;

         if(ncuts != 0) {
            if(!fPainter->IsInside(binx, biny)) {
               continue;
            }
         }
         Int_t    bin = GetBin(binx, biny);
         Double_t cxy = GetBinContent(bin);

         if(cxy != 0.0) {
            Double_t tmp = 0;
            // the following fill update wrongly the fBinSumw2- need to save it before
            if(useWeights) {
               tmp = binSumw2.fArray[binOut];
            }
            h1->Fill(xOut, fYaxis.GetBinCenter(inbin), cxy);
            if(useWeights) {
               binSumw2.fArray[binOut] = tmp + fSumw2.fArray[bin];
            }
         }
      }
   }

   // the statistics must be recalculated since by using the Fill method the total sum of weight^2 is
   // not computed correctly
   // for a profile does not much sense to re-use statistics of original TH2
   h1->ResetStats();
   // Also we need to set the entries since they have not been correctly calculated during the projection
   // we can only set them to the effective entries
   h1->SetEntries(h1->GetEffectiveEntries());

   if(opt.Contains("d")) {
      TVirtualPad* padsav = gPad;
      TVirtualPad* pad    = gROOT->GetSelectedPad();
      if(pad != nullptr) {
         pad->cd();
      }
      opt.Remove(opt.First("d"), 1);
      if(!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if(padsav != nullptr) {
         padsav->cd();
      }
   }
   return h1;
}

TH1D* GHSym::Projection(const char* name, Int_t firstBin, Int_t lastBin, Option_t* option) const
{
   /// method for performing projection

   const char* expectedName = "_pr";

   TString opt = option;
   TString cut;
   Int_t   i1 = opt.Index("[");
   if(i1 >= 0) {
      Int_t i2 = opt.Index("]");
      cut      = opt(i1, i2 - i1 + 1);
   }
   opt.ToLower();   // must be called after having parsed the cut name
   bool originalRange = opt.Contains("o");

   Int_t firstXBin = fXaxis.GetFirst();
   Int_t lastXBin  = fXaxis.GetLast();

   if(firstXBin == 0 && lastXBin == 0) {
      firstXBin = 1;
      lastXBin  = fXaxis.GetNbins();
   }

   if(lastBin < firstBin && fYaxis.TestBit(TAxis::kAxisRange)) {
      firstBin = fYaxis.GetFirst();
      lastBin  = fYaxis.GetLast();
      // For special case of TAxis::SetRange, when first == 1 and last
      // = N and the range bit has been set, the TAxis will return 0
      // for both.
      if(firstBin == 0 && lastBin == 0) {
         firstBin = 1;
         lastBin  = fYaxis.GetNbins();
      }
   }
   if(firstBin < 0) {
      firstBin = 0;
   }
   if(lastBin < 0) {
      lastBin = fYaxis.GetLast() + 1;
   }
   if(lastBin > fYaxis.GetLast() + 1) {
      lastBin = fYaxis.GetLast() + 1;
   }

   // Create the projection histogram
   char* pname = const_cast<char*>(name);
   if(name != nullptr && strcmp(name, expectedName) == 0) {
      auto nch = strlen(GetName()) + 4;
      pname    = new char[nch];
      snprintf(pname, nch, "%s%s", GetName(), name);
   }
   TH1D* h1 = nullptr;
   // check if histogram with identical name exist
   // if compatible reset and re-use previous histogram
   // (see https://savannah.cern.ch/bugs/?54340)
   TObject* h1obj = gROOT->FindObject(pname);
   if((h1obj != nullptr) && h1obj->InheritsFrom(TH1::Class())) {
      if(h1obj->IsA() != TH1D::Class()) {
         Error("DoProjection", "Histogram with name %s must be a TH1D and is a %s", name, h1obj->ClassName());
         return nullptr;
      }
      h1 = static_cast<TH1D*>(h1obj);
      // reset the existing histogram and set always the new binning for the axis
      // This avoid problems when the histogram already exists and the histograms is rebinned or its range has changed
      // (see https://savannah.cern.ch/bugs/?94101 or https://savannah.cern.ch/bugs/?95808 )
      h1->Reset();
      const TArrayD* xbins = fXaxis.GetXbins();
      if(xbins->fN == 0) {
         if(originalRange) {
            h1->SetBins(fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
         } else {
            h1->SetBins(lastXBin - firstXBin + 1, fXaxis.GetBinLowEdge(firstXBin), fXaxis.GetBinUpEdge(lastXBin));
         }
      } else {
         // case variable bins
         if(originalRange) {
            h1->SetBins(fXaxis.GetNbins(), xbins->fArray);
         } else {
            h1->SetBins(lastXBin - firstXBin + 1, &(xbins->fArray[firstXBin - 1]));
         }
      }
   }
   Int_t ncuts = 0;
   if(opt.Contains("[")) {
      const_cast<GHSym*>(this)->GetPainter();
      if(fPainter != nullptr) {
         ncuts = fPainter->MakeCuts(const_cast<char*>(cut.Data()));
      }
   }

   if(h1 == nullptr) {
      const TArrayD* bins = fXaxis.GetXbins();
      if(bins->fN == 0) {
         if(originalRange) {
            h1 = new TH1D(pname, GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
         } else {
            h1 = new TH1D(pname, GetTitle(), lastXBin - firstXBin + 1, fXaxis.GetBinLowEdge(firstXBin),
                          fXaxis.GetBinUpEdge(lastXBin));
         }
      } else {
         // case variable bins
         if(originalRange) {
            h1 = new TH1D(pname, GetTitle(), fXaxis.GetNbins(), bins->fArray);
         } else {
            h1 = new TH1D(pname, GetTitle(), lastXBin - firstXBin + 1, &(bins->fArray[firstXBin - 1]));
         }
      }
      if(opt.Contains("e") || (GetSumw2N() != 0)) {
         h1->Sumw2();
      }
   }
   if(pname != name) {
      delete[] pname;
   }

   // Copy the axis attributes and the axis labels if needed.
   h1->GetXaxis()->ImportAttributes(&fXaxis);
   THashList* labels = const_cast<TAxis*>(&fXaxis)->GetLabels();
   if(labels != nullptr) {
      TIter       iL(labels);
      TObjString* lb = nullptr;
      Int_t       i = 1;
      while((lb = static_cast<TObjString*>(iL())) != nullptr) {
         h1->GetXaxis()->SetBinLabel(i, lb->String().Data());
         i++;
      }
   }

   h1->SetLineColor(GetLineColor());
   h1->SetFillColor(GetFillColor());
   h1->SetMarkerColor(GetMarkerColor());
   h1->SetMarkerStyle(GetMarkerStyle());

   // Fill the projected histogram
   Double_t totcont       = 0;
   Bool_t   computeErrors = h1->GetSumw2N() != 0;

   // implement filling of projected histogram
   // xbin is bin number of xAxis (the projected axis). Loop is done on all bin of TH2 histograms
   // inbin is the axis being integrated. Loop is done only on the selected bins
   for(Int_t xbin = 0; xbin <= fXaxis.GetNbins() + 1; ++xbin) {
      Double_t err2 = 0.;
      Double_t cont = 0.;
      if(fXaxis.TestBit(TAxis::kAxisRange) && (xbin < firstXBin || xbin > lastXBin)) {
         continue;
      }

      for(Int_t ybin = firstBin; ybin <= lastBin; ++ybin) {
         if(ncuts != 0) {
            if(!fPainter->IsInside(xbin, ybin)) {
               continue;
            }
         }
         // sum bin content and error if needed
         cont += GetCellContent(xbin, ybin);
         if(computeErrors) {
            Double_t exy = GetCellError(xbin, ybin);
            err2 += exy * exy;
         }
      }
      // find corresponding bin number in h1 for xbin
      Int_t binOut = h1->GetXaxis()->FindBin(fXaxis.GetBinCenter(xbin));
      h1->SetBinContent(binOut, cont);
      if(computeErrors) {
         h1->SetBinError(binOut, TMath::Sqrt(err2));
      }
      // sum  all content
      totcont += cont;
   }

   // check if we can re-use the original statistics from  the previous histogram
   bool reuseStats = false;
   if((!fgStatOverflows && firstBin == 1 && lastBin == fYaxis.GetLast()) ||
      (fgStatOverflows && firstBin == 0 && lastBin == fYaxis.GetLast() + 1)) {
      reuseStats = true;
   } else {
      // also if total content match we can re-use
      double eps = 1.E-12;
      if(IsA() == GHSymF::Class()) {
         eps = 1.E-6;
      }
      if(fTsumw != 0 && TMath::Abs(fTsumw - totcont) < TMath::Abs(fTsumw) * eps) {
         reuseStats = true;
      }
   }
   if(ncuts != 0) {
      reuseStats = false;
   }
   // retrieve  the statistics and set in projected histogram if we can re-use it
   bool reuseEntries = reuseStats;
   // can re-use entries if underflow/overflow are included
   reuseEntries &= static_cast<int>(firstBin == 0 && lastBin == fYaxis.GetLast() + 1);
   if(reuseStats) {
		std::array<Double_t, kNstat> stats;
      GetStats(stats.data());
      h1->PutStats(stats.data());
   } else {
      // the statistics is automatically recalulated since it is reset by the call to SetBinContent
      // we just need to set the entries since they have not been correctly calculated during the projection
      // we can only set them to the effective entries
      h1->SetEntries(h1->GetEffectiveEntries());
   }
   if(reuseEntries) {
      h1->SetEntries(fEntries);
   } else {
      // re-compute the entries
      // in case of error calculation (i.e. when Sumw2() is set)
      // use the effective entries for the entries
      // since this  is the only way to estimate them
      Double_t entries = TMath::Floor(totcont + 0.5);   // to avoid numerical rounding
      if(h1->GetSumw2N() != 0) {
         entries = h1->GetEffectiveEntries();
      }
      h1->SetEntries(entries);
   }

   if(opt.Contains("d")) {
      TVirtualPad* padsav = gPad;
      TVirtualPad* pad    = gROOT->GetSelectedPad();
      if(pad != nullptr) {
         pad->cd();
      }
      opt.Remove(opt.First("d"), 1);
      // remove also other options
      if(opt.Contains("e")) {
         opt.Remove(opt.First("e"), 1);
      }
      if(!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if(padsav != nullptr) {
         padsav->cd();
      }
   }

   return h1;
}

void GHSym::PutStats(Double_t* stats)
{
   // Replace current statistics with the values in array stats
   TH1::PutStats(stats);
   fTsumwy  = stats[4];
   fTsumwy2 = stats[5];
   fTsumwxy = stats[6];
}

GHSym* GHSym::Rebin2D(Int_t ngroup, const char* newname)
{
   //   -*-*-*Rebin this histogram grouping ngroup/ngroup bins along the xaxis/yaxis together*-*-*-*-
   //         =================================================================================
   //   if newname is not blank a new temporary histogram hnew is created.
   //   else the current histogram is modified (default)
   //   The parameter ngroup indicates how many bins along the xaxis/yaxis of this
   //   have to me merged into one bin of hnew
   //   If the original histogram has errors stored (via Sumw2), the resulting
   //   histograms has new errors correctly calculated.
   //
   //   examples: if hpxpy is an existing GHSym histogram with 40 x 40 bins
   //     hpxpy->Rebin2D();  // merges two bins along the xaxis and yaxis in one in hpxpy
   //                        // Carefull: previous contents of hpxpy are lost
   //     hpxpy->Rebin2D(5); //merges five bins along the xaxisi and yaxis in one in hpxpy
   //     GHSym* hnew = hpxpy->Rebin2D(5,"hnew"); // creates a new histogram hnew
   //                        // merging 5 bins of h1 along the xaxis and yaxis in one bin
   //
   //   NOTE : If ngroup is not an exact divider of the number of bins,
   //          along the xaxis/yaxis the top limit(s) of the rebinned histogram
   //          is changed to the upper edge of the bin=newbins*ngroup
   //          and the corresponding bins are added to
   //          the overflow bin.
   //          Statistics will be recomputed from the new bin contents.

   Int_t    nbins = fXaxis.GetNbins();
   Double_t min   = fXaxis.GetXmin();
   Double_t max   = fXaxis.GetXmax();
   if((ngroup <= 0) || (ngroup > nbins)) {
      Error("Rebin", "Illegal value of ngroup=%d", ngroup);
      return nullptr;
   }

   Int_t newbins = nbins / ngroup;

   // Save old bin contents into a new array
   Double_t entries = fEntries;
   auto*    oldBins = new Double_t[(nbins + 2) * (nbins + 3) / 2];
   for(Int_t xbin = 0; xbin < nbins + 2; xbin++) {
      for(Int_t ybin = 0; ybin <= xbin; ybin++) {
         Int_t bin    = GetBin(xbin, ybin);
         oldBins[bin] = GetBinContent(bin);
      }
   }
   Double_t* oldErrors = nullptr;
   if(fSumw2.fN != 0) {
      oldErrors = new Double_t[(nbins + 2) * (nbins + 3) / 2];
      for(Int_t xbin = 0; xbin < nbins + 2; xbin++) {
         for(Int_t ybin = 0; ybin <= xbin; ybin++) {
            Int_t bin      = GetBin(xbin, ybin);
            oldErrors[bin] = GetBinError(bin);
         }
      }
   }

   // create a clone of the old histogram if newname is specified
   GHSym* hnew = this;
   if((newname != nullptr) && (strlen(newname) != 0u)) {
      hnew = static_cast<GHSym*>(Clone());
      hnew->SetName(newname);
   }

   // save original statistics
	std::array<Double_t, kNstat> stats;
	GetStats(stats.data());
   bool resetStat = false;

   // change axis specs and rebuild bin contents array
   if(newbins * ngroup != nbins) {
      max       = fXaxis.GetBinUpEdge(newbins * ngroup);
      resetStat = true;   // stats must be reset because top bins will be moved to overflow bin
   }
   // save the TAttAxis members (reset by SetBins) for x axis
   Int_t   nXdivisions  = fXaxis.GetNdivisions();
   Color_t xAxisColor   = fXaxis.GetAxisColor();
   Color_t xLabelColor  = fXaxis.GetLabelColor();
   Style_t xLabelFont   = fXaxis.GetLabelFont();
   Float_t xLabelOffset = fXaxis.GetLabelOffset();
   Float_t xLabelSize   = fXaxis.GetLabelSize();
   Float_t xTickLength  = fXaxis.GetTickLength();
   Float_t xTitleOffset = fXaxis.GetTitleOffset();
   Float_t xTitleSize   = fXaxis.GetTitleSize();
   Color_t xTitleColor  = fXaxis.GetTitleColor();
   Style_t xTitleFont   = fXaxis.GetTitleFont();
   // save the TAttAxis members (reset by SetBins) for y axis
   Int_t   nYdivisions  = fYaxis.GetNdivisions();
   Color_t yAxisColor   = fYaxis.GetAxisColor();
   Color_t yLabelColor  = fYaxis.GetLabelColor();
   Style_t yLabelFont   = fYaxis.GetLabelFont();
   Float_t yLabelOffset = fYaxis.GetLabelOffset();
   Float_t yLabelSize   = fYaxis.GetLabelSize();
   Float_t yTickLength  = fYaxis.GetTickLength();
   Float_t yTitleOffset = fYaxis.GetTitleOffset();
   Float_t yTitleSize   = fYaxis.GetTitleSize();
   Color_t yTitleColor  = fYaxis.GetTitleColor();
   Style_t yTitleFont   = fYaxis.GetTitleFont();

   // copy merged bin contents (ignore under/overflows)
   if(ngroup != 1) {
      if(fXaxis.GetXbins()->GetSize() > 0 || fYaxis.GetXbins()->GetSize() > 0) {
         // variable bin sizes in x or y, don't treat both cases separately
         auto* bins = new Double_t[newbins + 1];
         for(Int_t i = 0; i <= newbins; ++i) {
            bins[i] = fXaxis.GetBinLowEdge(1 + i * ngroup);
         }
         hnew->SetBins(newbins, bins, newbins, bins);   // changes also errors array (if any)
         delete[] bins;
      } else {
         hnew->SetBins(newbins, min, max, newbins, min, max);   // changes also errors array
      }

      Int_t    oldxbin = 1;
      Int_t    oldybin = 1;
      for(Int_t xbin = 1; xbin <= newbins; ++xbin) {
         oldybin = 1;
         for(Int_t ybin = 1; ybin <= xbin; ++ybin) {
            Double_t binContent = 0.;
            Double_t binError   = 0.;
            for(Int_t i = 0; i < ngroup; ++i) {
               if(oldxbin + i > nbins) {
                  break;
               }
               for(Int_t j = 0; j < ngroup; ++j) {
                  if(oldybin + j > nbins) {
                     break;
                  }
                  // get global bin (same conventions as in GHSym::GetBin(xbin,ybin)
                  if(oldybin + j <= oldxbin + i) {
                     bin = oldxbin + i + (oldybin + j) * (2 * fXaxis.GetNbins() - (oldybin + j) + 3) / 2;
                  } else {
                     bin = oldybin + j + (oldxbin + i) * (2 * fXaxis.GetNbins() - (oldxbin + i) + 3) / 2;
                  }
                  binContent += oldBins[bin];
                  if(oldErrors != nullptr) {
                     binError += oldErrors[bin] * oldErrors[bin];
                  }
               }
            }
            hnew->SetBinContent(xbin, ybin, binContent);
            if(oldErrors != nullptr) {
               hnew->SetBinError(xbin, ybin, TMath::Sqrt(binError));
            }
            oldybin += ngroup;
         }
         oldxbin += ngroup;
      }

      // Recompute correct underflows and overflows.

      // copy old underflow bin in x and y (0,0)
      hnew->SetBinContent(0, 0, oldBins[0]);
      if(oldErrors != nullptr) {
         hnew->SetBinError(0, 0, oldErrors[0]);
      }

      // calculate new overflow bin in x and y (newbins+1,newbins+1)
      Double_t binContent = 0.;
      Double_t binError   = 0.;
      for(Int_t xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
         for(Int_t ybin = oldybin; ybin <= xbin; ++ybin) {
            bin = xbin + ybin * (2 * nbins - ybin + 3) / 2;
            binContent += oldBins[bin];
            if(oldErrors != nullptr) {
               binError += oldErrors[bin] * oldErrors[bin];
            }
         }
      }
      hnew->SetBinContent(newbins + 1, newbins + 1, binContent);
      if(oldErrors != nullptr) {
         hnew->SetBinError(newbins + 1, newbins + 1, TMath::Sqrt(binError));
      }

      // calculate new underflow bin in x and overflow in y (0,newbins+1)
      binContent = 0.;
      binError   = 0.;
      for(Int_t ybin = oldybin; ybin <= nbins + 1; ++ybin) {
         bin = ybin * (2 * nbins - ybin + 3) / 2;
         binContent += oldBins[bin];
         if(oldErrors != nullptr) {
            binError += oldErrors[bin] * oldErrors[bin];
         }
      }
      hnew->SetBinContent(0, newbins + 1, binContent);
      if(oldErrors != nullptr) {
         hnew->SetBinError(0, newbins + 1, TMath::Sqrt(binError));
      }

      // calculate new overflow bin in x and underflow in y (newbins+1,0)
      binContent = 0.;
      binError   = 0.;
      for(Int_t xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
         bin = xbin;
         binContent += oldBins[bin];
         if(oldErrors != nullptr) {
            binError += oldErrors[bin] * oldErrors[bin];
         }
      }
      hnew->SetBinContent(newbins + 1, 0, binContent);
      if(oldErrors != nullptr) {
         hnew->SetBinError(newbins + 1, 0, TMath::Sqrt(binError));
      }

      //  recompute under/overflow contents in y for the new  x bins
      Int_t oldxbin2 = 1;
      for(Int_t xbin = 1; xbin <= newbins; ++xbin) {
         Double_t binContent0 = 0.;
			Double_t binContent2 = 0.;
         Double_t binError0 = 0.;
			Double_t binError2 = 0.;
         for(Int_t i = 0; i < ngroup; ++i) {
            if(oldxbin2 + i > nbins) {
               break;
            }
            // old underflow bin (in y)
            Int_t ufbin = oldxbin2 + i;
            binContent0 += oldBins[ufbin];
            if(oldErrors != nullptr) {
               binError0 += oldErrors[ufbin] * oldErrors[ufbin];
            }
            for(Int_t ybin = oldybin; ybin <= nbins + 1; ++ybin) {
               // old overflow bin (in y)
               Int_t ofbin = ufbin + ybin * (nbins + 2);
               binContent2 += oldBins[ofbin];
               if(oldErrors != nullptr) {
                  binError2 += oldErrors[ofbin] * oldErrors[ofbin];
               }
            }
         }
         hnew->SetBinContent(xbin, 0, binContent0);
         hnew->SetBinContent(xbin, newbins + 1, binContent2);
         if(oldErrors != nullptr) {
            hnew->SetBinError(xbin, 0, TMath::Sqrt(binError0));
            hnew->SetBinError(xbin, newbins + 1, TMath::Sqrt(binError2));
         }
         oldxbin2 += ngroup;
      }

      //  recompute under/overflow contents in x for the new y bins
      Int_t oldybin2 = 1;
      for(Int_t ybin = 1; ybin <= newbins; ++ybin) {
         Double_t binContent0 = 0.;
			Double_t binContent2 = 0.;
         Double_t binError0 = 0.;
			Double_t binError2 = 0.;
         for(Int_t i = 0; i < ngroup; ++i) {
            if(oldybin2 + i > nbins) {
               break;
            }
            // old underflow bin (in x)
            Int_t ufbin = (oldybin2 + i) * (nbins + 2);
            binContent0 += oldBins[ufbin];
            if(oldErrors != nullptr) {
               binError0 += oldErrors[ufbin] * oldErrors[ufbin];
            }
            for(Int_t xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
               Int_t ofbin = ufbin + xbin;
               binContent2 += oldBins[ofbin];
               if(oldErrors != nullptr) {
                  binError2 += oldErrors[ofbin] * oldErrors[ofbin];
               }
            }
         }
         hnew->SetBinContent(0, ybin, binContent0);
         hnew->SetBinContent(newbins + 1, ybin, binContent2);
         if(oldErrors != nullptr) {
            hnew->SetBinError(0, ybin, TMath::Sqrt(binError0));
            hnew->SetBinError(newbins + 1, ybin, TMath::Sqrt(binError2));
         }
         oldybin2 += ngroup;
      }
   }

   // Restore x axis attributes
   fXaxis.SetNdivisions(nXdivisions);
   fXaxis.SetAxisColor(xAxisColor);
   fXaxis.SetLabelColor(xLabelColor);
   fXaxis.SetLabelFont(xLabelFont);
   fXaxis.SetLabelOffset(xLabelOffset);
   fXaxis.SetLabelSize(xLabelSize);
   fXaxis.SetTickLength(xTickLength);
   fXaxis.SetTitleOffset(xTitleOffset);
   fXaxis.SetTitleSize(xTitleSize);
   fXaxis.SetTitleColor(xTitleColor);
   fXaxis.SetTitleFont(xTitleFont);
   // Restore y axis attributes
   fYaxis.SetNdivisions(nYdivisions);
   fYaxis.SetAxisColor(yAxisColor);
   fYaxis.SetLabelColor(yLabelColor);
   fYaxis.SetLabelFont(yLabelFont);
   fYaxis.SetLabelOffset(yLabelOffset);
   fYaxis.SetLabelSize(yLabelSize);
   fYaxis.SetTickLength(yTickLength);
   fYaxis.SetTitleOffset(yTitleOffset);
   fYaxis.SetTitleSize(yTitleSize);
   fYaxis.SetTitleColor(yTitleColor);
   fYaxis.SetTitleFont(yTitleFont);

   // restore statistics and entries  modified by SetBinContent
   hnew->SetEntries(entries);
   if(!resetStat) {
      hnew->PutStats(stats.data());
   }

   delete[] oldBins;
	delete[] oldErrors;
   return hnew;
}

void GHSym::Reset(Option_t* option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   TH1::Reset(option);
   TString opt = option;
   opt.ToUpper();

   if(opt.Contains("ICE") && !opt.Contains("S")) {
      return;
   }
   fTsumwy  = 0;
   fTsumwy2 = 0;
   fTsumwxy = 0;
   if(fMatrix != nullptr) {
      try {
         delete fMatrix;
         fMatrix = nullptr;
      } catch(std::exception& e) {
         fMatrix = nullptr;
      }
   }
}

void GHSym::SetCellContent(Int_t binx, Int_t biny, Double_t content)
{
   SetBinContent(GetBin(binx, biny), content);
}

void GHSym::SetCellError(Int_t binx, Int_t biny, Double_t content)
{
   SetBinError(GetBin(binx, biny), content);
}

void GHSym::SetShowProjectionX(Int_t nbins)
{
   // When the mouse is moved in a pad containing a 2-d view of this histogram
   // a second canvas shows the projection along X corresponding to the
   // mouse position along Y.
   // To stop the generation of the projections, delete the canvas
   // containing the projection.

   GetPainter();
   if(fPainter != nullptr) {
      fPainter->SetShowProjection("x", nbins);
   }
}

void GHSym::SetShowProjectionY(Int_t nbins)
{
   // When the mouse is moved in a pad containing a 2-d view of this histogram
   // a second canvas shows the projection along Y corresponding to the
   // mouse position along X.
   // To stop the generation of the projections, delete the canvas
   // containing the projection.

   GetPainter();
   if(fPainter != nullptr) {
      fPainter->SetShowProjection("y", nbins);
   }
}

TH1* GHSym::ShowBackground(Int_t niter, Option_t* option)
{
   //   This function calculates the background spectrum in this histogram.
   //   The background is returned as a histogram.
   //   to be implemented (may be)

   return reinterpret_cast<TH1*>(gROOT->ProcessLineFast(
      Form(R"(TSpectrum2::StaticBackground((TH1*)0x%lx,%d,"%s"))", (ULong_t)this, niter, option)));
}

Int_t GHSym::ShowPeaks(Double_t sigma, Option_t* option, Double_t threshold)
{
   // Interface to TSpectrum2::Search
   // the function finds peaks in this histogram where the width is > sigma
   // and the peak maximum greater than threshold*maximum bin content of this.
   // for more detauils see TSpectrum::Search.
   // note the difference in the default value for option compared to TSpectrum2::Search
   // option="" by default (instead of "goff")

   return static_cast<Int_t>(gROOT->ProcessLineFast(
      Form(R"(TSpectrum2::StaticSearch((TH1*)0x%lx,%g,"%s",%g))", (ULong_t)this, sigma, option, threshold)));
}

void GHSym::Smooth(Int_t ntimes, Option_t* option)
{
   // Smooth bin contents of this 2-d histogram using kernel algorithms
   // similar to the ones used in the raster graphics community.
   // Bin contents in the active range are replaced by their smooth values.
   // If Errors are defined via Sumw2, they are also scaled and computed.
   // However, note the resulting errors will be correlated between different-bins, so
   // the errors should not be used blindly to perform any calculation involving several bins,
   // like fitting the histogram.  One would need to compute also the bin by bin correlation matrix.
   //
   // 3 kernels are proposed k5a, k5b and k3a.
   // k5a and k5b act on 5x5 cells (i-2,i-1,i,i+1,i+2, and same for j)
   // k5b is a bit more stronger in smoothing
   // k3a acts only on 3x3 cells (i-1,i,i+1, and same for j).
   // By default the kernel "k5a" is used. You can select the kernels "k5b" or "k3a"
   // via the option argument.
   // If TAxis::SetRange has been called on the x or/and y axis, only the bins
   // in the specified range are smoothed.
   // In the current implementation if the first argument is not used (default value=1).
   //
   // implementation by David McKee (dmckee@bama.ua.edu). Extended by Rene Brun

   Double_t k5a[5][5] = {{0, 0, 1, 0, 0}, {0, 2, 2, 2, 0}, {1, 2, 5, 2, 1}, {0, 2, 2, 2, 0}, {0, 0, 1, 0, 0}};
   Double_t k5b[5][5] = {{0, 1, 2, 1, 0}, {1, 2, 4, 2, 1}, {2, 4, 8, 4, 2}, {1, 2, 4, 2, 1}, {0, 1, 2, 1, 0}};
   Double_t k3a[3][3] = {{0, 1, 0}, {1, 2, 1}, {0, 1, 0}};

   if(ntimes > 1) {
      Warning("Smooth", "Currently only ntimes=1 is supported");
   }
   TString opt = option;
   opt.ToLower();
   Int_t     ksize_x = 5;
   Int_t     ksize_y = 5;
   Double_t* kernel  = &k5a[0][0];
   if(opt.Contains("k5b")) {
      kernel = &k5b[0][0];
   }
   if(opt.Contains("k3a")) {
      kernel  = &k3a[0][0];
      ksize_x = 3;
      ksize_y = 3;
   }

   // find i,j ranges
   Int_t ifirst = fXaxis.GetFirst();
   Int_t ilast  = fXaxis.GetLast();
   Int_t jfirst = fYaxis.GetFirst();
   Int_t jlast  = fYaxis.GetLast();

   // Determine the size of the bin buffer(s) needed
   Double_t  nentries = fEntries;
   Int_t     nx       = GetNbinsX();
   Int_t     ny       = GetNbinsY();
   Int_t     bufSize  = (nx + 2) * (ny + 2);
   auto*     buf      = new Double_t[bufSize];
   Double_t* ebuf     = nullptr;
   if(fSumw2.fN != 0) {
      ebuf = new Double_t[bufSize];
   }

   // Copy all the data to the temporary buffers
   for(Int_t i = ifirst; i <= ilast; ++i) {
      for(Int_t j = jfirst; j <= jlast; ++j) {
         bin      = GetBin(i, j);
         buf[bin] = GetBinContent(bin);
         if(ebuf != nullptr) {
            ebuf[bin] = GetBinError(bin);
         }
      }
   }

   // Kernel tail sizes (kernel sizes must be odd for this to work!)
   Int_t x_push = (ksize_x - 1) / 2;
   Int_t y_push = (ksize_y - 1) / 2;

   // main work loop
   for(Int_t i = ifirst; i <= ilast; ++i) {
      for(Int_t j = jfirst; j <= jlast; ++j) {
         Double_t content = 0.0;
         Double_t error   = 0.0;
         Double_t norm    = 0.0;

         for(Int_t n = 0; n < ksize_x; ++n) {
            for(Int_t m = 0; m < ksize_y; ++m) {
               Int_t xb = i + (n - x_push);
               Int_t yb = j + (m - y_push);
               if((xb >= 1) && (xb <= nx) && (yb >= 1) && (yb <= ny)) {
                  bin        = GetBin(xb, yb);
                  Double_t k = kernel[n * ksize_y + m];
                  if(k != 0.0) {
                     norm += k;
                     content += k * buf[bin];
                     if(ebuf != nullptr) {
                        error += k * k * ebuf[bin] * ebuf[bin];
                     }
                  }
               }
            }
         }

         if(norm != 0.0) {
            SetBinContent(i, j, content / norm);
            if(ebuf != nullptr) {
               error /= (norm * norm);
               SetBinError(i, j, sqrt(error));
            }
         }
      }
   }
   fEntries = nentries;

   delete[] buf;
   delete[] ebuf;
}

//------------------------------------------------------------
// GHSymF methods (float = four bytes per cell)
//------------------------------------------------------------

GHSymF::GHSymF()
{
   SetBinsLength(9);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : GHSym(name, title, nbins, low, up)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }

   if(low >= up) {
      SetBuffer(fgBufferSize);
   }
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, const Double_t* bins) : GHSym(name, title, nbins, bins)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, const Float_t* bins) : GHSym(name, title, nbins, bins)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymF::GHSymF(const GHSymF& rhs)
{
   rhs.Copy(*this);
}

GHSymF::~GHSymF() = default;

TH2F* GHSymF::GetMatrix(bool force)
{
   if(fMatrix != nullptr && !force) {
      return static_cast<TH2F*>(fMatrix);
   }
   if(force && fMatrix != nullptr) {
      delete fMatrix;
   }

   fMatrix = new TH2F(Form("%s_mat", GetName()), GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax(),
                      fYaxis.GetNbins(), fYaxis.GetXmin(), fYaxis.GetXmax());
   // copy cell contents (including all overflow and underflow cells)
   for(int i = 0; i < fXaxis.GetNbins() + 2; ++i) {
      for(int j = 0; j < fXaxis.GetNbins() + 2; ++j) {
         fMatrix->SetBinContent(i, j, GetBinContent(i, j));
      }
   }
   return static_cast<TH2F*>(fMatrix);
}

void GHSymF::Copy(TObject& rhs) const
{
   GHSym::Copy(static_cast<GHSymF&>(rhs));
}

TH1* GHSymF::DrawCopy(Option_t* option, const char* name_postfix) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if(gPad != nullptr && !opt.Contains("same")) {
      gPad->Clear();
   }
   TString newName = (name_postfix) != nullptr ? TString::Format("%s%s", GetName(), name_postfix) : "";
   TH1*    newth1  = static_cast<TH1*>(Clone(newName));
   newth1->SetDirectory(nullptr);
   newth1->SetBit(kCanDelete);
   newth1->AppendPad(option);
   return newth1;
}

Double_t GHSymF::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if(fBuffer != nullptr) {
      const_cast<GHSymF*>(this)->BufferEmpty();
   }
   if(bin < 0) {
      bin = 0;
   }
   if(bin >= fNcells) {
      bin = fNcells - 1;
   }
   if(fArray == nullptr) {
      return 0;
   }
   return static_cast<Double_t>(fArray[bin]);
}

void GHSymF::Reset(Option_t* option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GHSym::Reset(option);
   TArrayF::Reset();
}

void GHSymF::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if(bin < 0) {
      return;
   }
   if(bin >= fNcells) {
      return;
   }
   fArray[bin] = static_cast<Float_t>(content);
}

void GHSymF::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) {
      n = (fXaxis.GetNbins() + 2) * (fYaxis.GetNbins() + 2);
   }
   fNcells = n;
   TArrayF::Set(n);
}

GHSymF& GHSymF::operator=(const GHSymF& h1)
{
   // Operator =

   if(this != &h1) {
      const_cast<GHSymF&>(h1).Copy(*this);
   }
   return *this;
}

GHSymF operator*(Float_t c1, GHSymF& h1)
{
   // Operator *

   GHSymF hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymF operator+(GHSymF& h1, GHSymF& h2)
{
   // Operator +

   GHSymF hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymF operator-(GHSymF& h1, GHSymF& h2)
{
   // Operator -

   GHSymF hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymF operator*(GHSymF& h1, GHSymF& h2)
{
   // Operator *

   GHSymF hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymF operator/(GHSymF& h1, GHSymF& h2)
{
   // Operator /

   GHSymF hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

//------------------------------------------------------------
// GHSymD methods (double = eight bytes per cell)
//------------------------------------------------------------

GHSymD::GHSymD()
{
   SetBinsLength(9);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : GHSym(name, title, nbins, low, up)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }

   if(low >= up) {
      SetBuffer(fgBufferSize);
   }
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, const Double_t* bins) : GHSym(name, title, nbins, bins)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, const Float_t* bins) : GHSym(name, title, nbins, bins)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GHSymD::GHSymD(const GHSymD& rhs)
{
   rhs.Copy(*this);
}

GHSymD::~GHSymD() = default;

TH2D* GHSymD::GetMatrix(bool force)
{
   if(fMatrix != nullptr && !force) {
      return static_cast<TH2D*>(fMatrix);
   }
   if(force && fMatrix != nullptr) {
      delete fMatrix;
   }

   fMatrix = new TH2D(Form("%s_mat", GetName()), Form("%s;%s;%s", GetTitle(), fXaxis.GetTitle(), fYaxis.GetTitle()),
                      fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax(), fYaxis.GetNbins(), fYaxis.GetXmin(),
                      fYaxis.GetXmax());
   // copy cell contents (including all overflow and underflow cells)
   for(int i = 0; i < fXaxis.GetNbins() + 2; ++i) {
      for(int j = 0; j < fXaxis.GetNbins() + 2; ++j) {
         fMatrix->SetBinContent(i, j, GetBinContent(i, j));
      }
   }
   return static_cast<TH2D*>(fMatrix);
}

void GHSymD::Copy(TObject& rhs) const
{
   GHSym::Copy(static_cast<GHSymD&>(rhs));
}

TH1* GHSymD::DrawCopy(Option_t* option, const char* name_postfix) const
{
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if(gPad != nullptr && !opt.Contains("same")) {
      gPad->Clear();
   }
   TString newName = (name_postfix) != nullptr ? TString::Format("%s%s", GetName(), name_postfix) : "";
   TH1*    newth1  = static_cast<TH1*>(Clone(newName));
   newth1->SetDirectory(nullptr);
   newth1->SetBit(kCanDelete);
   newth1->AppendPad(option);
   return newth1;
}

Double_t GHSymD::GetBinContent(Int_t bin) const
{
   // Get bin content.
   if(fBuffer != nullptr) {
      const_cast<GHSymD*>(this)->BufferEmpty();
   }
   if(bin < 0) {
      bin = 0;
   }
   if(bin >= fNcells) {
      bin = fNcells - 1;
   }
   if(fArray == nullptr) {
      return 0;
   }
   return static_cast<Double_t>(fArray[bin]);
}

void GHSymD::Reset(Option_t* option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GHSym::Reset(option);
   TArrayD::Reset();
}

void GHSymD::SetBinContent(Int_t bin, Double_t content)
{
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if(bin < 0) {
      return;
   }
   if(bin >= fNcells) {
      return;
   }
   fArray[bin] = static_cast<Float_t>(content);
}

void GHSymD::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) {
      n = (fXaxis.GetNbins() + 2) * (fYaxis.GetNbins() + 2);
   }
   fNcells = n;
   TArrayD::Set(n);
}

GHSymD& GHSymD::operator=(const GHSymD& h1)
{
   // Operator =

   if(this != &h1) {
      const_cast<GHSymD&>(h1).Copy(*this);
   }
   return *this;
}

GHSymD operator*(Float_t c1, GHSymD& h1)
{
   // Operator *

   GHSymD hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymD operator+(GHSymD& h1, GHSymD& h2)
{
   // Operator +

   GHSymD hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymD operator-(GHSymD& h1, GHSymD& h2)
{
   // Operator -

   GHSymD hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymD operator*(GHSymD& h1, GHSymD& h2)
{
   // Operator *

   GHSymD hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GHSymD operator/(GHSymD& h1, GHSymD& h2)
{
   // Operator /

   GHSymD hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

//------------------------------------------------------------
