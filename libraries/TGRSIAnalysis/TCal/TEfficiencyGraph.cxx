#include "TEfficiencyGraph.h"

#include <iostream>
#include "Globals.h"

/// \cond CLASSIMP
ClassImp(TEfficiencyGraph)
/// \endcond

TEfficiencyGraph::TEfficiencyGraph()
   : TCalGraph(), fIsAbsolute(false)
{
   Clear();
}

TEfficiencyGraph::~TEfficiencyGraph() = default;

TEfficiencyGraph::TEfficiencyGraph(const TEfficiencyGraph& copy) : TCalGraph(copy)
{
   //  copy.Copy(*this);
   fIsAbsolute = copy.fIsAbsolute;
}

void TEfficiencyGraph::Print(Option_t* opt) const
{
   TCalGraph::Print(opt);
}

void TEfficiencyGraph::Clear(Option_t* opt)
{
   TCalGraph::Clear(opt);
}

void TEfficiencyGraph::BuildGraph()
{
   ClearAllPoints();
   for(auto it : fCompareMap) {
      auto     pair              = it.second;
      Double_t src_energy        = pair.second.Centroid();
      Double_t src_energy_err    = pair.second.CentroidErr();
      Double_t src_intensity     = pair.second.Area();
      Double_t src_intensity_err = pair.second.AreaErr();

      Double_t meas_area     = pair.first.Area();
      Double_t meas_area_err = pair.first.AreaErr();

      Int_t point_to_add = GetN();

      SetPoint(point_to_add, src_energy, meas_area / src_intensity);
      SetPointError(point_to_add, src_energy_err,
                    meas_area / src_intensity * TMath::Sqrt(TMath::Power(meas_area_err / meas_area, 2.) +
                                                            TMath::Power(src_intensity_err / src_intensity, 2.0)));
   }
}

void TEfficiencyGraph::Scale(const Double_t& scale)
{
   for(auto& it : fCompareMap) {
      auto& pair = it.second;
      pair.first.SetArea(pair.first.Area() * scale, pair.first.AreaErr() * scale);
   }
   for(int i = 0; i < GetN(); ++i) {
      GetY()[i] *= scale;
      GetEY()[i] *= scale;
   }
}
