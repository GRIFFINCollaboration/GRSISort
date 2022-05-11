#include "GCube.h"

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
#include "TF3.h"
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

ClassImp(GCube)

GCube::GCube()
{
   fDimension = 3;
   fTsumwy    = 0;
   fTsumwy2   = 0;
   fTsumwxy   = 0;
   fMatrix    = nullptr;
}

// we have to repeat the code from the default constructor here, because calling the TH1 constructor and the GCube
// default constructor gives an error
GCube::GCube(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : TH1(name, title, nbins, low, up)
{
   fDimension = 3;
   fTsumwy    = 0;
   fTsumwy2   = 0;
   fTsumwxy   = 0;
   fMatrix    = nullptr;
   fYaxis.Set(nbins, low, up);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)*((nbins+2)+2)/6 cells
   fNcells = (fNcells * (nbins + 3) * (nbins + 4)) / 6;
}

GCube::GCube(const char* name, const char* title, Int_t nbins, const Double_t* bins) : TH1(name, title, nbins, bins)
{
   fDimension = 3;
   fTsumwy    = 0;
   fTsumwy2   = 0;
   fTsumwxy   = 0;
   fMatrix    = nullptr;
   fYaxis.Set(nbins, bins);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)*((nbins+2)+2)/6 cells
   fNcells = (fNcells * (nbins + 3) * (nbins + 4)) / 6;
}

GCube::GCube(const char* name, const char* title, Int_t nbins, const Float_t* bins) : TH1(name, title, nbins, bins)
{
   fDimension = 3;
   fTsumwy    = 0;
   fTsumwy2   = 0;
   fTsumwxy   = 0;
   fMatrix    = nullptr;
   fYaxis.Set(nbins, bins);
   // TH1 constructor sets fNcells to nbins+2
   // we need (nbins+2)*((nbins+2)+1)*((nbins+2)+2)/6 cells
   fNcells = (fNcells * (nbins + 3) * (nbins + 4)) / 6;
}

GCube::GCube(const GCube& rhs) : TH1()
{
   rhs.Copy(*this);
}

GCube::~GCube() = default;

Int_t GCube::BufferEmpty(Int_t action)
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

   Int_t nbEntries = static_cast<Int_t>(fBuffer[0]);
   if(nbEntries == 0) {
      return 0;
   }
   if(nbEntries < 0 && action == 0) {
      return 0; // histogram has been already filled from the buffer
   }
   Double_t* buffer = fBuffer;
   if(nbEntries < 0) {
      nbEntries = -nbEntries;
      fBuffer   = nullptr;
      Reset("ICES");
      fBuffer = buffer;
   }

   if(CanExtendAllAxes() || fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin() ||
      fZaxis.GetXmax() <= fZaxis.GetXmin()) {
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
         Double_t x = fBuffer[4 * i + 2];
         if(x < min) {
            min = x;
         }
         if(x > max) {
            max = x;
         }
         Double_t y = fBuffer[4 * i + 3];
         if(y < min) {
            min = y;
         }
         if(y > max) {
            max = y;
         }
         Double_t z = fBuffer[4 * i + 4];
         if(z < min) {
            min = z;
         }
         if(z > max) {
            max = z;
         }
      }
      if(fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin() ||
         fZaxis.GetXmax() <= fZaxis.GetXmin()) {
         THLimitsFinder::GetLimitsFinder()->FindGoodLimits(this, min, max, min, max, min, max);
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
         if(min < fZaxis.GetXmin()) {
            RebinAxis(min, &fZaxis);
         }
         if(max >= fZaxis.GetXmax()) {
            RebinAxis(max, &fZaxis);
         }
         fBuffer     = buffer;
         fBufferSize = keep;
      }
   }

   fBuffer = nullptr;
   for(Int_t i = 0; i < nbEntries; ++i) {
      Fill(buffer[4 * i + 2], buffer[4 * i + 3], buffer[4 * i + 4], buffer[4 * i + 1]);
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

Int_t GCube::BufferFill(Double_t x, Double_t y, Double_t z, Double_t w)
{
   if(fBuffer == nullptr) {
      return -3;
   }

   Int_t nbEntries = static_cast<Int_t>(fBuffer[0]);
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
   if(4 * nbEntries + 4 >= fBufferSize) {
      BufferEmpty(1);
      return Fill(x, y, w);
   }
   fBuffer[4 * nbEntries + 1] = w;
   fBuffer[4 * nbEntries + 2] = x;
   fBuffer[4 * nbEntries + 3] = y;
   fBuffer[4 * nbEntries + 4] = z;
   fBuffer[0] += 1;

   return -3;
}

void GCube::Copy(TObject& obj) const
{
   // Copy.

   TH1::Copy(obj);
   static_cast<GCube&>(obj).fTsumwy  = fTsumwy;
   static_cast<GCube&>(obj).fTsumwy2 = fTsumwy2;
   static_cast<GCube&>(obj).fTsumwxy = fTsumwxy;
   static_cast<GCube&>(obj).fMatrix  = nullptr;
}

Double_t GCube::DoIntegral(Int_t binx1, Int_t binx2, Int_t biny1, Int_t biny2, Int_t binz1, Int_t binz2,
                           Double_t& error, Option_t* option, Bool_t doError) const
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
   if(GetDimension() > 2) {
      Int_t nbinsz = GetNbinsZ();
      if(binz1 < 0) {
         binz1 = 0;
      }
      if(binz2 > nbinsz + 1 || binz2 < binz1) {
         binz2 = nbinsz + 1;
      }
   } else {
      binz1 = 0;
      binz2 = 0;
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
   Double_t dz       = 1.;
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
         for(Int_t binz = binz1; binz <= binz2; ++binz) {
            if(width) {
               dz = fZaxis.GetBinWidth(binz);
            }
            Int_t bin = GetBin(binx, biny, binz);
            if(width) {
               integral += GetBinContent(bin) * dx * dy * dz;
            } else {
               integral += GetBinContent(bin);
            }
            if(doError) {
               if(width) {
                  igerr2 += GetBinError(bin) * GetBinError(bin) * dx * dx * dy * dy * dz * dz;
               } else {
                  igerr2 += GetBinError(bin) * GetBinError(bin);
               }
            }
         }
      }
   }

   if(doError) {
      error = TMath::Sqrt(igerr2);
   }
   return integral;
}

Int_t GCube::Fill(Double_t)
{
   // Invalid Fill method
   Error("Fill", "Invalid signature - do nothing");
   return -1;
}

Int_t GCube::Fill(Double_t x, Double_t y, Double_t z)
{
   /// Increment cell defined by x,y,z by 1.
   if(fBuffer != nullptr) {
      return BufferFill(x, y, z, 1);
   }

   Int_t binx, biny, binz, bin;
   fEntries++;
   // go through all orderings of x,y,z to find right combination
   if(z <= y && y <= x) {
      // z, y, x
      binx = fXaxis.FindBin(x);
      biny = fYaxis.FindBin(y);
      binz = fZaxis.FindBin(z);
   } else if(z <= x && y <= x) {
      // y, z, x
      binx = fXaxis.FindBin(x);
      biny = fZaxis.FindBin(z);
      binz = fYaxis.FindBin(y);
   } else if(y <= x) { // at this stage we know that z > y and z > x if y <= x
                       // y, x, z
      binx = fZaxis.FindBin(z);
      biny = fXaxis.FindBin(x);
      binz = fYaxis.FindBin(y);
   } else if(z <= x) { // at this stage we know that y > x
                       // z, x, y
      binx = fYaxis.FindBin(y);
      biny = fXaxis.FindBin(x);
      binz = fZaxis.FindBin(z);
   } else if(z <= y) { // at this stage we know that y > x
                       // x, z, y
      binx = fYaxis.FindBin(y);
      biny = fZaxis.FindBin(z);
      binz = fXaxis.FindBin(x);
   } else {
      // x, y, z
      binx = fZaxis.FindBin(z);
      biny = fYaxis.FindBin(y);
      binz = fXaxis.FindBin(x);
   }

   if(binx < 0 || biny < 0 || binz < 0) {
      return -1;
   }
   bin = biny * (2 * fXaxis.GetNbins() - biny + 3) / 2 + binx - binz +
         (fXaxis.GetNbins() + 2) * (fXaxis.GetNbins() + 3) * (fXaxis.GetNbins() + 4) / 6 -
         (fXaxis.GetNbins() + 2 - binz) * (fXaxis.GetNbins() + 3 - binz) * (fXaxis.GetNbins() + 4 - binz) / 6 - biny;
   std::cout<<"binx,y,z = "<<binx<<","<<biny<<","<<binz<<" => bin = "<<bin<<std::endl;
   bin = binx + biny * (fXaxis.GetNbins() - (biny + 1.) / 2.) +
         binz * (binz / 2. * (binz / 3. - fXaxis.GetNbins() + 3.) + fXaxis.GetNbins() * (3 + fXaxis.GetNbins() / 2.) +
                 10. / 3.);
   std::cout<<"binx,y,z = "<<binx<<","<<biny<<","<<binz<<" => bin = "<<bin<<std::endl;
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
   if(binz == 0 || binz > fZaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   // not sure if these summed weights are calculated correct
   // as of now this is the method used in TH3
   ++fTsumw;
   ++fTsumw2;
   fTsumwx += x;
   fTsumwx2 += x * x;
   fTsumwy += y;
   fTsumwy2 += y * y;
   fTsumwxy += x * y;
   fTsumwz += z;
   fTsumwz2 += z * z;
   fTsumwxz += x * z;
   fTsumwyz += y * z;

   return bin;
}

Int_t GCube::Fill(Double_t x, Double_t y, Double_t z, Double_t w)
{
   /// Increment cell defined by x,y,z by w.
   if(fBuffer != nullptr) {
      return BufferFill(x, y, z, 1);
   }

   Int_t binx, biny, binz, bin;
   fEntries++;
   // go through all orderings of x,y,z to find right combination
   if(z <= y && y <= x) {
      // z, y, x
      binx = fXaxis.FindBin(x);
      biny = fYaxis.FindBin(y);
      binz = fZaxis.FindBin(z);
   } else if(z <= x && y <= x) {
      // y, z, x
      binx = fXaxis.FindBin(x);
      biny = fZaxis.FindBin(z);
      binz = fYaxis.FindBin(y);
   } else if(y <= x) { // at this stage we know that z > y and z > x if y <= x
                       // y, x, z
      binx = fZaxis.FindBin(z);
      biny = fXaxis.FindBin(x);
      binz = fYaxis.FindBin(y);
   } else if(z <= x) { // at this stage we know that y > x
                       // z, x, y
      binx = fYaxis.FindBin(y);
      biny = fXaxis.FindBin(x);
      binz = fZaxis.FindBin(z);
   } else if(z <= y) { // at this stage we know that y > x
                       // x, z, y
      binx = fYaxis.FindBin(y);
      biny = fZaxis.FindBin(z);
      binz = fXaxis.FindBin(x);
   } else {
      // x, y, z
      binx = fZaxis.FindBin(z);
      biny = fYaxis.FindBin(y);
      binz = fXaxis.FindBin(x);
   }

   if(binx < 0 || biny < 0 || binz < 0) {
      return -1;
   }
   bin = binx + biny * (fXaxis.GetNbins() - (biny + 1.) / 2.) +
         binz * (binz / 2. * (binz / 3. - fXaxis.GetNbins() + 3.) + fXaxis.GetNbins() * (3 + fXaxis.GetNbins() / 2.) +
                 10. / 3.);
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
   if(binz == 0 || binz > fZaxis.GetNbins()) {
      if(!fgStatOverflows) {
         return -1;
      }
   }
   // not sure if these summed weights are calculated correct
   // as of now this is the method used in TH3
   fTsumw += w;
   fTsumw2 += w * w;
   fTsumwx += w * x;
   fTsumwx2 += w * x * x;
   fTsumwy += w * y;
   fTsumwy2 += w * y * y;
   fTsumwxy += w * x * y;
   fTsumwz += w * z;
   fTsumwz2 += w * z * z;
   fTsumwxz += w * x * z;
   fTsumwyz += w * y * z;

   return bin;
}

Int_t GCube::Fill(const char* namex, const char* namey, const char* namez, Double_t w)
{
   // Increment cell defined by namex,namey,namez by a weight w
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

   Int_t binx, biny, binz, bin;
   fEntries++;
   binx = fXaxis.FindBin(namex);
   biny = fYaxis.FindBin(namey);
   binz = fZaxis.FindBin(namez);
   if(binx < 0 || biny < 0 || binz < 0) {
      return -1;
   }
   // sort so that binx >= biny >= binz
   if(binx < biny) {
      std::swap(binx, biny);
   }
   if(binx < binz) {
      std::swap(binx, binz);
   }
   if(biny < binz) {
      std::swap(biny, binz);
   }

   bin = binx + biny * (fXaxis.GetNbins() - (biny + 1.) / 2.) +
         binz * (binz / 2. * (binz / 3. - fXaxis.GetNbins() + 3.) + fXaxis.GetNbins() * (3 + fXaxis.GetNbins() / 2.) +
                 10. / 3.);
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
   Double_t z = fYaxis.GetBinCenter(binz);
   fTsumw += w;
   fTsumw2 += w * w;
   fTsumwx += w * x;
   fTsumwx2 += w * x * x;
   fTsumwy += w * y;
   fTsumwy2 += w * y * y;
   fTsumwxy += w * x * y;
   fTsumwz += w * z;
   fTsumwz2 += w * z * z;
   fTsumwxz += w * x * z;
   fTsumwyz += w * y * z;
   return bin;
}

void GCube::FillRandom(const char* fname, Int_t ntimes, TRandom* rng)
{
   ///*-*-*-*-*-*-*Fill histogram following distribution in function fname*-*-*-*
   ///*-*          =======================================================
   ///*-*
   ///*-*   The distribution contained in the function fname (TF2) is integrated
   ///*-*   over the channel contents.
   ///*-*   It is normalized to 1.
   ///*-*   Getting one random number implies:
   ///*-*     - Generating a random number between 0 and 1 (say r1)
   ///*-*     - Look in which bin in the normalized integral r1 corresponds to
   ///*-*     - Fill histogram channel
   ///*-*   ntimes random numbers are generated
   ///*-*
   ///*-*  One can also call TF2::GetRandom2 to get a random variate from a function.
   ///*-*
   ///*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   Int_t    bin, binx, biny, binz, ibin;
   Double_t r1, x, y, z;
   //*-*- Search for fname in the list of ROOT defined functions
   TObject* fobj = gROOT->GetFunction(fname);
   if(fobj == nullptr) {
      Error("FillRandom", "Unknown function: %s", fname);
      return;
   }
   TF3* f1 = static_cast<TF3*>(fobj);
   if(f1 == nullptr) {
      Error("FillRandom", "Function: %s is not a TF3", fname);
      return;
   }

   //*-*- Allocate temporary space to store the integral and compute integral
   Int_t nbinsx = GetNbinsX();
   Int_t nbinsy = GetNbinsY();
   Int_t nbinsz = GetNbinsZ();
   Int_t nxy    = nbinsx * nbinsy;
   Int_t nbins  = nxy * nbinsz;

   auto* integral = new Double_t[nbins + 1];
   ibin           = 0;
   integral[ibin] = 0;
   for(binz = 1; binz <= nbinsz; ++binz) {
      for(biny = 1; biny <= nbinsy; ++biny) {
         for(binx = 1; binx <= nbinsx; ++binx) {
            ++ibin;
            Double_t fint =
               f1->Integral(fXaxis.GetBinLowEdge(binx), fXaxis.GetBinUpEdge(binx), fYaxis.GetBinLowEdge(biny),
                            fYaxis.GetBinUpEdge(biny), fZaxis.GetBinLowEdge(binz), fZaxis.GetBinUpEdge(binz));
            integral[ibin] = integral[ibin - 1] + fint;
         }
      }
   }

   //*-*- Normalize integral to 1
   if(integral[nbins] == 0) {
      delete[] integral;
      Error("FillRandom", "Integral = zero");
      return;
   }
   for(bin = 1; bin <= nbins; ++bin) {
      integral[bin] /= integral[nbins];
   }

   //*-*--------------Start main loop ntimes
   for(int loop = 0; loop < ntimes; ++loop) {
      r1   = (rng != nullptr) ? rng->Rndm(loop) : gRandom->Rndm(loop);
      ibin = TMath::BinarySearch(nbins, &integral[0], r1);
      binz = ibin / nxy;
      biny = (ibin - nxy * binz) / nbinsx;
      binx = 1 + ibin - nbinsx * (biny + nbinsy * binz);
      ++biny;
      x = fXaxis.GetBinCenter(binx);
      y = fYaxis.GetBinCenter(biny);
      z = fZaxis.GetBinCenter(binz);
      Fill(x, y, z);
   }
   delete[] integral;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
void GCube::FillRandom(TH1* h, Int_t ntimes, TRandom*)
#else
void GCube::FillRandom(TH1* h, Int_t ntimes, TRandom* rng)
#endif
{
   ///*-*-*-*-*-*-*Fill histogram following distribution in histogram h*-*-*-*
   ///*-*          ====================================================
   ///*-*
   ///*-*   The distribution contained in the histogram h (TH2) is integrated
   ///*-*   over the channel contents.
   ///*-*   It is normalized to 1.
   ///*-*   Getting one random number implies:
   ///*-*     - Generating a random number between 0 and 1 (say r1)
   ///*-*     - Look in which bin in the normalized integral r1 corresponds to
   ///*-*     - Fill histogram channel
   ///*-*   ntimes random numbers are generated
   ///*-*
   ///*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-**-*-*-*-*-*-*-*

   if(h == nullptr) {
      Error("FillRandom", "Null histogram");
      return;
   }
   if(fDimension != h->GetDimension()) {
      Error("FillRandom", "Histograms with different dimensions");
      return;
   }

   if(h->ComputeIntegral() == 0) {
      return;
   }

   Double_t x, y, z;
   TH3*     h3 = static_cast<TH3*>(h);
   for(int loop = 0; loop < ntimes; ++loop) {
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 24, 0)
      h3->GetRandom3(x, y, z);
#else
      h3->GetRandom3(x, y, z, rng);
#endif
      Fill(x, y, z);
   }
}

Int_t GCube::FindFirstBinAbove(Double_t threshold, Int_t axis, Int_t firstBin, Int_t lastBin) const
{
   /// find first bin with content > threshold for axis (1=x, 2=y, 3=z)
   /// if no bins with content > threshold is found the function returns -1.

   if(axis < 1 || axis > 3) {
      Warning("FindFirstBinAbove", "Invalid axis number : %d, axis x assumed\n", axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsx) nbinsx = lastBin;
   Int_t nbinsy = fYaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsy) nbinsy = lastBin;
   Int_t nbinsz = fZaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsz) nbinsz = lastBin;
   if(axis == 1) {
      for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
         for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
            for(Int_t binz = firstBin; binz <= nbinsz; ++binz) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return binx;
               }
            }
         }
      }
   } else if(axis == 2) {
      for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            for(Int_t binz = firstBin; binz <= nbinsz; ++binz) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return biny;
               }
            }
         }
      }
   } else {
      for(Int_t binz = firstBin; binz <= nbinsz; ++binz) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return binz;
               }
            }
         }
      }
   }
   return -1;
}

Int_t GCube::FindLastBinAbove(Double_t threshold, Int_t axis, Int_t firstBin, Int_t lastBin) const
{
   // find last bin with content > threshold for axis (1=x, 2=y, 3=z)
   // if no bins with content > threshold is found the function returns -1.

   if(axis < 1 || axis > 3) {
      Warning("FindLastBinAbove", "Invalid axis number : %d, axis x assumed\n", axis);
      axis = 1;
   }
   Int_t nbinsx = fXaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsx) nbinsx = lastBin;
   Int_t nbinsy = fYaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsy) nbinsy = lastBin;
   Int_t nbinsz = fZaxis.GetNbins();
	if(lastBin > firstBin && lastBin < nbinsz) nbinsz = lastBin;
   if(axis == 1) {
      for(Int_t binx = nbinsx; binx >= firstBin; --binx) {
         for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
            for(Int_t binz = firstBin; binz <= nbinsz; ++binz) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return binx;
               }
            }
         }
      }
   } else if(axis == 2) {
      for(Int_t biny = nbinsy; biny >= firstBin; --biny) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            for(Int_t binz = firstBin; binz <= nbinsz; ++binz) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return biny;
               }
            }
         }
      }
   } else {
      for(Int_t binz = nbinsz; binz >= firstBin; --binz) {
         for(Int_t binx = firstBin; binx <= nbinsx; ++binx) {
            for(Int_t biny = firstBin; biny <= nbinsy; ++biny) {
               if(GetBinContent(binx, biny, binz) > threshold) {
                  return binz;
               }
            }
         }
      }
   }
   return -1;
}

////////////////////////////////////////////////////////////////////////////////
/// Project slices along Z in case of a 3-D histogram, then fit each slice
/// with function f1 and make a 2-d histogram for each fit parameter
/// Only cells in the bin range [binminx,binmaxx] and [binminy,binmaxy] are considered.
/// if f1=0, a gaussian is assumed
/// Before invoking this function, one can set a subrange to be fitted along Z
/// via f1->SetRange(zmin,zmax)
/// The argument option (default="QNR") can be used to change the fit options.
///     "Q" means Quiet mode
///     "N" means do not show the result of the fit
///     "R" means fit the function in the specified function range
///
/// Note that the generated histograms are added to the list of objects
/// in the current directory. It is the user's responsability to delete
/// these histograms.
///
///  Example: Assume a 3-d histogram h3
///   Root > h3->FitSlicesZ(); produces 4 TH2D histograms
///          with h3_0 containing parameter 0(Constant) for a Gaus fit
///                    of each cell in X,Y projected along Z
///          with h3_1 containing parameter 1(Mean) for a gaus fit
///          with h3_2 containing parameter 2(StdDev)  for a gaus fit
///          with h3_chi2 containing the chisquare/number of degrees of freedom for a gaus fit
///
///   Root > h3->Fit(0,15,22,0,0,10);
///          same as above, but only for bins 15 to 22 along X
///          and only for cells in X,Y for which the corresponding projection
///          along Z has more than cut bins filled.
///
///  NOTE: To access the generated histograms in the current directory, do eg:
///     TH2D *h3_1 = (TH2D*)gDirectory->Get("h3_1");

void GCube::FitSlicesZ(TF1* f1, Int_t binminx, Int_t binmaxx, Int_t binminy, Int_t binmaxy, Int_t cut, Option_t* option)
{
   Int_t nbinsx = fXaxis.GetNbins();
   Int_t nbinsy = fYaxis.GetNbins();
   Int_t nbinsz = fZaxis.GetNbins();
   if(binminx < 1) {
      binminx = 1;
   }
   if(binmaxx > nbinsx) {
      binmaxx = nbinsx;
   }
   if(binmaxx < binminx) {
      binminx = 1;
      binmaxx = nbinsx;
   }
   if(binminy < 1) {
      binminy = 1;
   }
   if(binmaxy > nbinsy) {
      binmaxy = nbinsy;
   }
   if(binmaxy < binminy) {
      binminy = 1;
      binmaxy = nbinsy;
   }

   // default is to fit with a gaussian
   if(f1 == nullptr) {
      f1 = static_cast<TF1*>(gROOT->GetFunction("gaus"));
      if(f1 == nullptr) {
         f1 = new TF1("gaus", "gaus", fZaxis.GetXmin(), fZaxis.GetXmax());
      } else {
         f1->SetRange(fZaxis.GetXmin(), fZaxis.GetXmax());
      }
   }
   const char* fname   = f1->GetName();
   Int_t       npar    = f1->GetNpar();
   auto*       parsave = new Double_t[npar];
   f1->GetParameters(parsave);

   // Create one 2-d histogram for each function parameter
   Int_t          ipar;
   char           name[80], title[80];
   TH2D*          hlist[25];
   const TArrayD* xbins = fXaxis.GetXbins();
   const TArrayD* ybins = fYaxis.GetXbins();
   for(ipar = 0; ipar < npar; ++ipar) {
      snprintf(name, 80, "%s_%d", GetName(), ipar);
      snprintf(title, 80, "Fitted value of par[%d]=%s", ipar, f1->GetParName(ipar));
      if(xbins->fN == 0) {
         hlist[ipar] = new TH2D(name, title, nbinsx, fXaxis.GetXmin(), fXaxis.GetXmax(), nbinsy, fYaxis.GetXmin(),
                                fYaxis.GetXmax());
      } else {
         hlist[ipar] = new TH2D(name, title, nbinsx, xbins->fArray, nbinsy, ybins->fArray);
      }
      hlist[ipar]->GetXaxis()->SetTitle(fXaxis.GetTitle());
      hlist[ipar]->GetYaxis()->SetTitle(fYaxis.GetTitle());
   }
   snprintf(name, 80, "%s_chi2", GetName());
   auto* hchi2 = new TH2D(name, "chisquare", nbinsx, fXaxis.GetXmin(), fXaxis.GetXmax(), nbinsy, fYaxis.GetXmin(),
                          fYaxis.GetXmax());

   // Loop on all cells in X,Y generate a projection along Z
   auto* hpz = new TH1D("R_temp", "_temp", nbinsz, fZaxis.GetXmin(), fZaxis.GetXmax());
   Int_t bin, binx, biny, binz;
   for(biny = binminy; biny <= binmaxy; biny++) {
      Float_t y = fYaxis.GetBinCenter(biny);
      for(binx = binminx; binx <= binmaxx; binx++) {
         Float_t x = fXaxis.GetBinCenter(binx);
         hpz->Reset();
         Int_t nfill = 0;
         for(binz = 1; binz <= nbinsz; binz++) {
            bin       = GetBin(binx, biny, binz);
            Float_t w = RetrieveBinContent(bin);
            if(w == 0) {
               continue;
            }
            hpz->Fill(fZaxis.GetBinCenter(binz), w);
            hpz->SetBinError(binz, GetBinError(bin));
            nfill++;
         }
         if(nfill < cut) {
            continue;
         }
         f1->SetParameters(parsave);
         hpz->Fit(fname, option);
         Int_t npfits = f1->GetNumberFitPoints();
         if(npfits > npar && npfits >= cut) {
            for(ipar = 0; ipar < npar; ipar++) {
               hlist[ipar]->Fill(x, y, f1->GetParameter(ipar));
               hlist[ipar]->SetBinError(binx, biny, f1->GetParError(ipar));
            }
            hchi2->SetBinContent(binx, biny, f1->GetChisquare() / (npfits - npar));
         }
      }
   }
   delete[] parsave;
   delete hpz;
}

Int_t GCube::GetBin(Int_t binx, Int_t biny, Int_t binz) const
{
   Int_t n = fXaxis.GetNbins() + 2;
   if(binx < 0) {
      binx = 0;
   }
   if(binx >= n) {
      binx = n - 1;
   }
   if(biny < 0) {
      biny = 0;
   }
   if(biny >= n) {
      biny = n - 1;
   }
   if(binz < 0) {
      binz = 0;
   }
   if(binz >= n) {
      binz = n - 1;
   }
   // sort so that binx >= biny >= binz
   if(binx < biny) {
      std::swap(binx, biny);
   }
   if(binx < binz) {
      std::swap(binx, binz);
   }
   if(biny < binz) {
      std::swap(biny, binz);
   }

   return binx + biny * (fXaxis.GetNbins() - (biny + 1.) / 2.) +
          binz * (binz / 2. * (binz / 3. - fXaxis.GetNbins() + 3.) + fXaxis.GetNbins() * (3 + fXaxis.GetNbins() / 2.) +
                  10. / 3.);
}

Double_t GCube::GetBinWithContent2(Double_t c, Int_t& binx, Int_t& biny, Int_t& binz, Int_t firstxbin, Int_t lastxbin,
                                   Int_t firstybin, Int_t lastybin, Int_t firstzbin, Int_t lastzbin,
                                   Double_t maxdiff) const
{
   // compute first cell (binx,biny,binz) in the range [firstxbin,lastxbin][firstybin,lastybin][firstzbin,lastzbin]
   // for which diff = abs(cell_content-c) <= maxdiff
   // In case several cells in the specified range with diff=0 are found
   // the first cell found is returned in binx,biny,binz.
   // In case several cells in the specified range satisfy diff <=maxdiff
   // the cell with the smallest difference is returned in binx,biny,binz.
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
   //        if firstzbin < 0, firstzbin is set to 1
   //        if(lastzbin < firstzbin then lastzbin is set to the number of bins in Z
   //          ie if firstzbin=1 and lastzbin=0 (default) the search is on all bins in Z except
   //          for Z's under- and overflow bins.
   // NOTE2: if maxdiff=0 (default), the first cell with content=c is returned.

   if(fDimension != 3) {
      binx = -1;
      biny = -1;
      binz = -1;
      Error("GetBinWithContent2", "function is only valid for 3-D histograms");
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
   if(firstzbin < 0) {
      firstzbin = 1;
   }
   if(lastzbin < firstzbin) {
      lastzbin = fZaxis.GetNbins();
   }
   Double_t diff, curmax = 1.e240;
   for(Int_t k = firstzbin; k <= lastzbin; k++) {
      for(Int_t j = firstybin; j <= lastybin; j++) {
         for(Int_t i = firstxbin; i <= lastxbin; i++) {
            diff = TMath::Abs(GetBinContent(i, j, k) - c);
            if(diff <= 0) {
               binx = i;
               biny = j;
               binz = k;
               return diff;
            }
            if(diff < curmax && diff <= maxdiff) {
               binx   = i;
               biny   = j;
               binz   = k;
               curmax = diff;
            }
         }
      }
   }
   return curmax;
}

Double_t GCube::GetCorrelationFactor(Int_t axis1, Int_t axis2) const
{
   ///*-*-*-*-*-*-*-*Return correlation factor between axis1 and axis2*-*-*-*-*
   ///*-*            ====================================================
   if(axis1 < 1 || axis2 < 1 || axis1 > 3 || axis2 > 3) {
      Error("GetCorrelationFactor", "Wrong parameters");
      return 0;
   }
   if(axis1 == axis2) {
      return 1;
   }
   Double_t stddev1 = GetStdDev(axis1);
   if(stddev1 == 0) {
      return 0;
   }
   Double_t stddev2 = GetStdDev(axis2);
   if(stddev2 == 0) {
      return 0;
   }
   return GetCovariance(axis1, axis2) / stddev1 / stddev2;
}

Double_t GCube::GetCovariance(Int_t axis1, Int_t axis2) const
{
   ///*-*-*-*-*-*-*-*Return covariance between axis1 and axis2*-*-*-*-*
   ///*-*            ====================================================

   if(axis1 < 1 || axis2 < 1 || axis1 > 3 || axis2 > 3) {
      Error("GetCovariance", "Wrong parameters");
      return 0;
   }
   Double_t stats[kNstat];
   GetStats(stats);
   Double_t sumw   = stats[0];
   Double_t sumw2  = stats[1];
   Double_t sumwx  = stats[2];
   Double_t sumwx2 = stats[3];
   Double_t sumwy  = stats[4];
   Double_t sumwy2 = stats[5];
   Double_t sumwxy = stats[6];
   Double_t sumwz  = stats[7];
   Double_t sumwz2 = stats[8];
   Double_t sumwxz = stats[9];
   Double_t sumwyz = stats[10];

   if(sumw == 0) {
      return 0;
   }
   if(axis1 == 1 && axis2 == 1) {
      return TMath::Abs(sumwx2 / sumw - sumwx * sumwx / sumw2);
   }
   if(axis1 == 2 && axis2 == 2) {
      return TMath::Abs(sumwy2 / sumw - sumwy * sumwy / sumw2);
   }
   if(axis1 == 3 && axis2 == 3) {
      return TMath::Abs(sumwz2 / sumw - sumwz * sumwz / sumw2);
   }
   if((axis1 == 1 && axis2 == 2) || (axis1 == 2 && axis2 == 1)) {
      return sumwxy / sumw - sumwx / sumw * sumwy / sumw;
   }
   if((axis1 == 1 && axis2 == 3) || (axis1 == 3 && axis2 == 1)) {
      return sumwxz / sumw - sumwx / sumw * sumwz / sumw;
   }
   if((axis1 == 2 && axis2 == 3) || (axis1 == 3 && axis2 == 2)) {
      return sumwyz / sumw - sumwy / sumw * sumwz / sumw;
   }
   return 0;
}

void GCube::GetRandom3(Double_t& x, Double_t& y, Double_t& z)
{
   // return 3 random numbers along axis x, y, and z distributed according
   // the cellcontents of a 3-dim histogram
   // return a NaN if the histogram has a bin with negative content

   Int_t    nbinsx = GetNbinsX();
   Int_t    nbinsy = GetNbinsY();
   Int_t    nbinsz = GetNbinsZ();
   Int_t    nxy    = nbinsx * nbinsy;
   Int_t    nbins  = nxy * nbinsz;
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
      z = 0;
      return;
   }
   // case histogram has negative bins
   if(integral == TMath::QuietNaN()) {
      x = TMath::QuietNaN();
      y = TMath::QuietNaN();
      z = TMath::QuietNaN();
      return;
   }

   Double_t r1   = gRandom->Rndm();
   Int_t    ibin = TMath::BinarySearch(nbins, fIntegral, r1);
   Int_t    binz = ibin / nxy;
   Int_t    biny = (ibin - nxy * binz) / nbinsx;
   Int_t    binx = ibin - nbinsx * (biny + nbinsy * binz);
   x             = fXaxis.GetBinLowEdge(binx + 1);
   if(r1 > fIntegral[ibin]) {
      x += fXaxis.GetBinWidth(binx + 1) * (r1 - fIntegral[ibin]) / (fIntegral[ibin + 1] - fIntegral[ibin]);
   }
   y = fYaxis.GetBinLowEdge(biny + 1) + fYaxis.GetBinWidth(biny + 1) * gRandom->Rndm();
   z = fZaxis.GetBinLowEdge(binz + 1) + fZaxis.GetBinWidth(binz + 1) * gRandom->Rndm();
}

void GCube::GetStats(Double_t* stats) const
{
   /// fill the array stats from the contents of this histogram
   /// The array stats must be correctly dimensionned in the calling program.
   /// stats[0]  = sumw
   /// stats[1]  = sumw2
   /// stats[2]  = sumwx
   /// stats[3]  = sumwx2
   /// stats[4]  = sumwy
   /// stats[5]  = sumwy2
   /// stats[6]  = sumwxy
   /// stats[7]  = sumwz
   /// stats[8]  = sumwz2
   /// stats[9]  = sumwxz
   /// stats[10] = sumwyz
   ///
   /// If no axis-subranges are specified (via TAxis::SetRange), the array stats
   /// is simply a copy of the statistics quantities computed at filling time.
   /// If sub-ranges are specified, the function recomputes these quantities
   /// from the bin contents in the current axis ranges.
   ///
   ///  Note that the mean value/RMS is computed using the bins in the currently
   ///  defined ranges (see TAxis::SetRange). By default the ranges include
   ///  all bins from 1 to nbins included, excluding underflows and overflows.
   ///  To force the underflows and overflows in the computation, one must
   ///  call the static function TH1::StatOverflows(kTRUE) before filling
   ///  the histogram.

   if(fBuffer != nullptr) {
      const_cast<GCube*>(this)->BufferEmpty();
   }

   Int_t    bin, binx, biny, binz;
   Double_t w, err;
   Double_t x, y, z;
   if((fTsumw == 0 && fEntries > 0) || fXaxis.TestBit(TAxis::kAxisRange) || fYaxis.TestBit(TAxis::kAxisRange) ||
      fZaxis.TestBit(TAxis::kAxisRange)) {
      for(bin = 0; bin < 7; ++bin) {
         stats[bin] = 0;
      }

      Int_t firstBinX = fXaxis.GetFirst();
      Int_t lastBinX  = fXaxis.GetLast();
      Int_t firstBinY = fYaxis.GetFirst();
      Int_t lastBinY  = fYaxis.GetLast();
      Int_t firstBinZ = fZaxis.GetFirst();
      Int_t lastBinZ  = fZaxis.GetLast();
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
         if(!fZaxis.TestBit(TAxis::kAxisRange)) {
            if(firstBinZ == 1) {
               firstBinZ = 0;
            }
            if(lastBinZ == fZaxis.GetNbins()) {
               lastBinZ += 1;
            }
         }
      }
      for(binz = firstBinZ; binz <= lastBinZ; ++binz) {
         z = fZaxis.GetBinCenter(binz);
         for(biny = firstBinY; biny <= lastBinY; ++biny) {
            y = fYaxis.GetBinCenter(biny);
            for(binx = firstBinX; binx <= lastBinX; ++binx) {
               bin = GetBin(binx, biny, binz);
               x   = fXaxis.GetBinCenter(binx);
               w   = GetBinContent(bin);
               err = TMath::Abs(GetBinError(bin));
               stats[0] += w;
               stats[1] += err * err;
               stats[2] += w * x;
               stats[3] += w * x * x;
               stats[4] += w * y;
               stats[5] += w * y * y;
               stats[6] += w * x * y;
               stats[7] += w * z;
               stats[8] += w * z * z;
               stats[9] += w * x * z;
               stats[10] += w * y * z;
            }
         }
      }
   } else {
      stats[0]  = fTsumw;
      stats[1]  = fTsumw2;
      stats[2]  = fTsumwx;
      stats[3]  = fTsumwx2;
      stats[4]  = fTsumwy;
      stats[5]  = fTsumwy2;
      stats[6]  = fTsumwxy;
      stats[7]  = fTsumwz;
      stats[8]  = fTsumwz2;
      stats[9]  = fTsumwxz;
      stats[10] = fTsumwyz;
   }
}

Double_t GCube::Integral(Option_t* option) const
{
   // Return integral of bin contents. Only bins in the bins range are considered.
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x and in y.

   return Integral(fXaxis.GetFirst(), fXaxis.GetLast(), fYaxis.GetFirst(), fYaxis.GetLast(), fZaxis.GetFirst(),
                   fZaxis.GetLast(), option);
}

Double_t GCube::Integral(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Int_t firstzbin,
                         Int_t lastzbin, Option_t* option) const
{
   // Return integral of bin contents in range [firstxbin,lastxbin],[firstybin,lastybin],[firstzbin,lastzbin]
   // for a 3-D histogram
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x, y, and z.
   double err = 0;
   return DoIntegral(firstxbin, lastxbin, firstybin, lastybin, firstzbin, lastzbin, err, option);
}

Double_t GCube::IntegralAndError(Int_t firstxbin, Int_t lastxbin, Int_t firstybin, Int_t lastybin, Int_t firstzbin,
                                 Int_t lastzbin, Double_t& error, Option_t* option) const
{
   // Return integral of bin contents in range [firstxbin,lastxbin],[firstybin,lastybin],[firstzbin,lastzbin]
   // for a 3-D histogram. Calculates also the integral error using error propagation
   // from the bin errors assumming that all the bins are uncorrelated.
   // By default the integral is computed as the sum of bin contents in the range.
   // if option "width" is specified, the integral is the sum of
   // the bin contents multiplied by the bin width in x, y, and z.

   return DoIntegral(firstxbin, lastxbin, firstybin, lastybin, firstzbin, lastzbin, error, option, kTRUE);
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GCube::Interpolate(Double_t)
#else
Double_t GCube::Interpolate(Double_t) const
#endif
{
   // illegal for a TH3
   Error("Interpolate", "This function must be called with 3 arguments for a TH3");
   return 0;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GCube::Interpolate(Double_t, Double_t)
#else
Double_t GCube::Interpolate(Double_t, Double_t) const
#endif
{
   // illegal for a TH3
   Error("Interpolate", "This function must be called with 3 arguments for a TH3");
   return 0;
}

#if ROOT_VERSION_CODE < ROOT_VERSION(6, 20, 0)
Double_t GCube::Interpolate(Double_t x, Double_t y, Double_t z)
#else
Double_t GCube::Interpolate(Double_t x, Double_t y, Double_t z) const
#endif
{
   /// Given a point P(x,y,z), Interpolate approximates the value via trilinear interpolation
   /// based on the 8 nearest bin center points ( corner of the cube surronding the points)
   /// The Algorithm is described in http://en.wikipedia.org/wiki/Trilinear_interpolation
   /// The given values (x,y,z) must be between first bin center and  last bin center for each coordinate:
   ///
   ///   fXAxis.GetBinCenter(1) < x  < fXaxis.GetBinCenter(nbinX)     AND
   ///   fYAxis.GetBinCenter(1) < y  < fYaxis.GetBinCenter(nbinY)     AND
   ///   fZAxis.GetBinCenter(1) < z  < fZaxis.GetBinCenter(nbinZ)

   Int_t ubx = fXaxis.FindBin(x);
   if(x < fXaxis.GetBinCenter(ubx)) {
      ubx -= 1;
   }
   Int_t obx = ubx + 1;

   Int_t uby = fYaxis.FindBin(y);
   if(y < fYaxis.GetBinCenter(uby)) {
      uby -= 1;
   }
   Int_t oby = uby + 1;

   Int_t ubz = fZaxis.FindBin(z);
   if(z < fZaxis.GetBinCenter(ubz)) {
      ubz -= 1;
   }
   Int_t obz = ubz + 1;

   if(ubx <= 0 || uby <= 0 || ubz <= 0 || obx > fXaxis.GetNbins() || oby > fYaxis.GetNbins() ||
      obz > fZaxis.GetNbins()) {
      Error("Interpolate", "Cannot interpolate outside histogram domain.");
      return 0;
   }

   Double_t xw = fXaxis.GetBinCenter(obx) - fXaxis.GetBinCenter(ubx);
   Double_t yw = fYaxis.GetBinCenter(oby) - fYaxis.GetBinCenter(uby);
   Double_t zw = fZaxis.GetBinCenter(obz) - fZaxis.GetBinCenter(ubz);

   Double_t xd = (x - fXaxis.GetBinCenter(ubx)) / xw;
   Double_t yd = (y - fYaxis.GetBinCenter(uby)) / yw;
   Double_t zd = (z - fZaxis.GetBinCenter(ubz)) / zw;

   Double_t v[] = {GetBinContent(ubx, uby, ubz), GetBinContent(ubx, uby, obz), GetBinContent(ubx, oby, ubz),
                   GetBinContent(ubx, oby, obz), GetBinContent(obx, uby, ubz), GetBinContent(obx, uby, obz),
                   GetBinContent(obx, oby, ubz), GetBinContent(obx, oby, obz)};

   Double_t i1 = v[0] * (1 - zd) + v[1] * zd;
   Double_t i2 = v[2] * (1 - zd) + v[3] * zd;
   Double_t j1 = v[4] * (1 - zd) + v[5] * zd;
   Double_t j2 = v[6] * (1 - zd) + v[7] * zd;

   Double_t w1 = i1 * (1 - yd) + i2 * yd;
   Double_t w2 = j1 * (1 - yd) + j2 * yd;

   Double_t result = w1 * (1 - xd) + w2 * xd;

   return result;
}

Double_t GCube::KolmogorovTest(const TH1* h2, Option_t* option) const
{
   ///  Statistical test of compatibility in shape between
   ///  THIS histogram and h3, using Kolmogorov test.
   ///     Default: Ignore under- and overflow bins in comparison
   ///
   ///     option is a character string to specify options
   ///         "U" include Underflows in test
   ///         "O" include Overflows
   ///         "N" include comparison of normalizations
   ///         "D" Put out a line of "Debug" printout
   ///         "M" Return the Maximum Kolmogorov distance instead of prob
   ///
   ///   The returned function value is the probability of test
   ///       (much less than one means NOT compatible)
   ///
   ///   The KS test uses the distance between the pseudo-CDF's obtained
   ///   from the histogram. Since in more than 1D the order for generating the pseudo-CDF is
   ///   arbitrary, we use the pseudo-CDF's obtained from all the possible 6 combinatons of the 3 axis.
   ///  The average of all the maximum  distances obtained is used in the tests.
   ///
   ///  Code adapted by Rene Brun from original HBOOK routine HDIFF

   TString opt = option;
   opt.ToUpper();

   Double_t prb = 0;
   TH1*     h1  = const_cast<TH1*>(static_cast<const TH1*>(this));
   if(h2 == nullptr) {
      return 0;
   }
   TAxis* xaxis1 = h1->GetXaxis();
   TAxis* xaxis2 = const_cast<TAxis*>(h2->GetXaxis());
   Int_t  nc1    = xaxis1->GetNbins();
   Int_t  nc2    = xaxis2->GetNbins();

   // Check consistency of dimensions
   if(h1->GetDimension() != 3 || h2->GetDimension() != 3) {
      Error("KolmogorovTest", "Histograms must be 3-D\n");
      return 0;
   }

   // Check consistency in number of channels
   if(nc1 != nc2 || nc1 < 1) {
      Error("KolmogorovTest", "Number of channels is different, %d and %d\n", nc1, nc2);
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

   //   Should we include Uflows, Oflows?
   Int_t ibeg = 1;
   Int_t iend = nc1;
   if(opt.Contains("U")) {
      ibeg = 0;
   }
   if(opt.Contains("O")) {
      iend = nc1 + 1;
   }

   Int_t    i, j, k;
   Double_t sum1 = 0;
   Double_t sum2 = 0;
   Double_t w1   = 0;
   Double_t w2   = 0;
   for(i = ibeg; i <= iend; ++i) {
      for(j = ibeg; j <= iend; ++j) {
         for(k = ibeg; k <= iend; ++k) {
            sum1 += h1->GetBinContent(i, j, k);
            sum2 += h2->GetBinContent(i, j, k);
            Double_t ew1 = h1->GetBinError(i, j, k);
            Double_t ew2 = h2->GetBinError(i, j, k);
            w1 += ew1 * ew1;
            w2 += ew2 * ew2;
         }
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
   Double_t esum1 = 0, esum2 = 0;
   if(w1 > 0) {
      esum1 = sum1 * sum1 / w1;
   } else {
      afunc1 = kTRUE; // use later for calculating z
   }

   if(w2 > 0) {
      esum2 = sum2 * sum2 / w2;
   } else {
      afunc2 = kTRUE; // use later for calculating z
   }

   if(afunc2 && afunc1) {
      Error("KolmogorovTest", "Errors are zero for both histograms\n");
      return 0;
   }

   //   Find Kolmogorov distance
   //   order is arbitrary take average of all possible 6 starting orders x,y,z
   int order[3] = {0, 1, 2};
   int binbeg[3];
   int binend[3];
   int ibin[3];
   int bin;
   binbeg[0] = ibeg;
   binbeg[1] = ibeg;
   binbeg[2] = ibeg;
   binend[0] = iend;
   binend[1] = iend;
   binend[2] = iend;
   Double_t vdfmax[6]; // there are in total 6 combinations
   int      icomb = 0;
   Double_t s1    = 1. / (6. * sum1);
   Double_t s2    = 1. / (6. * sum2);
   Double_t rsum1 = 0, rsum2 = 0;
   do {
      // loop on bins
      Double_t dmax = 0;
      for(i = binbeg[order[0]]; i <= binend[order[0]]; i++) {
         for(j = binbeg[order[1]]; j <= binend[order[1]]; j++) {
            for(k = binbeg[order[2]]; k <= binend[order[2]]; k++) {
               ibin[order[0]] = i;
               ibin[order[1]] = j;
               ibin[order[2]] = k;
               bin            = h1->GetBin(ibin[0], ibin[1], ibin[2]);
               rsum1 += s1 * h1->GetBinContent(bin);
               rsum2 += s2 * h2->GetBinContent(bin);
               dmax = TMath::Max(dmax, TMath::Abs(rsum1 - rsum2));
            }
         }
      }
      vdfmax[icomb] = dmax;
      icomb++;
   } while(TMath::Permute(3, order));

   // get average of distances
   Double_t dfmax = TMath::Mean(6, vdfmax);

   //    Get Kolmogorov probability
   Double_t factnm;
   if(afunc1) {
      factnm = TMath::Sqrt(sum2);
   } else if(afunc2) {
      factnm = TMath::Sqrt(sum1);
   } else {
      factnm = TMath::Sqrt(sum1 * sum2 / (sum1 + sum2));
   }
   Double_t z = dfmax * factnm;

   prb = TMath::KolmogorovProb(z);

   Double_t prb1 = 0, prb2 = 0;
   // option N to combine normalization makes sense if both afunc1 and afunc2 are false
   if(opt.Contains("N") && !(afunc1 || afunc2)) {
      // Combine probabilities for shape and normalization
      prb1          = prb;
      Double_t d12  = esum1 - esum2;
      Double_t chi2 = d12 * d12 / (esum1 + esum2);
      prb2          = TMath::Prob(chi2, 1);
      //     see Eadie et al., section 11.6.2
      if(prb > 0 && prb2 > 0) {
         prb = prb * prb2 * (1 - TMath::Log(prb * prb2));
      } else {
         prb = 0;
      }
   }

   //    debug printout
   if(opt.Contains("D")) {
      std::cout<<" Kolmo Prob  h1 = "<<h1->GetName()<<", sum1 = "<<sum1<<std::endl;
      std::cout<<" Kolmo Prob  h2 = "<<h2->GetName()<<", sum2 = "<<sum2<<std::endl;
      std::cout<<" Kolmo Probabil = "<<prb<<", Max dist = "<<dfmax<<std::endl;
      if(opt.Contains("N")) {
         std::cout<<" Kolmo Probabil = "<<prb1<<" for shape alone, "<<prb2<<" for normalisation alone"<<std::endl;
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
      return dfmax; // return avergae of max distance
   }

   return prb;
}

Long64_t GCube::Merge(TCollection* list)
{
   /// Add all histograms in the collection to this histogram.
   /// This function computes the min/max for the axes,
   /// compute a new number of bins, if necessary,
   /// add bin contents, errors and statistics.
   /// If overflows are present and limits are different the function will fail.
   /// The function returns the total number of entries in the result histogram
   /// if the merge is successfull, -1 otherwise.
   ///
   /// IMPORTANT remark. The 2 axis x and y may have different number
   /// of bins and different limits, BUT the largest bin width must be
   /// a multiple of the smallest bin width and the upper limit must also
   /// be a multiple of the bin width.

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
   TAxis  newZAxis;
   Bool_t initialLimitsFound = kFALSE;
   Bool_t allSameLimits      = kTRUE;
   Bool_t allHaveLimits      = kTRUE;
   Bool_t firstNonEmptyHist  = kTRUE;

   TIter  next(&inlist);
   GCube* h = this;
   do {
      // skip empty histgrams
      if(h->fTsumw == 0 && h->GetEntries() == 0) {
         continue;
      }

      Bool_t hasLimits = h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax();
      allHaveLimits    = allHaveLimits && hasLimits;

      if(hasLimits) {
         h->BufferEmpty();

         // this is done in case the first histograms are empty and
         // the histogram have different limits
         if(firstNonEmptyHist) {
            // set axis limits in the case the first histogram did not have limits
            if(h != this) {
               if(!SameLimitsAndNBins(fXaxis, *(h->GetXaxis()))) {
                  fXaxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
               }
               if(!SameLimitsAndNBins(fYaxis, *(h->GetYaxis()))) {
                  fYaxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
               }
               if(!SameLimitsAndNBins(fZaxis, *(h->GetZaxis()))) {
                  fZaxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax());
               }
            }
            firstNonEmptyHist = kFALSE;
         }

         if(!initialLimitsFound) {
            // this is executed the first time an histogram with limits is found
            // to set some initial values on the new axes
            initialLimitsFound = kTRUE;
            newXAxis.Set(h->GetXaxis()->GetNbins(), h->GetXaxis()->GetXmin(), h->GetXaxis()->GetXmax());
            newYAxis.Set(h->GetYaxis()->GetNbins(), h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
            newZAxis.Set(h->GetZaxis()->GetNbins(), h->GetZaxis()->GetXmin(), h->GetYaxis()->GetXmax());
         } else {
            // check first if histograms have same bins
            if(!SameLimitsAndNBins(newXAxis, *(h->GetXaxis())) || !SameLimitsAndNBins(newYAxis, *(h->GetYaxis())) ||
               !SameLimitsAndNBins(newZAxis, *(h->GetZaxis()))) {
               allSameLimits = kFALSE;
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
               if(!RecomputeAxisLimits(newYAxis, *(h->GetYaxis()))) {
                  Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                                 "first: (%d, %f, %f), second: (%d, %f, %f)",
                        newYAxis.GetNbins(), newYAxis.GetXmin(), newYAxis.GetXmax(), h->GetYaxis()->GetNbins(),
                        h->GetYaxis()->GetXmin(), h->GetYaxis()->GetXmax());
                  return -1;
               }
               if(!RecomputeAxisLimits(newZAxis, *(h->GetZaxis()))) {
                  Error("Merge", "Cannot merge histograms - limits are inconsistent:\n "
                                 "first: (%d, %f, %f), second: (%d, %f, %f)",
                        newZAxis.GetNbins(), newZAxis.GetXmin(), newZAxis.GetXmax(), h->GetZaxis()->GetNbins(),
                        h->GetZaxis()->GetXmin(), h->GetZaxis()->GetXmax());
                  return -1;
               }
            }
         }
      }
   } while((h = static_cast<GCube*>(next())) != nullptr);
   if(h == nullptr && ((*next) != nullptr)) {
      Error("Merge", "Attempt to merge object of class: %s to a %s", (*next)->ClassName(), ClassName());
      return -1;
   }
   next.Reset();

   // In the case of histogram with different limits
   // newX(Y)Axis will now have the new found limits
   // but one needs first to clone this histogram to perform the merge
   // The clone is not needed when all histograms have the same limits
   GCube* hclone = nullptr;
   if(!allSameLimits) {
      // We don't want to add the clone to gDirectory,
      // so remove our kMustCleanup bit temporarily
      Bool_t mustCleanup = TestBit(kMustCleanup);
      if(mustCleanup) {
         ResetBit(kMustCleanup);
      }
      hclone = static_cast<GCube*>(IsA()->New());
      hclone->SetDirectory(nullptr);
      Copy(*hclone);
      if(mustCleanup) {
         SetBit(kMustCleanup);
      }
      BufferEmpty(1); // To remove buffer.
      Reset();        // BufferEmpty sets limits so we can't use it later.
      SetEntries(0);
      inlist.AddFirst(hclone);
   }

   if(!allSameLimits && initialLimitsFound) {
      SetBins(newXAxis.GetNbins(), newXAxis.GetXmin(), newXAxis.GetXmax(), newYAxis.GetNbins(), newYAxis.GetXmin(),
              newYAxis.GetXmax(), newZAxis.GetNbins(), newZAxis.GetXmin(), newZAxis.GetXmax());
   }

   if(!allHaveLimits) {
      // fill this histogram with all the data from buffers of histograms without limits
      while((h = static_cast<GCube*>(next())) != nullptr) {
         if(h->GetXaxis()->GetXmin() >= h->GetXaxis()->GetXmax() && (h->fBuffer != nullptr)) {
            // no limits
            Int_t nbentries = static_cast<Int_t>(h->fBuffer[0]);
            for(Int_t i = 0; i < nbentries; i++) {
               Fill(h->fBuffer[4 * i + 2], h->fBuffer[4 * i + 3], h->fBuffer[4 * i + 4], h->fBuffer[4 * i + 1]);
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
         return static_cast<Long64_t>(GetEntries()); // all histograms have been processed
      }
      next.Reset();
   }

   // merge bin contents and errors
   Double_t stats[kNstat];
   Double_t totstats[kNstat];
   for(Int_t i = 0; i < kNstat; ++i) {
      totstats[i] = stats[i] = 0;
   }
   GetStats(totstats);
   Double_t nentries = GetEntries();
   Int_t    binx, biny, binz, ix, iy, iz, nx, ny, nz, bin, ibin;
   Double_t cu;
   Bool_t canExtend = CanExtendAllAxes();
   SetCanExtend(TH1::kNoAxis); // reset, otherwise setting the under/overflow will extend the axis

   while((h = static_cast<GCube*>(next())) != nullptr) {
      // process only if the histogram has limits; otherwise it was processed before
      if(h->GetXaxis()->GetXmin() < h->GetXaxis()->GetXmax()) {
         // import statistics
         h->GetStats(stats);
         for(Int_t i = 0; i < kNstat; ++i) {
            totstats[i] += stats[i];
         }
         nentries += h->GetEntries();

         nx = h->GetXaxis()->GetNbins();
         ny = h->GetYaxis()->GetNbins();
         nz = h->GetZaxis()->GetNbins();

         // mantain loop in separate binz, biny and binz to avoid
         // callinig FindBin(x,y,z) for every bin
         for(binz = 0; binz <= nz + 1; ++binz) {
            if(!allSameLimits) {
               iz = fZaxis.FindBin(h->GetZaxis()->GetBinCenter(binz));
            } else {
               iz = binz;
            }

            for(biny = 0; biny <= ny + 1; ++biny) {
               if(!allSameLimits) {
                  iy = fYaxis.FindBin(h->GetYaxis()->GetBinCenter(biny));
               } else {
                  iy = biny;
               }
               for(binx = 0; binx <= nx + 1; ++binx) {
                  bin = binx + (nx + 2) * (biny + (ny + 2) * binz);
                  cu  = h->GetBinContent(bin);
                  if(!allSameLimits) {
                     // look at non-empty unerflow/overflows
                     if(cu != 0 && (h->IsBinUnderflow(bin) || h->IsBinOverflow(bin))) {
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
                  ibin = GetBin(ix, iy, iz);

                  if(ibin < 0) {
                     continue;
                  }
                  AddBinContent(ibin, cu);
                  if(fSumw2.fN != 0) {
                     Double_t error1 = h->GetBinError(bin);
                     fSumw2.fArray[ibin] += error1 * error1;
                  }
               }
            }
         }
      }
   }
   if(canExtend) {
      SetCanExtend(static_cast<UInt_t>(canExtend));
   }

   // copy merged stats
   PutStats(totstats);
   SetEntries(nentries);
   if(hclone != nullptr) {
      inlist.Remove(hclone);
      delete hclone;
   }
   return static_cast<Long64_t>(nentries);
}

TH1D* GCube::Projection(const char* name, Int_t firstBiny, Int_t lastBiny, Int_t firstBinz, Int_t lastBinz,
                        Option_t* option) const
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
   opt.ToLower(); // must be called after having parsed the cut name
   bool originalRange = opt.Contains("o");

   Int_t firstXBin = fXaxis.GetFirst();
   Int_t lastXBin  = fXaxis.GetLast();

   if(firstXBin == 0 && lastXBin == 0) {
      firstXBin = 1;
      lastXBin  = fXaxis.GetNbins();
   }

   if(lastBiny < firstBiny && fYaxis.TestBit(TAxis::kAxisRange)) {
      firstBiny = fYaxis.GetFirst();
      lastBiny  = fYaxis.GetLast();
      // For special case of TAxis::SetRange, when first == 1 and last
      // = N and the range bit has been set, the TAxis will return 0
      // for both.
      if(firstBiny == 0 && lastBiny == 0) {
         firstBiny = 1;
         lastBiny  = fYaxis.GetNbins();
      }
   }
   if(firstBiny < 0) {
      firstBiny = 0;
   }
   if(lastBiny < 0) {
      lastBiny = fYaxis.GetLast() + 1;
   }
   if(lastBiny > fYaxis.GetLast() + 1) {
      lastBiny = fYaxis.GetLast() + 1;
   }

   if(lastBinz < firstBinz && fZaxis.TestBit(TAxis::kAxisRange)) {
      firstBinz = fZaxis.GetFirst();
      lastBinz  = fZaxis.GetLast();
      // For special case of TAxis::SetRange, when first == 1 and last
      // = N and the range bit has been set, the TAxis will return 0
      // for both.
      if(firstBinz == 0 && lastBinz == 0) {
         firstBinz = 1;
         lastBinz  = fZaxis.GetNbins();
      }
   }
   if(firstBinz < 0) {
      firstBinz = 0;
   }
   if(lastBinz < 0) {
      lastBinz = fZaxis.GetLast() + 1;
   }
   if(lastBinz > fZaxis.GetLast() + 1) {
      lastBinz = fZaxis.GetLast() + 1;
   }

   // Create the projection histogram
   char* pname = const_cast<char*>(name);
   if(name != nullptr && strcmp(name, expectedName) == 0) {
      Int_t nch = strlen(GetName()) + 4;
      pname     = new char[nch];
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
      TObjString* lb;
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
   Double_t cont, err2;
   Double_t totcont       = 0;
   Bool_t   computeErrors = h1->GetSumw2N() != 0;

   // implement filling of projected histogram
   // xbin is bin number of xAxis (the projected axis). Loop is done on all bin of TH2 histograms
   // inbin is the axis being integrated. Loop is done only on the selected bins
   for(Int_t xbin = 0; xbin <= fXaxis.GetNbins() + 1; ++xbin) {
      err2 = 0;
      cont = 0;
      if(fXaxis.TestBit(TAxis::kAxisRange) && (xbin < firstXBin || xbin > lastXBin)) {
         continue;
      }

      for(Int_t ybin = firstBiny; ybin <= lastBiny; ++ybin) {
         for(Int_t zbin = firstBinz; zbin <= lastBinz; ++zbin) {
            // sum bin content and error if needed
            cont += GetBinContent(xbin, ybin, zbin);
            if(computeErrors) {
               Double_t exy = GetBinError(xbin, ybin, zbin);
               err2 += exy * exy;
            }
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
   if(((!fgStatOverflows && firstBiny == 1 && lastBiny == fYaxis.GetLast()) ||
       (fgStatOverflows && firstBiny == 0 && lastBiny == fYaxis.GetLast() + 1)) &&
      ((!fgStatOverflows && firstBinz == 1 && lastBinz == fZaxis.GetLast()) ||
       (fgStatOverflows && firstBinz == 0 && lastBinz == fZaxis.GetLast() + 1))) {
      reuseStats = true;
   } else {
      // also if total content match we can re-use
      double eps = 1.E-12;
      if(IsA() == GCubeF::Class()) {
         eps = 1.E-6;
      }
      if(fTsumw != 0 && TMath::Abs(fTsumw - totcont) < TMath::Abs(fTsumw) * eps) {
         reuseStats = true;
      }
   }
   // retrieve  the statistics and set in projected histogram if we can re-use it
   bool reuseEntries = reuseStats;
   // can re-use entries if underflow/overflow are included
   reuseEntries &= static_cast<int>(firstBiny == 0 && lastBiny == fYaxis.GetLast() + 1 && firstBinz == 0 &&
                                    lastBinz == fYaxis.GetLast() + 1);
   if(reuseStats) {
      Double_t stats[kNstat];
      GetStats(stats);
      h1->PutStats(stats);
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
      Double_t entries = TMath::Floor(totcont + 0.5); // to avoid numerical rounding
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

void GCube::PutStats(Double_t* stats)
{
   // Replace current statistics with the values in array stats
   TH1::PutStats(stats);
   fTsumwy  = stats[4];
   fTsumwy2 = stats[5];
   fTsumwxy = stats[6];
   fTsumwz  = stats[7];
   fTsumwz2 = stats[8];
   fTsumwxz = stats[9];
   fTsumwyz = stats[10];
}

GCube* GCube::Rebin3D(Int_t ngroup, const char* newname)
{
   ///   -*-*-*Rebin this histogram grouping ngroup/ngroup bins along the xaxis/yaxis/zaxis together*-*-*-*-
   ///         =================================================================================
   ///   if newname is not blank a new temporary histogram hnew is created.
   ///   else the current histogram is modified (default)
   ///   The parameter ngroup indicates how many bins along the xaxis/yaxis/zaxis of this
   ///   have to me merged into one bin of hnew
   ///   If the original histogram has errors stored (via Sumw2), the resulting
   ///   histograms has new errors correctly calculated.
   ///
   ///   examples: if hpxpy is an existing GCube histogram with 40 x 40 x 40 bins
   ///     hpxpy->Rebin3D();  // merges two bins along the xaxis, yaxis, and zaxis in one in hpxpy
   ///                        // Carefull: previous contents of hpxpy are lost
   ///     hpxpy->Rebin3D(5); //merges five bins along the xaxis, yaxis, and zaxis in one in hpxpy
   ///     GCube* hnew = hpxpy->Rebin3D(5,"hnew"); // creates a new histogram hnew
   ///                        // merging 5 bins of h1 along the xaxis, yaxis, and zaxis in one bin
   ///
   ///   NOTE : If ngroup is not an exact divider of the number of bins,
   ///          along the xaxis/yaxis/zaxis the top limit(s) of the rebinned histogram
   ///          is changed to the upper edge of the bin=newbins*ngroup
   ///          and the corresponding bins are added to
   ///          the overflow bin.
   ///          Statistics will be recomputed from the new bin contents.

   Int_t    i, j, xbin, ybin, zbin;
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
   auto*    oldBins = new Double_t[(nbins + 2) * (nbins + 3) * (nbins + 4) / 6];
   for(xbin = 0; xbin < nbins + 2; xbin++) {
      for(ybin = 0; ybin <= xbin; ybin++) {
         for(zbin = 0; zbin <= ybin; zbin++) {
            Int_t bin    = GetBin(xbin, ybin, zbin);
            oldBins[bin] = GetBinContent(bin);
         }
      }
   }
   Double_t* oldErrors = nullptr;
   if(fSumw2.fN != 0) {
      oldErrors = new Double_t[(nbins + 2) * (nbins + 3) * (nbins + 4) / 6];
      for(xbin = 0; xbin < nbins + 2; xbin++) {
         for(ybin = 0; ybin <= xbin; ybin++) {
            for(zbin = 0; zbin <= ybin; zbin++) {
               Int_t bin      = GetBin(xbin, ybin, zbin);
               oldErrors[bin] = GetBinError(bin);
            }
         }
      }
   }

   // create a clone of the old histogram if newname is specified
   GCube* hnew = this;
   if(newname != nullptr && (strlen(newname) != 0u)) {
      hnew = static_cast<GCube*>(Clone());
      hnew->SetName(newname);
   }

   // save original statistics
   Double_t stat[kNstat];
   GetStats(stat);
   bool resetStat = false;

   // change axis specs and rebuild bin contents array
   if(newbins * ngroup != nbins) {
      max       = fXaxis.GetBinUpEdge(newbins * ngroup);
      resetStat = true; // stats must be reset because top bins will be moved to overflow bin
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
   // save the TAttAxis members (reset by SetBins) for z axis
   Int_t   nZdivisions  = fZaxis.GetNdivisions();
   Color_t zAxisColor   = fZaxis.GetAxisColor();
   Color_t zLabelColor  = fZaxis.GetLabelColor();
   Style_t zLabelFont   = fZaxis.GetLabelFont();
   Float_t zLabelOffset = fZaxis.GetLabelOffset();
   Float_t zLabelSize   = fZaxis.GetLabelSize();
   Float_t zTickLength  = fZaxis.GetTickLength();
   Float_t zTitleOffset = fZaxis.GetTitleOffset();
   Float_t zTitleSize   = fZaxis.GetTitleSize();
   Color_t zTitleColor  = fZaxis.GetTitleColor();
   Style_t zTitleFont   = fZaxis.GetTitleFont();

   // copy merged bin contents (ignore under/overflows)
   if(ngroup != 1) {
      if(fXaxis.GetXbins()->GetSize() > 0 || fYaxis.GetXbins()->GetSize() > 0 || fZaxis.GetXbins()->GetSize() > 0) {
         // variable bin sizes in x or y, don't treat both cases separately
         auto* bins = new Double_t[newbins + 1];
         for(i = 0; i <= newbins; ++i) {
            bins[i] = fXaxis.GetBinLowEdge(1 + i * ngroup);
         }
         hnew->SetBins(newbins, bins, newbins, bins); // changes also errors array (if any)
         delete[] bins;
      } else {
         hnew->SetBins(newbins, min, max, newbins, min, max); // changes also errors array
      }

      Double_t binContent, binError;
      Int_t    oldxbin = 1;
      Int_t    oldybin = 1;
      Int_t    bin;
      for(xbin = 1; xbin <= newbins; ++xbin) {
         oldybin = 1;
         for(ybin = 1; ybin <= xbin; ++ybin) {
            binContent = 0;
            binError   = 0;
            for(i = 0; i < ngroup; ++i) {
               if(oldxbin + i > nbins) {
                  break;
               }
               for(j = 0; j < ngroup; ++j) {
                  if(oldybin + j > nbins) {
                     break;
                  }
                  // get global bin (same conventions as in GCube::GetBin(xbin,ybin)
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
      binContent = 0;
      binError   = 0;
      for(xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
         for(ybin = oldybin; ybin <= xbin; ++ybin) {
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
      binContent = 0;
      binError   = 0;
      for(ybin = oldybin; ybin <= nbins + 1; ++ybin) {
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
      binContent = 0;
      binError   = 0;
      for(xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
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
      Double_t binContent0, binContent2;
      Double_t binError0, binError2;
      Int_t    oldxbin2, oldybin2;
      Int_t    ufbin, ofbin;
      oldxbin2 = 1;
      for(xbin = 1; xbin <= newbins; ++xbin) {
         binContent0 = binContent2 = 0;
         binError0 = binError2 = 0;
         for(i = 0; i < ngroup; ++i) {
            if(oldxbin2 + i > nbins) {
               break;
            }
            // old underflow bin (in y)
            ufbin = oldxbin2 + i;
            binContent0 += oldBins[ufbin];
            if(oldErrors != nullptr) {
               binError0 += oldErrors[ufbin] * oldErrors[ufbin];
            }
            for(ybin = oldybin; ybin <= nbins + 1; ++ybin) {
               // old overflow bin (in y)
               ofbin = ufbin + ybin * (nbins + 2);
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
      oldybin2 = 1;
      for(ybin = 1; ybin <= newbins; ++ybin) {
         binContent0 = binContent2 = 0;
         binError0 = binError2 = 0;
         for(i = 0; i < ngroup; ++i) {
            if(oldybin2 + i > nbins) {
               break;
            }
            // old underflow bin (in x)
            ufbin = (oldybin2 + i) * (nbins + 2);
            binContent0 += oldBins[ufbin];
            if(oldErrors != nullptr) {
               binError0 += oldErrors[ufbin] * oldErrors[ufbin];
            }
            for(xbin = oldxbin; xbin <= nbins + 1; ++xbin) {
               ofbin = ufbin + xbin;
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
   // Restore z axis attributes
   fZaxis.SetNdivisions(nZdivisions);
   fZaxis.SetAxisColor(zAxisColor);
   fZaxis.SetLabelColor(zLabelColor);
   fZaxis.SetLabelFont(zLabelFont);
   fZaxis.SetLabelOffset(zLabelOffset);
   fZaxis.SetLabelSize(zLabelSize);
   fZaxis.SetTickLength(zTickLength);
   fZaxis.SetTitleOffset(zTitleOffset);
   fZaxis.SetTitleSize(zTitleSize);
   fZaxis.SetTitleColor(zTitleColor);
   fZaxis.SetTitleFont(zTitleFont);

   // restore statistics and entries  modified by SetBinContent
   hnew->SetEntries(entries);
   if(!resetStat) {
      hnew->PutStats(stat);
   }

   delete[] oldBins;
   if(oldErrors != nullptr) {
      delete[] oldErrors;
   }
   return hnew;
}

void GCube::Reset(Option_t* option)
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

void GCube::SetShowProjection(const char* option, Int_t nbins)
{
   /// When the mouse is moved in a pad containing a 3-d view of this histogram
   /// a second canvas shows a projection type given as option.
   /// To stop the generation of the projections, delete the canvas
   /// containing the projection.
   /// option may contain a combination of the characters x,y,z,e
   /// option = "x" return the x projection into a TH1D histogram
   /// option = "y" return the y projection into a TH1D histogram
   /// option = "z" return the z projection into a TH1D histogram
   /// option = "xy" return the x versus y projection into a TH2D histogram
   /// option = "yx" return the y versus x projection into a TH2D histogram
   /// option = "xz" return the x versus z projection into a TH2D histogram
   /// option = "zx" return the z versus x projection into a TH2D histogram
   /// option = "yz" return the y versus z projection into a TH2D histogram
   /// option = "zy" return the z versus y projection into a TH2D histogram
   /// option can also include the drawing option for the projection, eg to draw
   /// the xy projection using the draw option "box" do
   ///   myhist.SetShowProjection("xy box");
   /// This function is typically called from the context menu.
   /// NB: the notation "a vs b" means "a" vertical and "b" horizontal

   GetPainter();
   if(fPainter != nullptr) {
      fPainter->SetShowProjection(option, nbins);
   }
}

TH1* GCube::ShowBackground(Int_t niter, Option_t* option)
{
   //   This function calculates the background spectrum in this histogram.
   //   The background is returned as a histogram.
   //   to be implemented (may be)

   return reinterpret_cast<TH1*>(gROOT->ProcessLineFast(
      Form(R"(TSpectrum2::StaticBackground((TH1*)0x%lx,%d,"%s"))", (ULong_t)this, niter, option)));
}

Int_t GCube::ShowPeaks(Double_t sigma, Option_t* option, Double_t threshold)
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

void GCube::Smooth(Int_t ntimes, Option_t* option)
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
   Int_t i, j, bin;
   for(i = ifirst; i <= ilast; ++i) {
      for(j = jfirst; j <= jlast; ++j) {
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
   for(i = ifirst; i <= ilast; ++i) {
      for(j = jfirst; j <= jlast; ++j) {
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
// GCubeF methods (float = four bytes per cell)
//------------------------------------------------------------

ClassImp(GCubeF)

   GCubeF::GCubeF()
   : GCube(), TArrayF()
{
   SetBinsLength(9);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeF::GCubeF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : GCube(name, title, nbins, low, up)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }

   if(low >= up) {
      SetBuffer(fgBufferSize);
   }
}

GCubeF::GCubeF(const char* name, const char* title, Int_t nbins, const Double_t* bins) : GCube(name, title, nbins, bins)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeF::GCubeF(const char* name, const char* title, Int_t nbins, const Float_t* bins) : GCube(name, title, nbins, bins)
{
   TArrayF::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeF::GCubeF(const GCubeF& rhs) : GCube(), TArrayF()
{
   rhs.Copy(*this);
}

GCubeF::~GCubeF() = default;

TH2F* GCubeF::GetMatrix(bool force)
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
         for(int k = 0; k < fXaxis.GetNbins() + 2; ++k) {
            fMatrix->SetBinContent(i, j, k, GetBinContent(i, j, k));
         }
      }
   }
   return static_cast<TH2F*>(fMatrix);
}

void GCubeF::Copy(TObject& rh) const
{
   GCube::Copy(static_cast<GCubeF&>(rh));
}

TH1* GCubeF::DrawCopy(Option_t* option, const char* name_postfix) const
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

Double_t GCubeF::GetBinContent(Int_t bin) const
{
   // Get bin content.

   if(fBuffer != nullptr) {
      const_cast<GCubeF*>(this)->BufferEmpty();
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
   return Double_t(fArray[bin]);
}

void GCubeF::Reset(Option_t* option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GCube::Reset(option);
   TArrayF::Reset();
}

void GCubeF::SetBinContent(Int_t bin, Double_t content)
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
   fArray[bin] = Float_t(content);
}

void GCubeF::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) {
      n = (fXaxis.GetNbins() + 2) * (fYaxis.GetNbins() + 2);
   }
   fNcells = n;
   TArrayF::Set(n);
}

GCubeF& GCubeF::operator=(const GCubeF& h1)
{
   // Operator =

   if(this != &h1) {
      const_cast<GCubeF&>(h1).Copy(*this);
   }
   return *this;
}

GCubeF operator*(Float_t c1, GCubeF& h1)
{
   // Operator *

   GCubeF hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeF operator+(GCubeF& h1, GCubeF& h2)
{
   // Operator +

   GCubeF hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeF operator-(GCubeF& h1, GCubeF& h2)
{
   // Operator -

   GCubeF hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeF operator*(GCubeF& h1, GCubeF& h2)
{
   // Operator *

   GCubeF hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeF operator/(GCubeF& h1, GCubeF& h2)
{
   // Operator /

   GCubeF hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

//------------------------------------------------------------
// GCubeD methods (double = eight bytes per cell)
//------------------------------------------------------------

ClassImp(GCubeD)

   GCubeD::GCubeD()
   : GCube(), TArrayD()
{
   SetBinsLength(9);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeD::GCubeD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
   : GCube(name, title, nbins, low, up)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }

   if(low >= up) {
      SetBuffer(fgBufferSize);
   }
}

GCubeD::GCubeD(const char* name, const char* title, Int_t nbins, const Double_t* bins) : GCube(name, title, nbins, bins)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeD::GCubeD(const char* name, const char* title, Int_t nbins, const Float_t* bins) : GCube(name, title, nbins, bins)
{
   TArrayD::Set(fNcells);
   if(fgDefaultSumw2) {
      Sumw2();
   }
}

GCubeD::GCubeD(const GCubeD& rhs) : GCube(), TArrayD()
{
   rhs.Copy(*this);
}

GCubeD::~GCubeD() = default;

TH2D* GCubeD::GetMatrix(bool force)
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
         for(int k = 0; k < fXaxis.GetNbins() + 2; ++k) {
            fMatrix->SetBinContent(i, j, k, GetBinContent(i, j, k));
         }
      }
   }
   return static_cast<TH2D*>(fMatrix);
}

void GCubeD::Copy(TObject& rh) const
{
   GCube::Copy(static_cast<GCubeD&>(rh));
}

TH1* GCubeD::DrawCopy(Option_t* option, const char* name_postfix) const
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

Double_t GCubeD::GetBinContent(Int_t bin) const
{
   // Get bin content.
   if(fBuffer != nullptr) {
      const_cast<GCubeD*>(this)->BufferEmpty();
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
   return Double_t(fArray[bin]);
}

void GCubeD::Reset(Option_t* option)
{
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GCube::Reset(option);
   TArrayD::Reset();
}

void GCubeD::SetBinContent(Int_t bin, Double_t content)
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
   fArray[bin] = Float_t(content);
}

void GCubeD::SetBinsLength(Int_t n)
{
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) {
      n = (fXaxis.GetNbins() + 2) * (fYaxis.GetNbins() + 2);
   }
   fNcells = n;
   TArrayD::Set(n);
}

GCubeD& GCubeD::operator=(const GCubeD& h1)
{
   // Operator =

   if(this != &h1) {
      const_cast<GCubeD&>(h1).Copy(*this);
   }
   return *this;
}

GCubeD operator*(Float_t c1, GCubeD& h1)
{
   // Operator *

   GCubeD hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeD operator+(GCubeD& h1, GCubeD& h2)
{
   // Operator +

   GCubeD hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeD operator-(GCubeD& h1, GCubeD& h2)
{
   // Operator -

   GCubeD hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeD operator*(GCubeD& h1, GCubeD& h2)
{
   // Operator *

   GCubeD hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

GCubeD operator/(GCubeD& h1, GCubeD& h2)
{
   // Operator /

   GCubeD hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(nullptr);
   return hnew;
}

//------------------------------------------------------------
