#include "GH2Base.h"

#include <iostream>

#include <TDirectory.h>

#include "GH1D.h"
#include "SuppressTH1GDirectory.h"

/// \cond CLASSIMP
ClassImp(GH2Base)
/// \endcond

GH2Base::~GH2Base()
{
   fProjections->Delete();
   fSummaryProjections->Delete();
}

void GH2Base::Init()
{
   fProjections        = new TList();
   fSummaryProjections = new TList();
   fIsSummary          = false;
   fSummaryDirection   = EDirection::kXDirection;
}

void GH2Base::GH2Clear(Option_t* opt)
{
   TString sopt(opt);
   fProjections->Clear();
   fSummaryProjections->Clear();
}

GH1D* GH2Base::Projection_Background(int axis, int firstbin, int lastbin, int first_bg_bin, int last_bg_bin,
                                     EBackgroundSubtraction mode)
{
   std::string title;
   std::string name;
   std::string sproj;
   TH1D*       proj    = nullptr;
   TH1D*       bg_proj = nullptr;

   double xlow, xhigh, bg_xlow, bg_xhigh;

   SuppressTH1GDirectory sup;

   if(axis == 0) {
      xlow     = GetTH2()->GetXaxis()->GetBinLowEdge(firstbin);
      xhigh    = GetTH2()->GetXaxis()->GetBinUpEdge(lastbin);
      bg_xlow  = GetTH2()->GetXaxis()->GetBinLowEdge(first_bg_bin);
      bg_xhigh = GetTH2()->GetXaxis()->GetBinUpEdge(last_bg_bin);
      sproj    = "projx";
      proj     = GetTH2()->ProjectionX("temp1", firstbin, lastbin);
      bg_proj  = GetTH2()->ProjectionX("temp2", first_bg_bin, last_bg_bin);
   } else if(axis == 1) {
      xlow     = GetTH2()->GetYaxis()->GetBinLowEdge(firstbin);
      xhigh    = GetTH2()->GetYaxis()->GetBinUpEdge(lastbin);
      bg_xlow  = GetTH2()->GetYaxis()->GetBinLowEdge(first_bg_bin);
      bg_xhigh = GetTH2()->GetYaxis()->GetBinUpEdge(last_bg_bin);
      sproj    = "projy";
      proj     = GetTH2()->ProjectionY("temp1", firstbin, lastbin);
      bg_proj  = GetTH2()->ProjectionY("temp2", first_bg_bin, last_bg_bin);
   } else {
      return nullptr;
   }
   name =
      Form("%s_%s_%d_%d_bg_%d_%d", GetTH2()->GetName(), sproj.c_str(), firstbin, lastbin, first_bg_bin, last_bg_bin);
   title = Form("%s_%s_%d[%.02f]_%d[%.02f]_bg_%d[%.02f]_%d[%.02f]", GetTH2()->GetName(), sproj.c_str(), firstbin, xlow,
                lastbin, xhigh, first_bg_bin, bg_xlow, last_bg_bin, bg_xhigh);

   double bg_scaling = double(lastbin - firstbin) / double(last_bg_bin - first_bg_bin);
   if(mode == EBackgroundSubtraction::kNoBackground) {
      bg_scaling = 0;
   }

   proj->Add(bg_proj, -bg_scaling);
   auto* output = new GH1D(*proj);
   proj->Delete();
   bg_proj->Delete();

   output->SetName(name.c_str());
   output->SetTitle(title.c_str());
   output->SetParent(GetTH2());
   output->SetProjectionAxis(axis);
   output->SetDirectory(nullptr);
   fProjections->Add(output);
   return output;
}

GH1D* GH2Base::GH2ProjectionX(const char* name, int firstbin, int lastbin, Option_t* option, bool KeepEmpty)
{
   std::string title;
   double      xlow  = GetTH2()->GetYaxis()->GetBinLowEdge(firstbin);
   double      xhigh = GetTH2()->GetYaxis()->GetBinUpEdge(lastbin);
   bool        total = false;
   if(firstbin == 0 && lastbin == -1) {
      total = true;
      title = Form("%s_totalx", GetTH2()->GetName());
   } else {
      title = Form("%s_projx_%d[%.02f]_%d[%.02f]", GetTH2()->GetName(), firstbin, xlow, lastbin, xhigh);
   }

   std::string actual_name = name;
   if(actual_name == "_px") {
      if(total) {
         actual_name = title;
      } else {
         actual_name = Form("%s_projx_%d_%d", GetTH2()->GetName(), firstbin, lastbin);
      }
   }

   GH1D* output = nullptr;
   {
      SuppressTH1GDirectory sup;
      TH1D*                 proj = GetTH2()->ProjectionX("temp", firstbin, lastbin, option);
      output                     = new GH1D(*proj);
      proj->Delete();
   }

   output->SetName(actual_name.c_str());
   output->SetTitle(title.c_str());
   output->SetParent(GetTH2());
   output->SetProjectionAxis(0);
   output->SetDirectory(nullptr);

   if(fIsSummary) {
      if(KeepEmpty || output->Integral() > 0) {
         fSummaryProjections->Add(output);
      }
   } else {
      if(KeepEmpty || output->Integral() > 0) {
         fProjections->Add(output);
      }
   }
   return output;
}

GH1D* GH2Base::ProjectionX_Background(int firstbin, int lastbin, int first_bg_bin, int last_bg_bin,
                                      EBackgroundSubtraction mode)
{
   return Projection_Background(0, firstbin, lastbin, first_bg_bin, last_bg_bin, mode);
}

GH1D* GH2Base::GH2ProjectionY(const char* name, int firstbin, int lastbin, Option_t* option, bool KeepEmpty)
{
   std::string title;
   double      ylow  = GetTH2()->GetXaxis()->GetBinLowEdge(firstbin);
   double      yhigh = GetTH2()->GetXaxis()->GetBinUpEdge(lastbin);
   bool        total = false;
   if(firstbin == 0 && lastbin == -1) {
      total = true;
      title = Form("%s_totaly", GetTH2()->GetName());
   } else {
      title = Form("%s_projy_%d[%.02f]_%d[%.02f]", GetTH2()->GetName(), firstbin, ylow, lastbin, yhigh);
   }

   std::string actual_name = name;
   if(actual_name == "_py") {
      if(total) {
         actual_name = title;
      } else {
         actual_name = Form("%s_projy_%d_%d", GetTH2()->GetName(), firstbin, lastbin);
      }
   }

   GH1D* output = nullptr;
   {
      SuppressTH1GDirectory sup;
      TH1D*                 proj = GetTH2()->ProjectionY("temp", firstbin, lastbin, option);
      output                     = new GH1D(*proj);
      proj->Delete();
   }

   output->SetName(actual_name.c_str());
   output->SetTitle(title.c_str());
   output->SetParent(GetTH2());
   output->SetProjectionAxis(1);
   output->SetDirectory(nullptr);

   if(fIsSummary) {
      if(KeepEmpty || output->Integral() > 0) {
         fSummaryProjections->Add(output);
      }
   } else {
      if(KeepEmpty || output->Integral() > 0) {
         fProjections->Add(output);
      }
   }
   return output;
}

GH1D* GH2Base::ProjectionY_Background(int firstbin, int lastbin, int first_bg_bin, int last_bg_bin,
                                      EBackgroundSubtraction mode)
{
   return Projection_Background(1, firstbin, lastbin, first_bg_bin, last_bg_bin, mode);
}

GH1D* GH2Base::GetPrevious(const GH1D* curr, bool)
{
   if(fIsSummary) {
      return GetPrevSummary(curr, false);
   }

   TObjLink* link = fProjections->FirstLink();
   while(link != nullptr) {
      if(link->GetObject() == curr) {
         break;
      }
      link = link->Next();
   }
   if(link == nullptr) {
      return nullptr;
   }

   if(link->Prev() != nullptr) {
      return static_cast<GH1D*>(link->Prev()->GetObject());
   }
   return static_cast<GH1D*>(fProjections->Last());
}

GH1D* GH2Base::GetNext(const GH1D* curr, bool)
{
   if(fIsSummary) {
      return GetNextSummary(curr, false);
   }

   TObjLink* link = fProjections->FirstLink();
   while(link != nullptr) {
      if(link->GetObject() == curr) {
         break;
      }
      link = link->Next();
   }
   if(link == nullptr) {
      return nullptr;
   }

   if(link->Next() != nullptr) {
      return static_cast<GH1D*>(link->Next()->GetObject());
   }
   return static_cast<GH1D*>(fProjections->First());
}

GH1D* GH2Base::GetNextSummary(const GH1D* curr, bool DrawEmpty)
{
   int         binnum = 1;
   std::string name;
   if(curr != nullptr) {
      name                  = curr->GetName();
      size_t underscore_pos = name.rfind('_');
      binnum                = std::atoi(name.c_str() + underscore_pos + 1);
      binnum++;
   }

   int max_binnum;
   if(fSummaryDirection == EDirection::kXDirection) {
      max_binnum = GetTH2()->GetXaxis()->GetNbins();
   } else {
      max_binnum = GetTH2()->GetYaxis()->GetNbins();
   }

   if(binnum > max_binnum) {
      binnum = 1;
   }
   GH1D* g         = nullptr;
   int   start_bin = binnum;
	switch(fSummaryDirection) {
		case EDirection::kXDirection:
			while(true) {
				std::string hist_name = Form("%s_%d", GetTH2()->GetName(), binnum);
				g                     = static_cast<GH1D*>(fSummaryProjections->FindObject(hist_name.c_str()));
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}

				g = GH2ProjectionY(hist_name.c_str(), binnum, binnum, "", DrawEmpty);
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}
				binnum++;
				if(binnum == start_bin) {
					break;
				} else if(binnum > max_binnum) {
					binnum = 1;
				}
			}
			break;
		case EDirection::kYDirection:
			while(true) {
				std::string hist_name = Form("%s_%d", GetTH2()->GetName(), binnum);

				g = static_cast<GH1D*>(fSummaryProjections->FindObject(hist_name.c_str()));
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}

				g = GH2ProjectionX(hist_name.c_str(), binnum, binnum, "", DrawEmpty);
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}
				binnum++;
				if(binnum == start_bin) {
					break;
				} else if(binnum > max_binnum) {
					binnum = 1;
				}
			}
			break;
	}
	return g;
	// return SummaryProject(binnum);
}

GH1D* GH2Base::GetPrevSummary(const GH1D* curr, bool DrawEmpty)
{
	int         binnum = 1;
	std::string name;
	if(curr != nullptr) {
		name                  = curr->GetName();
		size_t underscore_pos = name.rfind('_');
		binnum                = std::atoi(name.c_str() + underscore_pos + 1);
		binnum--;
	}

	int max_binnum;
	if(fSummaryDirection == EDirection::kXDirection) {
		max_binnum = GetTH2()->GetXaxis()->GetNbins();
	} else {
		max_binnum = GetTH2()->GetYaxis()->GetNbins();
	}

	if(binnum <= 0) {
		binnum = max_binnum;
	}

	std::string hist_name = Form("%s_%d", GetTH2()->GetName(), binnum);
	TObject*    obj       = fSummaryProjections->FindObject(hist_name.c_str());
	if(obj != nullptr) {
		return static_cast<GH1D*>(obj);
	}

	int start_bin = binnum;
	switch(fSummaryDirection) {
		case EDirection::kXDirection:
			while(true) {
				// std::string hist_name = Form("%s_%d",GetTH2()->GetName(),binnum);
				std::string hist_name2 = Form("%s_%d", GetTH2()->GetName(), binnum);
				GH1D*       g          = GH2ProjectionY(hist_name2.c_str(), binnum, binnum, "", DrawEmpty);
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}
				binnum--;
				if(binnum == start_bin) {
					break;
				} else if(binnum <= 0) {
					binnum = max_binnum;
				}
			}
			break;
		case EDirection::kYDirection:
			while(true) {
				// std::string hist_name = Form("%s_%d",GetTH2()->GetName(),binnum);
				std::string hist_name2 = Form("%s_%d", GetTH2()->GetName(), binnum);
				GH1D*       g          = GH2ProjectionX(hist_name2.c_str(), binnum, binnum, "", DrawEmpty);
				if((g != nullptr) && g->Integral() > 0) {
					return g;
				}
				binnum--;
				if(binnum == start_bin) {
					break;
				} else if(binnum <= 0) {
					binnum = max_binnum;
				}
			}
			break;
	}
	return nullptr;
	// return SummaryProject(binnum);
}

