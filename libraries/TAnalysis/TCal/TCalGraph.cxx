#include "TCalGraph.h"

#include <iostream>
#include "Globals.h"

TCalGraph::TCalGraph()
{
   Clear();
}

TCalGraph::TCalGraph(const TCalGraph& copy) : TGraphErrors(copy)
{
   //  copy.Copy(*this);
}

void TCalGraph::Print(Option_t* opt) const
{
   std::cout << DRED;
   TGraphErrors::Print(opt);
   std::cout << RESET_COLOR;
   for(const auto& iter : fCompareMap) {
      std::cout << DBLUE << " Data " << RESET_COLOR << std::endl;
      iter.second.first.Print();
      std::cout << DGREEN << " Source " << RESET_COLOR << std::endl;
      iter.second.second.Print();
      std::cout << std::endl;
   }
}

void TCalGraph::Clear(Option_t* opt)
{
   TGraphErrors::Clear(opt);
   fCompareMap.clear();
}

void TCalGraph::AddPoint(const TCalPoint& cal_point)
{
   Int_t idx = GetN();
   SetPoint(idx, cal_point.Centroid(), cal_point.Area());
   SetPointError(idx, cal_point.CentroidErr(), cal_point.AreaErr());
}

Int_t TCalGraph::FindClosestPointX(const Double_t& x_val)
{
   Int_t     closest_idx  = 0;
   Double_t  closest_dist = 999999.;
   Double_t* x_arr        = GetX();
   for(int i = 0; i < GetN(); ++i) {
      if(std::abs(x_arr[i] - x_val) < closest_dist) {
         closest_idx  = i;
         closest_dist = std::abs(x_arr[i] - x_val);
      }
   }
   return closest_idx;
}

Double_t TCalGraph::FindDistToClosestPointX(const Double_t& x_val)
{
   Int_t idx = FindClosestPointX(x_val);
   return std::fabs(GetX()[idx] - x_val);
}

Int_t TCalGraph::AddLists(const TCalList& cal_list, const TSourceList& src_list)
{
   // This is where we build the compared list
   const auto& cal_map = cal_list.Map();
   const auto& src_map = src_list.Map();

   std::vector<std::pair<UInt_t, Double_t>> missing_cal_values_vec;
   std::vector<std::pair<UInt_t, Double_t>> missing_src_values_vec;
   TCalList                                 missing_cal_values;
   TCalList                                 missing_src_values;

   // Look in cal_list for src_list entries
   for(const auto& cal : cal_map) {
      const auto& iter = src_map.find(cal.first);
      if(iter != src_map.end()) {   // we found the matching data point
         fCompareMap.insert(std::make_pair(cal.first, std::make_pair(cal.second, iter->second)));
      } else {
         missing_cal_values.AddPoint(cal.second);
         missing_cal_values_vec.emplace_back(cal.first, cal.second.Centroid());
      }
   }

   // Find if anything is missing from source list
   for(const auto& src_it : src_map) {
      const auto& cal_it = cal_map.find(src_it.first);
      if(cal_it == cal_map.end()) {   // we couldn't find the src data
         missing_src_values.AddPoint(src_it.second);
         missing_src_values_vec.push_back(std::make_pair(src_it.first, src_it.second.Centroid()));
      }
   }

   if(!missing_cal_values_vec.empty()) {
      std::cout << DRED << "Couldn't find matching cal points for: " << std::endl;
      for(auto mit : missing_cal_values_vec) {
         std::cout << mit.first << " Centroid = " << mit.second << std::endl;
      }
   }
   if(!missing_src_values_vec.empty()) {
      std::cout << DGREEN << "Couldn't find matching source points for: " << std::endl;
      for(auto mit : missing_src_values_vec) {
         std::cout << mit.first << " Centroid = " << mit.second << std::endl;
      }
   }
   std::cout << RESET_COLOR;

   CorrectMissingPoints(missing_cal_values, missing_src_values);

   return static_cast<Int_t>(fCompareMap.size());
}

void TCalGraph::CorrectMissingPoints(TCalList& cal_list, TCalList& src_list)
{
   const auto& cal_map = cal_list.Map();
   const auto& src_map = src_list.Map();
   std::cout << "Attempting to match points within +/- 2 keV" << std::endl;
   for(const auto& cal_it : cal_map) {
      for(const auto& src_it : src_map) {
         if(std::abs(static_cast<Int_t>(cal_it.first - src_it.first)) < 3.0) {
            fCompareMap.insert(std::make_pair(src_it.first, std::make_pair(cal_it.second, src_it.second)));
            std::cout << "Matched: " << cal_it.first << " and " << src_it.first << std::endl;
            break;
         }
      }
   }
}

void TCalGraph::ClearAllPoints(Option_t*)
{
   // Clear all of the data points
   while(RemovePoint(0) != -1) {
   }
}
