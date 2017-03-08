#include "GHSym.h"

#include "TROOT.h"
#include "THashList.h"
#include "THLimitsFinder.h"
#include "TVirtualHistPainter.h"
#include "TObjString.h"
#include "TVirtualPad.h"
#include "TMath.h"
#include "TPad.h"

#include <iostream>

ClassImp(GHSym)

GHSym::GHSym() {
	fDimension = 2;
}

GHSym::GHSym(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
	: TH1(name, title, nbins, low, up) {
	fDimension = 2;
	fTsumwy  = 0;
	fTsumwy2 = 0;
	fTsumwxy = 0;
	fYaxis.Set(nbins, low, up);
	// TH1 constructor sets fNcells to nbins+2
	// we need (nbins+2)*((nbins+2)+1)/2 cells 
	fNcells = (fNcells*(nbins+3))/2;
}

GHSym::GHSym(const char* name, const char* title, Int_t nbins, const Double_t* bins)
	: TH1(name, title, nbins, bins) {
	fDimension = 2;
	fTsumwy  = 0;
	fTsumwy2 = 0;
	fTsumwxy = 0;
	fYaxis.Set(nbins, bins);
	// TH1 constructor sets fNcells to nbins+2
	// we need (nbins+2)*((nbins+2)+1)/2 cells 
	fNcells = (fNcells*(nbins+3))/2;
}

GHSym::GHSym(const char* name, const char* title, Int_t nbins, const Float_t* bins)
	: TH1(name, title, nbins, bins) {
	fDimension = 2;
	fTsumwy  = 0;
	fTsumwy2 = 0;
	fTsumwxy = 0;
	fYaxis.Set(nbins, bins);
	// TH1 constructor sets fNcells to nbins+2
	// we need (nbins+2)*((nbins+2)+1)/2 cells 
	fNcells = (fNcells*(nbins+3))/2;
}

GHSym::~GHSym() {
}

Int_t GHSym::BufferEmpty(Int_t action) {
	/// Fill histogram with all entries in the buffer.
	/// action = -1 histogram is reset and refilled from the buffer (called by THistPainter::Paint)
	/// action =  0 histogram is filled from the buffer
	/// action =  1 histogram is filled and buffer is deleted
	///             The buffer is automatically deleted when the number of entries
	///             in the buffer is greater than the number of entries in the histogram
	if(fBuffer == nullptr) return 0;

	Int_t nbEntries = static_cast<Int_t>(fBuffer[0]);
	if(nbEntries == 0) return 0;
	if(nbEntries < 0 && action == 0) return 0; // histogram has been already filled from the buffer
	Double_t* buffer = fBuffer;
	if(nbEntries < 0) {
		nbEntries = -nbEntries;
		fBuffer = nullptr;
		Reset("ICES");
		fBuffer = buffer;
	}

   if(TestBit(kCanRebin) || fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin()) {
      // find min, max of entries in buffer
		// for the symmetric matrix x- and y-range are the same
      Double_t min = fBuffer[2];
      Double_t max = min;
		if(fBuffer[3] < min) min = fBuffer[3];
		if(fBuffer[3] > max) max = fBuffer[3];
      for(Int_t i = 1; i < nbEntries; ++i) {
         Double_t x = fBuffer[3*i+2];
         if(x < min) min = x;
         if(x > max) max = x;
         Double_t y = fBuffer[3*i+3];
         if(y < min) min = y;
         if(y > max) max = y;
      }
      if(fXaxis.GetXmax() <= fXaxis.GetXmin() || fYaxis.GetXmax() <= fYaxis.GetXmin()) {
         THLimitsFinder::GetLimitsFinder()->FindGoodLimits(this, min, max, min, max);
      } else {
         fBuffer = nullptr;
         Int_t keep = fBufferSize;
			fBufferSize = 0;
         if(min <  fXaxis.GetXmin()) RebinAxis(min,&fXaxis);
         if(max >= fXaxis.GetXmax()) RebinAxis(max,&fXaxis);
         if(min <  fYaxis.GetXmin()) RebinAxis(min,&fYaxis);
         if(max >= fYaxis.GetXmax()) RebinAxis(max,&fYaxis);
         fBuffer = buffer;
         fBufferSize = keep;
      }
   }

   fBuffer = 0;
   for(Int_t i = 0; i < nbEntries; ++i) {
      Fill(buffer[3*i+2], buffer[3*i+3], buffer[3*i+1]);
   }
   fBuffer = buffer;

   if(action > 0) {
		delete[] fBuffer;
		fBuffer = 0;
		fBufferSize = 0;
	} else {
      if(nbEntries == (Int_t)fEntries) fBuffer[0] = -nbEntries;
      else                             fBuffer[0] = 0;
   }
   return nbEntries;
}

Int_t GHSym::BufferFill(Double_t x, Double_t y, Double_t w) {
	if(fBuffer == nullptr) return -3;

	Int_t nbEntries = static_cast<Int_t>(fBuffer[0]);
	if(nbEntries < 0) {
		nbEntries = -nbEntries;
		fBuffer[0] = nbEntries;
		if(fEntries > 0) {
			Double_t* buffer = fBuffer;
			fBuffer = nullptr;
			Reset("ICES");
			fBuffer = buffer;
		}
	}
	if(3*nbEntries+3 >= fBufferSize) {
		BufferEmpty(1);
		return Fill(x, y, w);
	}
	fBuffer[3*nbEntries+1] = w;
	fBuffer[3*nbEntries+2] = x;
	fBuffer[3*nbEntries+3] = y;
	fBuffer[0] += 1;

	return -3;
}

void GHSym::Copy(TObject &obj) const {
   // Copy.

   TH1::Copy(obj);
   static_cast<GHSym&>(obj).fTsumwy      = fTsumwy;
   static_cast<GHSym&>(obj).fTsumwy2     = fTsumwy2;
   static_cast<GHSym&>(obj).fTsumwxy     = fTsumwxy;
}

Int_t GHSym::Fill(Double_t x, Double_t y) {
	/// Increment cell defined by x,y by 1.
	if(fBuffer) return BufferFill(x,y,1);

	Int_t binx, biny, bin;
	fEntries++;
	if(y <= x) {
		binx = fXaxis.FindBin(x);
		biny = fYaxis.FindBin(y);
	} else {
		binx = fXaxis.FindBin(y);
		biny = fYaxis.FindBin(x);
	}
	if(binx < 0 || biny < 0) return -1;
	bin  = biny*(2*fXaxis.GetNbins()-biny+3)/2 + binx;
	AddBinContent(bin);
	if(fSumw2.fN) ++fSumw2.fArray[bin];
	if(binx == 0 || binx > fXaxis.GetNbins()) {
		if(!fgStatOverflows) return -1;
	}
	if(biny == 0 || biny > fYaxis.GetNbins()) {
		if(!fgStatOverflows) return -1;
	}
	// not sure if these summed weights are calculated correct
	// as of now this is the method used in TH2
	++fTsumw;
	++fTsumw2;
	fTsumwx  += x;
	fTsumwx2 += x*x;
	fTsumwy  += y;
	fTsumwy2 += y*y;
	fTsumwxy += x*y;
	return bin;
}

Int_t GHSym::Fill(Double_t x, Double_t y, Double_t w) {
	/// Increment cell defined by x,y by 1.
	if(fBuffer) return BufferFill(x,y,1);

	Int_t binx, biny, bin;
	fEntries++;
	if(y <= x) {
		binx = fXaxis.FindBin(x);
		biny = fYaxis.FindBin(y);
	} else {
		binx = fXaxis.FindBin(y);
		biny = fYaxis.FindBin(x);
	}
	if(binx < 0 || biny < 0) return -1;
	bin  = biny*(2*fXaxis.GetNbins()-biny+3)/2 + binx;
	AddBinContent(bin, w);
	if(fSumw2.fN) fSumw2.fArray[bin] += w*w;
	if(binx == 0 || binx > fXaxis.GetNbins()) {
		if(!fgStatOverflows) return -1;
	}
	if(biny == 0 || biny > fYaxis.GetNbins()) {
		if(!fgStatOverflows) return -1;
	}
	// not sure if these summed weights are calculated correct
	// as of now this is the method used in TH2
	fTsumw   += w;
	fTsumw2  += w*w;
	fTsumwx  += w*x;
	fTsumwx2 += w*x*x;
	fTsumwy  += w*y;
	fTsumwy2 += w*y*y;
	fTsumwxy += w*x*y;
	return bin;
}

TH1D* GHSym::Projection(const char* name, Int_t firstBin, Int_t lastBin, Option_t* option) const {
   /// method for performing projection

   const char* expectedName = "_pr";

   TString opt = option;
   TString cut;
   Int_t i1 = opt.Index("[");
   if(i1>=0) {
      Int_t i2 = opt.Index("]");
      cut = opt(i1,i2-i1+1);
   }
   opt.ToLower();  //must be called after having parsed the cut name
   bool originalRange = opt.Contains("o");

   Int_t firstXBin = fXaxis.GetFirst();
	Int_t lastXBin = fXaxis.GetLast();

   if(firstXBin == 0 && lastXBin == 0) {
      firstXBin = 1; 
		lastXBin = fXaxis.GetNbins();
   }

   if(lastBin < firstBin && fYaxis.TestBit(TAxis::kAxisRange)) {
      firstBin = fYaxis.GetFirst();
      lastBin = fYaxis.GetLast();
      // For special case of TAxis::SetRange, when first == 1 and last
      // = N and the range bit has been set, the TAxis will return 0
      // for both.
      if(firstBin == 0 && lastBin == 0) {
         firstBin = 1;
         lastBin = fYaxis.GetNbins();
      }
   }
   if(firstBin < 0) firstBin = 0;
   if(lastBin  < 0) lastBin  = fYaxis.GetLast() + 1;
   if(lastBin  > fYaxis.GetLast()+1) lastBin  = fYaxis.GetLast() + 1;

   // Create the projection histogram
   char* pname = const_cast<char*>(name);
   if(name != nullptr && strcmp(name, expectedName) == 0) {
      Int_t nch = strlen(GetName()) + 4;
      pname = new char[nch];
      snprintf(pname,nch,"%s%s",GetName(),name);
   }
   TH1D* h1 = nullptr;
   //check if histogram with identical name exist
   // if compatible reset and re-use previous histogram
   // (see https://savannah.cern.ch/bugs/?54340)
   TObject* h1obj = gROOT->FindObject(pname);
   if(h1obj && h1obj->InheritsFrom(TH1::Class())) {
      if(h1obj->IsA() != TH1D::Class() ) {
         Error("DoProjection","Histogram with name %s must be a TH1D and is a %s",name,h1obj->ClassName());
         return 0;
      }
      h1 = static_cast<TH1D*>(h1obj);
      // reset the existing histogram and set always the new binning for the axis
      // This avoid problems when the histogram already exists and the histograms is rebinned or its range has changed
      // (see https://savannah.cern.ch/bugs/?94101 or https://savannah.cern.ch/bugs/?95808 )
      h1->Reset();
      const TArrayD* xbins = fXaxis.GetXbins();
      if(xbins->fN == 0) {
         if(originalRange)
            h1->SetBins(fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
         else
            h1->SetBins(lastXBin-firstXBin+1, fXaxis.GetBinLowEdge(firstXBin), fXaxis.GetBinUpEdge(lastXBin));
      } else {
         // case variable bins
         if(originalRange)
            h1->SetBins(fXaxis.GetNbins(), xbins->fArray);
         else
            h1->SetBins(lastXBin-firstXBin+1, &(xbins->fArray[firstXBin-1]));
      }
   }
   Int_t ncuts = 0;
   if(opt.Contains("[")) {
      const_cast<GHSym*>(this)->GetPainter();
      if(fPainter) ncuts = fPainter->MakeCuts((char*)cut.Data());
   }

   if(h1 == nullptr) {
      const TArrayD* bins = fXaxis.GetXbins();
      if(bins->fN == 0) {
         if(originalRange)
            h1 = new TH1D(pname, GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
         else
            h1 = new TH1D(pname, GetTitle(), lastXBin-firstXBin+1,
                          fXaxis.GetBinLowEdge(firstXBin), fXaxis.GetBinUpEdge(lastXBin));
      } else {
         // case variable bins
         if(originalRange)
            h1 = new TH1D(pname, GetTitle(), fXaxis.GetNbins(), bins->fArray);
         else
            h1 = new TH1D(pname, GetTitle(), lastXBin-firstXBin+1, &(bins->fArray[firstXBin-1]));
      }
      if(opt.Contains("e") || GetSumw2N() ) h1->Sumw2();
   }
   if(pname != name)  delete[] pname;

   // Copy the axis attributes and the axis labels if needed.
   h1->GetXaxis()->ImportAttributes(&fXaxis);
   THashList* labels = const_cast<TAxis*>(&fXaxis)->GetLabels();
   if(labels != nullptr) {
      TIter iL(labels);
      TObjString* lb;
      Int_t i = 1;
      while ((lb=(TObjString*)iL())) {
         h1->GetXaxis()->SetBinLabel(i,lb->String().Data());
         i++;
      }
   }

   h1->SetLineColor(GetLineColor());
   h1->SetFillColor(GetFillColor());
   h1->SetMarkerColor(GetMarkerColor());
   h1->SetMarkerStyle(GetMarkerStyle());

   // Fill the projected histogram
   Double_t cont,err2;
   Double_t totcont = 0;
   Bool_t  computeErrors = h1->GetSumw2N();

   // implement filling of projected histogram
   // xbin is bin number of xAxis (the projected axis). Loop is done on all bin of TH2 histograms
   // inbin is the axis being integrated. Loop is done only on the selected bins
   for(Int_t xbin = 0; xbin <= fXaxis.GetNbins() + 1; ++xbin) {
      err2 = 0;
      cont = 0;
      if(fXaxis.TestBit(TAxis::kAxisRange) && (xbin < firstXBin || xbin > lastXBin)) continue;

      for(Int_t ybin = firstBin ; ybin <= lastBin ; ++ybin) {
         if(ncuts) {
            if(!fPainter->IsInside(xbin, ybin)) continue;
         }
         // sum bin content and error if needed
         cont += GetCellContent(xbin, ybin);
         if(computeErrors) {
            Double_t exy = GetCellError(xbin, ybin);
            err2  += exy*exy;
         }
      }
      // find corresponding bin number in h1 for xbin
      Int_t binOut = h1->GetXaxis()->FindBin(fXaxis.GetBinCenter(xbin));
      h1->SetBinContent(binOut, cont);
      if(computeErrors) h1->SetBinError(binOut,TMath::Sqrt(err2));
      // sum  all content
      totcont += cont;
   }

   // check if we can re-use the original statistics from  the previous histogram
   bool reuseStats = false;
   if((fgStatOverflows == false && firstBin == 1 && lastBin == fYaxis.GetLast()     ) ||
      (fgStatOverflows == true  && firstBin == 0 && lastBin == fYaxis.GetLast() + 1 ) )
      reuseStats = true;
   else {
      // also if total content match we can re-use
      double eps = 1.E-12;
      if(IsA() == GHSymF::Class()) eps = 1.E-6;
      if(fTsumw != 0 && TMath::Abs(fTsumw - totcont) <  TMath::Abs(fTsumw) * eps)
         reuseStats = true;
   }
   if(ncuts) reuseStats = false;
   // retrieve  the statistics and set in projected histogram if we can re-use it
   bool reuseEntries = reuseStats;
   // can re-use entries if underflow/overflow are included
   reuseEntries &= (firstBin==0 && lastBin == fYaxis.GetLast()+1);
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
      Double_t entries =  TMath::Floor( totcont + 0.5); // to avoid numerical rounding
      if(h1->GetSumw2N()) entries = h1->GetEffectiveEntries();
      h1->SetEntries( entries );
   }

   if(opt.Contains("d")) {
      TVirtualPad* padsav = gPad;
      TVirtualPad* pad = gROOT->GetSelectedPad();
      if(pad) pad->cd();
      opt.Remove(opt.First("d"),1);
      // remove also other options
      if(opt.Contains("e")) opt.Remove(opt.First("e"),1);
      if(!gPad || !gPad->FindObject(h1)) {
         h1->Draw(opt);
      } else {
         h1->Paint(opt);
      }
      if(padsav) padsav->cd();
   }

   return h1;
}

Int_t GHSym::GetBin(Int_t binx, Int_t biny) const {
	Int_t n = fXaxis.GetNbins()+2;
	if(binx < 0)  binx = 0;
	if(binx >= n) binx = n-1;
	if(biny < 0)  biny = 0;
	if(biny >= n) biny = n-1;
	if(biny <= binx) {
		return binx + biny*(2*fXaxis.GetNbins()-biny+3)/2;
	}
	return biny + binx*(2*fXaxis.GetNbins()-binx+3)/2;
}

Double_t GHSym::GetCellContent(Int_t binx, Int_t biny) const {
	return GetBinContent(GetBin(binx, biny));
}

//------------------------------------------------------------
// GHSymF methods (float = four bytes per cell)
//------------------------------------------------------------

ClassImp(GHSymF)

GHSymF::GHSymF() : GHSym(), TArrayF() {
	SetBinsLength(9);
	if(fgDefaultSumw2) Sumw2();
}

GHSymF::~GHSymF() {
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
	: GHSym(name, title, nbins, low, up) {
	TArrayF::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();

	if(low >= up) SetBuffer(fgBufferSize);
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, const Double_t* bins)
	: GHSym(name, title, nbins, bins) {
	TArrayF::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();
}

GHSymF::GHSymF(const char* name, const char* title, Int_t nbins, const Float_t* bins)
	: GHSym(name, title, nbins, bins) {
	TArrayF::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();
}

TH2F* GHSymF::GetMatrix() {
	TH2F* mat = nullptr;

	//try and reuse existing histogram
   TObject* h1obj = gROOT->FindObject(Form("%s_mat", GetName()));
   if(h1obj && h1obj->InheritsFrom(TH1::Class())) {
      if(h1obj->IsA() != TH2F::Class() ) {
         Error("DoProjection","Histogram with name %s_mat must be a TH2F and is a %s", GetName(), h1obj->ClassName());
         return 0;
      }
      mat = static_cast<TH2F*>(h1obj);
      // reset the existing histogram and set always the new binning for the axis
      // This avoid problems when the histogram already exists and the histograms is rebinned or its range has changed
      // (see https://savannah.cern.ch/bugs/?94101 or https://savannah.cern.ch/bugs/?95808 )
      mat->Reset();
		mat->GetXaxis()->Set(fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
		mat->GetYaxis()->Set(fYaxis.GetNbins(), fYaxis.GetXmin(), fYaxis.GetXmax());
   }
	if(mat == nullptr) {
		mat = new TH2F(Form("%s_mat", GetName()), GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax(), fYaxis.GetNbins(), fYaxis.GetXmin(), fYaxis.GetXmax());
	}
	//copy cell contents (including all overflow and underflow cells)
	for(int i = 0; i < fXaxis.GetNbins()+2; ++i) {
		for(int j = 0; j < fXaxis.GetNbins()+2; ++j) {
			mat->SetBinContent(i,j,GetBinContent(i,j));
		}
	}
	return mat;
}

void GHSymF::Copy(TObject& rh) const {
	GHSym::Copy(static_cast<GHSymF&>(rh));
}

TH1* GHSymF::DrawCopy(Option_t *option) const {
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if(gPad != nullptr && !opt.Contains("same")) gPad->Clear();
   GHSymF* newth2 = static_cast<GHSymF*>(Clone());
   newth2->SetDirectory(nullptr);
   newth2->SetBit(kCanDelete);
   newth2->AppendPad(option);
   return newth2;
}

Double_t GHSymF::GetBinContent(Int_t bin) const {
   // Get bin content.
   
   if(fBuffer) const_cast<GHSymF*>(this)->BufferEmpty();
   if(bin < 0) bin = 0;
   if(bin >= fNcells) bin = fNcells-1;
   if(!fArray) return 0;
   return Double_t(fArray[bin]);
}  
   
void GHSymF::Reset(Option_t *option) {
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GHSym::Reset(option);
   TArrayF::Reset();
}
   
void GHSymF::SetBinContent(Int_t bin, Double_t content) {
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if(bin < 0) return;
   if(bin >= fNcells) return;
   fArray[bin] = Float_t(content);
}

void GHSymF::SetBinsLength(Int_t n) {
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2);
   fNcells = n;
   TArrayF::Set(n);
}

GHSymF& GHSymF::operator=(const GHSymF& h1) {
   // Operator =

   if(this != &h1)  const_cast<GHSymF&>(h1).Copy(*this);
   return *this;
}

GHSymF operator*(Float_t c1, GHSymF& h1) {
   // Operator *

   GHSymF hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymF operator+(GHSymF& h1, GHSymF& h2) {
   // Operator +

   GHSymF hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymF operator-(GHSymF& h1, GHSymF& h2) {
   // Operator -

   GHSymF hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymF operator*(GHSymF& h1, GHSymF& h2) {
   // Operator *

   GHSymF hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymF operator/(GHSymF& h1, GHSymF& h2) {
   // Operator /

   GHSymF hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//------------------------------------------------------------
// GHSymD methods (double = eight bytes per cell)
//------------------------------------------------------------

ClassImp(GHSymD)

GHSymD::GHSymD() : GHSym(), TArrayD() {
	SetBinsLength(9);
	if(fgDefaultSumw2) Sumw2();
}

GHSymD::~GHSymD() {
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, Double_t low, Double_t up)
	: GHSym(name, title, nbins, low, up) {
	TArrayD::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();

	if(low >= up) SetBuffer(fgBufferSize);
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, const Double_t* bins)
	: GHSym(name, title, nbins, bins) {
	TArrayD::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();
}

GHSymD::GHSymD(const char* name, const char* title, Int_t nbins, const Float_t* bins)
	: GHSym(name, title, nbins, bins) {
	TArrayD::Set(fNcells);
	if(fgDefaultSumw2) Sumw2();
}

TH2D* GHSymD::GetMatrix() {
	TH2D* mat = nullptr;

	//try and reuse existing histogram
   TObject* h1obj = gROOT->FindObject(Form("%s_mat", GetName()));
   if(h1obj && h1obj->InheritsFrom(TH1::Class())) {
      if(h1obj->IsA() != TH2D::Class() ) {
         Error("DoProjection","Histogram with name %s_mat must be a TH2D and is a %s", GetName(), h1obj->ClassName());
         return 0;
      }
      mat = static_cast<TH2D*>(h1obj);
      // reset the existing histogram and set always the new binning for the axis
      // This avoid problems when the histogram already exists and the histograms is rebinned or its range has changed
      // (see https://savannah.cern.ch/bugs/?94101 or https://savannah.cern.ch/bugs/?95808 )
      mat->Reset();
		mat->GetXaxis()->Set(fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax());
		mat->GetYaxis()->Set(fYaxis.GetNbins(), fYaxis.GetXmin(), fYaxis.GetXmax());
   }
	if(mat == nullptr) {
		mat = new TH2D(Form("%s_mat", GetName()), GetTitle(), fXaxis.GetNbins(), fXaxis.GetXmin(), fXaxis.GetXmax(), fYaxis.GetNbins(), fYaxis.GetXmin(), fYaxis.GetXmax());
	}
	//copy cell contents (including all overflow and underflow cells)
	for(int i = 0; i < fXaxis.GetNbins()+2; ++i) {
		for(int j = 0; j < fXaxis.GetNbins()+2; ++j) {
			mat->SetBinContent(i,j,GetBinContent(i,j));
		}
	}
	return mat;
}

void GHSymD::Copy(TObject& rh) const {
	GHSym::Copy(static_cast<GHSymD&>(rh));
}

TH1* GHSymD::DrawCopy(Option_t *option) const {
   // Draw copy.

   TString opt = option;
   opt.ToLower();
   if(gPad != nullptr && !opt.Contains("same")) gPad->Clear();
   GHSymD* newth2 = static_cast<GHSymD*>(Clone());
   newth2->SetDirectory(nullptr);
   newth2->SetBit(kCanDelete);
   newth2->AppendPad(option);
   return newth2;
}

Double_t GHSymD::GetBinContent(Int_t bin) const {
   // Get bin content.
   if(fBuffer) const_cast<GHSymD*>(this)->BufferEmpty();
   if(bin < 0) bin = 0;
   if(bin >= fNcells) bin = fNcells-1;
   if(!fArray) return 0;
   return Double_t(fArray[bin]);
}  
   
void GHSymD::Reset(Option_t *option) {
   //*-*-*-*-*-*-*-*Reset this histogram: contents, errors, etc*-*-*-*-*-*-*-*
   //*-*            ===========================================

   GHSym::Reset(option);
   TArrayD::Reset();
}
   
void GHSymD::SetBinContent(Int_t bin, Double_t content) {
   // Set bin content
   fEntries++;
   fTsumw = 0;
   if(bin < 0) return;
   if(bin >= fNcells) return;
   fArray[bin] = Float_t(content);
}

void GHSymD::SetBinsLength(Int_t n) {
   // Set total number of bins including under/overflow
   // Reallocate bin contents array

   if(n < 0) n = (fXaxis.GetNbins()+2)*(fYaxis.GetNbins()+2);
   fNcells = n;
   TArrayD::Set(n);
}

GHSymD& GHSymD::operator=(const GHSymD& h1) {
   // Operator =

   if(this != &h1)  const_cast<GHSymD&>(h1).Copy(*this);
   return *this;
}

GHSymD operator*(Float_t c1, GHSymD& h1) {
   // Operator *

   GHSymD hnew = h1;
   hnew.Scale(c1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymD operator+(GHSymD& h1, GHSymD& h2) {
   // Operator +

   GHSymD hnew = h1;
   hnew.Add(&h2, 1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymD operator-(GHSymD& h1, GHSymD& h2) {
   // Operator -

   GHSymD hnew = h1;
   hnew.Add(&h2, -1);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymD operator*(GHSymD& h1, GHSymD& h2) {
   // Operator *

   GHSymD hnew = h1;
   hnew.Multiply(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

GHSymD operator/(GHSymD& h1, GHSymD& h2) {
   // Operator /

   GHSymD hnew = h1;
   hnew.Divide(&h2);
   hnew.SetDirectory(0);
   return hnew;
}

//------------------------------------------------------------
