#include "TCalList.h"
#include "Globals.h"

#include <iostream>

TCalList::TCalList()
   : TNamed()
{
   Clear();
}

TCalList::~TCalList() = default;

TCalList::TCalList(const char* name, const char* title) : TNamed(name, title)
{
   Clear();
}

TCalList::TCalList(const TCalList& copy) : TNamed(copy)
{
   copy.Copy(*this);
}

void TCalList::Copy(TObject& obj) const
{
   TNamed::Copy(obj);
   static_cast<TCalList&>(obj).Clear();
   for(auto it : fCalList) {
      static_cast<TCalList&>(obj).AddPoint(it.second);
   }
}

void TCalList::AddPoint(const TCalPoint& point)
{
   AddPoint(std::floor(point.Centroid()), point);
}

void TCalList::AddPoint(const UInt_t& idx, const TCalPoint& point)
{
   fCalList.insert(std::make_pair(idx, point));
}

bool TCalList::SetPointIndex(const UInt_t& old_idx, const UInt_t& new_idx)
{
   auto it = fCalList.find(old_idx);
   if(it != fCalList.end()) {
      // This means we found the old index! no go ahead and save the pair
      TCalPoint sav_pt = it->second;
      // delete the old pair
      fCalList.erase(it);
      // insert the new pair
      fCalList.insert(std::make_pair(new_idx, sav_pt));
      return true;
   }
   // didn't find it, return false
   return false;
}

void TCalList::Print(Option_t*) const
{
   int idx = 0;
   std::cout << GetName() << "   " << GetTitle() << std::endl;
   for(auto it : fCalList) {
      std::cout << idx++ << "    " << it.first << std::endl;
      it.second.Print();
   }
}

void TCalList::Clear(Option_t*)
{
   fCalList.clear();
}

void TCalList::FillGraph(TGraph* graph) const
{
   graph->Clear();
   Int_t         i  = 0;
   TGraphErrors* ge = static_cast<TGraphErrors*>(graph);

   for(auto it : fCalList) {
      graph->SetPoint(i, it.second.Centroid(), it.second.Area());
      if(ge != nullptr) {
         ge->SetPointError(i++, it.second.CentroidErr(), it.second.AreaErr());
      }
   }
}
