#include "TEpicsFrag.h"

#include <iostream>
#include <iomanip>
#include <stdexcept>
#include <limits>

#include <ctime>

#include "TChannel.h"
#include "TRunInfo.h"

///////////////////////////////////////////////////////////////
///
/// \class TEpicsFrag
///
/// This Class should contain all the information found in
/// NOT typeid 1 midas events. aka Epics (scaler) Events.
///
///
///////////////////////////////////////////////////////////////

std::vector<std::string> TEpicsFrag::fNameList;
std::map<Long64_t, TEpicsFrag> TEpicsFrag::fScalerMap;
Long64_t                       TEpicsFrag::fSmallestTime = std::numeric_limits<Long64_t>::max();

void TEpicsFrag::Clear(Option_t*)
{
   // Clears the TEpicsFrag.
   fDaqTimeStamp = 0;
   fDaqId        = -1;

   fName.clear();
   fData.clear();
}

void TEpicsFrag::Print(Option_t*) const
{
   // Prints the TEpicsFrag. This includes Midas information as well the data
   // kep inside of the scaler.
   size_t largest = fData.size();
   std::cout << "------ EPICS " << largest << " Varibles Found ------" << std::endl;

	// TODO maybe we can change this to std::array?
   char buff[20];
   ctime(&fDaqTimeStamp);
   struct tm* timeInfo = localtime(&fDaqTimeStamp);
   strftime(buff, 20, "%b %d %H:%M:%S", timeInfo);

   std::cout << "  DaqTimeStamp: " << buff << std::endl;
   std::cout << "  DaqId:    	 " << fDaqId << std::endl;
   for(size_t i = 0; i < largest; i++) {
      std::cout << std::setw(3) << i << ":  ";
      std::cout << std::setw(30) << fName.at(i) << " --- ";
      std::cout << fData.at(i);
      std::cout << std::endl;
   }
}

void TEpicsFrag::AddEpicsVariable(const char* name)
{
   fNameList.emplace_back(name);
}

std::string TEpicsFrag::GetEpicsVariableName(const int& index)
{
   try {
      return fNameList.at(index);
   } catch(const std::out_of_range& oor) {
      std::cout << DRED << "Could not find variable at position " << index << ", returning nothing" << std::endl;
      return "";
   }
}

void TEpicsFrag::PrintVariableNames()
{
   int idx = 0;
   for(const auto& name : fNameList) {
      std::cout << idx++ << ":  " << name << std::endl;
   }
}

void TEpicsFrag::SetEpicsNameList(const std::vector<std::string>& names)
{
   fNameList.clear();
   for(const auto& name : names) {
      fNameList.push_back(name);
   }
}

void TEpicsFrag::BuildScalerMap(TTree* tree)
{
   if(tree == nullptr) {
      std::cout << DRED << "Could not build map from tree" << RESET_COLOR << std::endl;
   }
   // Loop through the tree and insert the scalers into the map
   fScalerMap.clear();
   TEpicsFrag* my_frag = nullptr;
   if(tree->SetBranchAddress("TEpicsFrag", &my_frag) == 0) {
      for(int i = 0; i < tree->GetEntries(); ++i) {
         tree->GetEntry(i);
         if((static_cast<Long64_t>(my_frag->fDaqTimeStamp) - static_cast<Long64_t>(TRunInfo::RunStart())) < fSmallestTime) {
            fSmallestTime = static_cast<Long64_t>(my_frag->fDaqTimeStamp) - static_cast<Long64_t>(TRunInfo::RunStart());
         }
         fScalerMap[static_cast<Long64_t>(my_frag->fDaqTimeStamp) -
                    static_cast<Long64_t>(TRunInfo::RunStart())] = *my_frag;
      }
   } else {
      std::cout << DRED << "Could not build map from tree" << RESET_COLOR << std::endl;
   }
}

void TEpicsFrag::BuildScalerMap()
{
   TTree* scaler_tree = static_cast<TTree*>(gDirectory->Get("EpicsTree"));
   if(scaler_tree == nullptr) {
      return;
   }

   BuildScalerMap(scaler_tree);
}

TEpicsFrag* TEpicsFrag::GetScalerAtTime(Long64_t time)
{
   if(fScalerMap.empty()) {
      BuildScalerMap();
      if(fScalerMap.empty()) {
         std::cout << DRED << "Could not build the epics map" << RESET_COLOR << std::endl;
         return nullptr;
      }
   }
   if(time < fSmallestTime) {
      time = fSmallestTime;
   }
   return &((--(fScalerMap.upper_bound(time)))->second);
}

void TEpicsFrag::PrintScalerMap()
{
   if(fScalerMap.empty()) {
      BuildScalerMap();
      if(fScalerMap.empty()) {
         std::cout << DRED << "Could not build the epics map" << RESET_COLOR << std::endl;
         return;
      }
   }
   for(const auto& item : fScalerMap) {
      std::cout << item.first << "    " << item.second.fDaqTimeStamp << std::endl;
   }
}
