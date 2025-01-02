#include "GH1D.h"

#include <iostream>
#include <fstream>
#include <cstring>

#include "TVirtualPad.h"
#include "TString.h"
#include "TF1.h"
#include "TFrame.h"
//#include "TROOT.h"
//#include "TSystem.h"
#include "KeySymbols.h"
#include "TVirtualX.h"

#include "Globals.h"
#include "GRootCommands.h"
#include "GCanvas.h"
#include "GH2I.h"
#include "GH2D.h"

int GH1D::fVerboseLevel = 0;

GH1D::GH1D(const TH1& source) : fParent(nullptr), fProjectionAxis(-1)
{
   source.Copy(*this);
}

GH1D::GH1D(const TH1* source) : fParent(nullptr), fProjectionAxis(-1)
{
   source->Copy(*this);
}

GH1D::GH1D(const TF1& function, Int_t nbinsx, Double_t xlow, Double_t xup)
   : TH1D(Form("%s_hist", function.GetName()), Form("%s_hist", function.GetName()), nbinsx, xlow, xup), fParent(nullptr),
     fProjectionAxis(-1)
{
   for(int i = 0; i < nbinsx; i++) {
      Fill(GetBinCenter(i), function.Eval(i));
   }
}

bool GH1D::WriteDatFile(const char* outFile)
{
   if(strlen(outFile) < 1) {
      return false;
   }

   std::ofstream out;
   out.open(outFile);

   if(!(out.is_open())) {
      return false;
   }

   for(int i = 0; i < GetNbinsX(); i++) {
      out << GetXaxis()->GetBinCenter(i) << "\t" << GetBinContent(i) << std::endl;
   }
   out << std::endl;
   out.close();

   return true;
}

/*
GH1D::GH1D(const TH1 *source)
  : fParent(nullptr), fProjectionAxis(-1) {
  if(source->GetDiminsion()>1) {
    return;
  }

  // Can copy from any 1-d TH1, not just a TH1D
  source->Copy(*this);

  // Force a refresh of any parameters stored in the option string.
  SetOption(GetOption());
}

void GH1D::SetOption(Option_t* opt) {
  fOption = opt;

  TString sopt = opt;
  if(sopt.Index("axis:")) {
    fProjectionAxis = 0;// TODO
  }
}
*/

void GH1D::Clear(Option_t* opt)
{
   TH1D::Clear(opt);
   fParent = nullptr;
}

void GH1D::Print(Option_t* opt) const
{
   TH1D::Print(opt);
   std::cout << "\tParent: " << fParent.GetObject() << std::endl;
}

void GH1D::Copy(TObject& obj) const
{
   TH1D::Copy(obj);

   static_cast<GH1D&>(obj).fParent = fParent;
}

void GH1D::Draw(Option_t* opt)
{
   TString option(opt);
   if(option.Contains("new", TString::kIgnoreCase)) {
      option.ReplaceAll("new", "");
      new GCanvas;
   }
   TH1D::Draw(option.Data());
	DrawRegions(opt);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
		SetPad(gPad);
   }
}

TH1* GH1D::DrawCopy(Option_t* opt, const char* name_postfix) const
{
   TH1* hist = TH1D::DrawCopy(opt, name_postfix);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

TH1* GH1D::DrawNormalized(Option_t* opt, Double_t norm) const
{
   TH1* hist = TH1D::DrawNormalized(opt, norm);
   if(gPad != nullptr) {
      gPad->Update();
      gPad->GetFrame()->SetBit(TBox::kCannotMove);
   }
   return hist;
}

GH1D* GH1D::GetPrevious(bool DrawEmpty) const
{
   if((fParent.GetObject() != nullptr) && fParent.GetObject()->InheritsFrom(GH2Base::Class())) {
      GH2D* gpar  = static_cast<GH2D*>(fParent.GetObject());
      int   first = GetXaxis()->GetFirst();
      int   last  = GetXaxis()->GetLast();
      GH1D* prev  = gpar->GetPrevious(this, DrawEmpty);
      prev->GetXaxis()->SetRange(first, last);
      return prev;   // gpar->GetPrevious(this,DrawEmpty);
   }
   return nullptr;
}

GH1D* GH1D::GetNext(bool DrawEmpty) const
{
   if((fParent.GetObject() != nullptr) && fParent.GetObject()->InheritsFrom(GH2Base::Class())) {
      GH2D* gpar  = static_cast<GH2D*>(fParent.GetObject());
      int   first = GetXaxis()->GetFirst();
      int   last  = GetXaxis()->GetLast();
      GH1D* next  = gpar->GetNext(this, DrawEmpty);
      next->GetXaxis()->SetRange(first, last);
      return next;   // gpar->GetNext(this,DrawEmpty);
   }
   return nullptr;
}

GH1D* GH1D::Project(double value_low, double value_high) const
{

   if((fParent.GetObject() != nullptr) && fParent.GetObject()->InheritsFrom(GH2Base::Class()) && fProjectionAxis != -1) {
      if(value_low > value_high) {
         std::swap(value_low, value_high);
      }
      GH2D* gpar = static_cast<GH2D*>(fParent.GetObject());
      if(fProjectionAxis == 0) {
         int bin_low  = gpar->GetXaxis()->FindBin(value_low);
         int bin_high = gpar->GetXaxis()->FindBin(value_high);
         return gpar->ProjectionY("_py", bin_low, bin_high);
      }
      int bin_low  = gpar->GetYaxis()->FindBin(value_low);
      int bin_high = gpar->GetYaxis()->FindBin(value_high);
      return gpar->ProjectionX("_px", bin_low, bin_high);
   }
   return nullptr;
}

GH1D* GH1D::Project_Background(double value_low, double value_high, double bg_value_low, double bg_value_high,
                               EBackgroundSubtraction mode) const
{
   if((fParent.GetObject() != nullptr) && fParent.GetObject()->InheritsFrom(GH2Base::Class()) && fProjectionAxis != -1) {
      if(value_low > value_high) {
         std::swap(value_low, value_high);
      }
      if(bg_value_low > bg_value_high) {
         std::swap(bg_value_low, bg_value_high);
      }

      GH2D* gpar = static_cast<GH2D*>(fParent.GetObject());
      if(fProjectionAxis == 0) {
         int bin_low     = gpar->GetXaxis()->FindBin(value_low);
         int bin_high    = gpar->GetXaxis()->FindBin(value_high);
         int bg_bin_low  = gpar->GetXaxis()->FindBin(bg_value_low);
         int bg_bin_high = gpar->GetXaxis()->FindBin(bg_value_high);

         return gpar->ProjectionY_Background(bin_low, bin_high, bg_bin_low, bg_bin_high, mode);
      }
      int bin_low     = gpar->GetYaxis()->FindBin(value_low);
      int bin_high    = gpar->GetYaxis()->FindBin(value_high);
      int bg_bin_low  = gpar->GetYaxis()->FindBin(bg_value_low);
      int bg_bin_high = gpar->GetYaxis()->FindBin(bg_value_high);

      return gpar->ProjectionX_Background(bin_low, bin_high, bg_bin_low, bg_bin_high, mode);
   }
   return nullptr;
}

GH1D* GH1D::Project(int bins)
{
   GH1D*  proj = nullptr;
   double ymax = GetMinimum();
   double ymin = GetMaximum();
   if(bins == -1) {
      bins = static_cast<int>(std::abs(ymax - ymin));
      if(bins < 1) {
         bins = 100;
      }
   }
   proj = new GH1D(Form("%s_y_axis_projection", GetName()), Form("%s_y_axis_projection", GetName()), bins, ymin, ymax);
   for(int i = 0; i < GetNbinsX(); i++) {
      if(GetBinContent(i) != 0) {
         proj->Fill(GetBinContent(i));
      }
   }

   return proj;
}

void GH1D::SetPad(TVirtualPad* pad)
{
    fPad = pad;
    fPad->GetCanvas()->Connect("ProcessedEvent(Int_t, Int_t, Int_t, TObject*)", "GH1D", this, "HandleMovement(Int_t,Int_t,Int_t, TObject*)");
	 gClient->Connect("ProcessedEvent(Event_t*, Window_t)", "GH1D", this, "HandleEvent(Event_t*, Window_t)");
}

void GH1D::HandleMovement(Int_t eventType, Int_t eventX, Int_t eventY, TObject* selected)
{
	double currentX = fPad->PixeltoX(eventX);
	double currentY = fPad->PixeltoY(eventY - fPad->GetWh());
	
	if(selected != nullptr && selected->InheritsFrom(TRegion::Class())) {
		auto* region = static_cast<TRegion*>(selected);
		if(eventType == kKeyPress && eventX == kKey_d) {
			RemoveRegion(region);
		} else if(eventType == kButton1Down) {
			fStartX = currentX;
			fStartY = currentY;
			if(VerboseLevel() > 1) {
				std::cout << "button 1 down at " << currentX << ", " << currentY << std::endl;
			}
		} else if(eventType == kButton1Motion) {
			if(VerboseLevel() > 2) {
				std::cout << "button 1 motion at " << currentX << ", " << currentY << std::endl;
			}
		} else if(eventType == kButton1Up) {
			region->Update(fStartX, currentX);
			if(VerboseLevel() > 1) {
				std::cout << "button 1 up at " << currentX << ", " << currentY << std::endl;
			}
		}
	} else if(VerboseLevel() > 3) {
		std::cout << "nullptr selected at " << currentX << ", " << currentY << std::endl;
	}
}

void GH1D::HandleEvent(Event_t* event, Window_t window)
{
	//auto* tgWindow = gClient->GetWindowById(window);

	if(gPad != fPad && VerboseLevel() < 5) { 
		return;
	}

	UInt_t              keySymbol = 0;
	std::array<char, 2> str;
	gVirtualX->LookupString(event, str.data(), str.size(), keySymbol);

	if((VerboseLevel() > 1 && event->fType != kMotionNotify && event->fType != kLeaveNotify) ||
		(VerboseLevel() > 2 && event->fState != 0) ||
		(VerboseLevel() > 3)) {
		std::cout << __PRETTY_FUNCTION__ << ", event " << event << ", window " << window << ", type " << event->fType << ", code " << event->fCode << ", state " << event->fState << ", x " << event->fX << ", root-x " << event->fXRoot << ", y " << event->fY << ", root-y " << event->fYRoot << ", x/y coordinates: x " << fPad->PixeltoX(event->fX) << ", y " << fPad->PixeltoY(event->fY - fPad->GetWh()) << ", root-x " << fPad->PixeltoX(event->fXRoot) << ", root-y " << fPad->PixeltoY(event->fYRoot - fPad->GetWh()) << ", key symbol " << keySymbol << " = " << hex(keySymbol) << "; fPad " << fPad << " = \"" << fPad->GetName() << "\", gPad " << gPad << " = \"" << gPad->GetName() << "\"" << std::endl;
	}

	switch(event->fType) {
		case kGKeyPress: // if a key is held, this one gets repeated
			if(str[0] == kESC) {   // ESC sets the escape flag
				gROOT->SetEscape();
				gPad->Modified();
			}
			if(str[0] == 3) {   // ctrl-c sets the interrupt flag
				gROOT->SetInterrupt();
			}

			switch(keySymbol) {
				case kKey_b:
					fBackground = true;
					break;
				case kKey_g:
					fGate = true;
					break;
				case kKey_r:
					fRegion = true;
					break;
				case kKey_l:
					// toggle logscale for y
					if(fPad->GetLogy() == 0) {
						fPad->SetLogy(1);
					} else {
						fPad->SetLogy(0);
					}
					// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been re-drawn ...
					UpdatePad();
					DrawRegions();
					UpdatePad();
					break;
				case kKey_p:
					//f2DPlayer->Project();
					break;
				case kKey_u:
					GetXaxis()->UnZoom();
					GetYaxis()->UnZoom();
					// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been re-drawn ...
					UpdatePad();
					DrawRegions();
					UpdatePad();
					if(VerboseLevel() > 1) {
						PrintRegions();
					}
					break;
				case kKey_U:
					GetYaxis()->UnZoom();
					// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been updated ...
					UpdatePad();
					DrawRegions();
					UpdatePad();
					if(VerboseLevel() > 1) {
						PrintRegions();
					}
					break;
				case kKey_Escape:
					if(fGate || fBackground || fRegion) {
						RemoveCurrentRegion();
					}
					if(VerboseLevel() > 0) {
						std::cout << "Escape!" << std::endl;
					}
					break;
				case kKey_Left:
				case kKey_Up:
				case kKey_Right:
				case kKey_Down:
					// handle arrow keys
					if(VerboseLevel() > 0) {
						std::cout << "Moving histogram" << std::endl;
					}
					Move1DHistogram(keySymbol, this);
					// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been updated ...
					UpdatePad();
					DrawRegions();
					UpdatePad();
					if(VerboseLevel() > 1) {
						PrintRegions();
					}
					break;
			}
			break;
		case kKeyRelease:
			break;
		case kButtonPress:
			switch(event->fCode) {
				case 1:
					if(event->fState == 0 && (fGate || fBackground || fRegion)) {
						fStartX = fPad->PixeltoX(event->fX);
						fStartY = fPad->PixeltoY(event->fY - fPad->GetWh());
						fCurrentRegion = new TBox(fStartX, fStartY, fStartX, fStartY);
						if(fGate) {
							fCurrentRegion->SetFillColorAlpha(kRed, 0.2);
						} else if(fBackground) {
							fCurrentRegion->SetFillColorAlpha(kBlue, 0.2);
						} else if(fRegion) {
							fCurrentRegion->SetFillColorAlpha(fRegionColor[fNofRegions%fRegionColor.size()], 0.2);
						}
						fCurrentRegion->Draw();
						if(VerboseLevel() > 1) {
							PrintRegions();
						}
					}
					break;
				case 4:
				case 5:
					// code 4 & 5 are wheel down & up, respectively
					// state 0 - no key, 1 - shift, 4 - control, 8 - option, 16 - command
					if(event->fState == 0) {
						// we want to increase/reduce our range by 10% w/o changing the minimum
						// for wheel down/up, respectively
						double factor = 0.90;
						if(event->fCode == 4) {
							factor = 1.10;
						}
						if(VerboseLevel() > 0) {
							std::cout << "factor " << factor << ": maximum " << GetMinimum() + (GetMaximum() - GetMinimum())*factor << ", old range " << (GetMaximum() - GetMinimum()) << " = " << GetMaximum() << " - " << GetMinimum() << std::endl;
						}
						SetMaximum(GetMinimum() + (GetMaximum() - GetMinimum())*factor);
						if(VerboseLevel() > 0) {
							std::cout << "new range: " << (GetMaximum() - GetMinimum()) << " = " << GetMaximum() << " - " << GetMinimum() << std::endl;
						}
					} else if(event->fState == 1) {
						if(event->fCode == 4) {
							Move1DHistogram(kKey_Left, this);
						} else {
							Move1DHistogram(kKey_Right, this);
						}
					}
					// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been updated ...
					UpdatePad();
					DrawRegions();
					UpdatePad();
					if(VerboseLevel() > 1) {
						PrintRegions();
					}
					break;
				default:
					break;
			}
			break;
		case kMotionNotify:
			if(event->fState == 256 && (fGate || fBackground || fRegion)) {
				double currentX = fPad->PixeltoX(event->fX);
				double currentY = fPad->PixeltoY(event->fY - fPad->GetWh());
				if(VerboseLevel() > 0) {
					std::cout << "current box: " << fStartX << " - " << fStartY << ", " << currentX << " - " << currentY << ", canvas: " << fPad->PixeltoX(fPad->GetCanvas()->GetEventX()) << " - " << fPad->PixeltoY(fPad->GetCanvas()->GetEventY()) << std::endl;
				}
				fCurrentRegion->SetX2(currentX);
				fCurrentRegion->SetY2(currentY);
				fCurrentRegion->Draw();
				UpdatePad();
				if(VerboseLevel() > 1) {
					PrintRegions();
				}
			}
			break;
		case kButtonRelease:
			if(event->fCode == 1) {
				if(event->fState == 0) {
					if(!fGate && !fBackground && !fRegion) {
						// check whether we are in the frame or not
						double currentX = fPad->PixeltoX(event->fX);
						double currentY = fPad->PixeltoY(event->fY - fPad->GetWh());
						if(VerboseLevel() > 1) { std::cout << "current x " << currentX << ", current y " << currentY << std::endl; }
						if(Pad()->GetFrame()->GetX1() < currentX && currentX < Pad()->GetFrame()->GetX2() &&
							Pad()->GetFrame()->GetY1() < currentY && currentY < Pad()->GetFrame()->GetY2()) {
							if(VerboseLevel() > 0) {
								std::cout << "inside frame" << std::endl;
							}
						} else {
							// not a new gate, background, or region, and we're outside the frame => we just zoomed in on the x-axis
							GetYaxis()->UnZoom();
							if(VerboseLevel() > 0) {
								std::cout << "outside frame" << std::endl;
							}
						}
						// we need to update the pad to get the new frame the histogram is draw in, then update it again after the regions have been updated ...
						UpdatePad();
						DrawRegions();
						UpdatePad();
						if(VerboseLevel() > 1) {
							PrintRegions();
						}
					} else {
						double stopX = fPad->PixeltoX(event->fX);
						double stopY = fPad->PixeltoY(event->fY - fPad->GetWh());
						fCurrentRegion->SetX2(stopX);
						fCurrentRegion->SetY2(stopY);
						if(fGate) {
							if(VerboseLevel() > 0) {
								std::cout << "new gate: " << fStartX << " - " << stopX << std::endl;
							}
							fRegions.Add(new TRegion(fCurrentRegion, ERegionType::kGate, this));
							fGate = false;
						} else if(fBackground) {
							if(VerboseLevel() > 0) {
								std::cout << "new background: " << fStartX << " - " << stopX << std::endl;
							}
							fRegions.Add(new TRegion(fCurrentRegion, ERegionType::kBackground, this));
							fBackground = false;
						} else if(fRegion) {
							if(VerboseLevel() > 0) {
								std::cout << "new region: " << fStartX << " - " << stopX << std::endl;
							}
							fRegions.Add(new TRegion(fCurrentRegion, ERegionType::kRegion, this));
							fRegion = false;
							++fNofRegions;
						}
						RemoveCurrentRegion();
						static_cast<TRegion*>(fRegions.Last())->Draw();
						if(VerboseLevel() > 1) {
							PrintRegions();
						}
					}
				}
			}
			break;
		default:
			break;
	}
}

void GH1D::UpdateRegions()
{
	for(auto* obj : fRegions) {
		static_cast<TRegion*>(obj)->Update();
	}
}

void GH1D::DrawRegions(Option_t* opt)
{
	for(auto* obj : fRegions) {
		static_cast<TRegion*>(obj)->Draw(opt);
	}
}

void GH1D::PrintRegions()
{
	std::cout << GetName() << "/" << GetTitle() << " - Regions:" << std::endl;
	int index = 0;
	for(auto* obj : *(Pad()->GetListOfPrimitives())) {
		if(obj->InheritsFrom(TRegion::Class())) {
			std::cout << index << ": " << obj << " ";
			obj->Print();
			++index;
		}
	}
}

void GH1D::RemoveCurrentRegion()
{
	fGate = false;
	fBackground = false;
	fRegion = false;
	fPad->GetListOfPrimitives()->Remove(fCurrentRegion);
	delete fCurrentRegion;
	fCurrentRegion = nullptr;
}

void GH1D::RemoveRegion(TRegion* region)
{
	fPad->GetListOfPrimitives()->Remove(region);
	fRegions.Remove(region);
}

TRegion::TRegion(TBox* box, ERegionType type, GH1D* parent)
	: TBox(*box), fParent(std::move(parent)), fType(std::move(type)), fLowX(box->GetX1()), fHighX(box->GetX2())
{
	// make sure that x1 is smaller than x2
	if(GetX1() > GetX2()) {
		double tmpX = GetX1();
		SetX1(GetX2());
		SetX2(tmpX);
		std::swap(fLowX, fHighX);
	}
	// align the edges of the region with the bins of the histogram
	auto* axis = parent->GetXaxis();
	fLowX = axis->GetBinLowEdge(axis->FindBin(fLowX));
	fHighX = axis->GetBinUpEdge(axis->FindBin(fHighX));
}

bool TRegion::Update()
{
	/// Updates y-range of this region to match the current y-range of the frame the parent is drawn in.
	/// Also checks the x-range and adjust it to only cover the displayed range if necessary.
	/// Returns whether the region should be drawn or not.
	/// Does not update the pad itself!

	bool result = true;

	auto* frame = fParent->Pad()->GetFrame();
	if(GH1D::VerboseLevel() > 1) {
		std::cout << "Frame: " << frame->GetX1() << " - " << frame->GetX2() << ", " << frame->GetY1() << " - " << frame->GetY2() << " -> updating y-range from " << GetY1() << " - " << GetY2();
	}
	if(fParent->Pad()->GetLogy() == 0) {
		// linear scale: set y-range directly from frame
		SetY1(frame->GetY1());
		SetY2(frame->GetY2());
	} else {
		// logarithmic scale: frame reports the exponent as y1 and y2
		SetY1(TMath::Power(10., frame->GetY1()));
		SetY2(TMath::Power(10., frame->GetY2()));
	}
	if(GH1D::VerboseLevel() > 1) {
		std::cout << " to " << GetY1() << " - " << GetY2() << std::endl;
	}

	if(GH1D::VerboseLevel() > 1) {
		std::cout << "Updating x-range from " << GetX1() << " - " << GetX2() << " (" << fLowX << " - " << fHighX << ")";
	}
	if(frame->GetX1() < fLowX && fHighX < frame->GetX2()) {
		// whole region is visible -> use original range
		if(GH1D::VerboseLevel() > 1) {
			std::cout << " (whole region)";
		}
		SetX1(fLowX);
		SetX2(fHighX);
	} else if(frame->GetX2() < fLowX || fHighX < frame->GetX1()) {
		// region is out of frame, do nothing? What if the region is just out of the frame but in the canvas?
		// Using GetUxmin() and GetUxmax() to set range to values outside the pad range and not just the frame range does not work.
		// So we simply hide this region whenever it is outside the frame.
		// This also means we have to actively draw it whenever it is (partly) in frame (in case it was hidden before).
		if(GH1D::VerboseLevel() > 1) {
			std::cout << " (out of " << frame->GetX1() << " - " << frame->GetX2() << ")";
		}
		result = false;
	} else if(fHighX < frame->GetX2()) {
		// low part of region is out of frame -> adjust low x
		if(GH1D::VerboseLevel() > 1) {
			std::cout << " (high part)";
		}
		SetX1(frame->GetX1());
		SetX2(fHighX);
	} else if(frame->GetX1() < fLowX) {
		// high part of region is out of frame -> adjust high x
		if(GH1D::VerboseLevel() > 1) {
			std::cout << " (low part)";
		}
		SetX1(fLowX);
		SetX2(frame->GetX2());
	} else {
		// region is larger than visible range on both side -> adjust both
		if(GH1D::VerboseLevel() > 1) {
			std::cout << " (region too big)";
		}
		SetX1(frame->GetX1());
		SetX2(frame->GetX2());
	}
	if(GH1D::VerboseLevel() > 1) {
		std::cout << " to " << GetX1() << " - " << GetX2() << std::endl;
	}

	return result;
}

void TRegion::Draw(Option_t* opt)
{
	/// Draw this region. Calls Update() first to update ranges and to check if we actuall want to draw this range.
	/// If Update() returns false this function will actually hide this region!
	/// Updates the pad at the end.
	if(fParent == nullptr) {
		throw std::runtime_error("Can't draw region without parent histogram!");
	}

	if(Update()) {
		if(fParent->Pad()->GetListOfPrimitives()->FindObject(this) == nullptr) {
			TBox::Draw(opt);
		} else if(GH1D::VerboseLevel() > 0) {
			std::cout << this << " region " << fLowX << " - " << fHighX << " already has been drawn" << std::endl;
		}
	} else {
		Hide();
	}
	fParent->UpdatePad();
}

void TRegion::Hide()
{
	/// Hide this region. Removes region from list of primitives. Does not update the pad.
	if(fParent == nullptr) {
		throw std::runtime_error("Can't draw region without parent histogram!");
	}

	fParent->Pad()->GetListOfPrimitives()->Remove(this);
	if(GH1D::VerboseLevel() > 1) {
		std::cout << "hid region " << fLowX << " - " << fHighX << std::endl;
		fParent->PrintRegions();
	}
}

void TRegion::Update(double startX, double stopX)
{
	/// Depending on the starting position this function updates the left or right edge of the region to the stopping position.
	if(GH1D::VerboseLevel() > 1) {
		std::cout << "region " << fLowX << " - " << fHighX << ", start x " << startX << ", stop x " << stopX;
	}
	if(std::abs(startX-fLowX) < std::abs(startX-fHighX)) {
		fLowX = stopX;
	} else {
		fHighX = stopX;
	}
	if(GH1D::VerboseLevel() > 1) {
		std::cout << " => " << fLowX << " - " << fHighX << std::endl;
	}
	Update();
}
