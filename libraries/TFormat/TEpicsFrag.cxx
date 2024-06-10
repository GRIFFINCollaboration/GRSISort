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

/// \cond CLASSIMP
ClassImp(TEpicsFrag)
/// \endcond

   std::vector<std::string> TEpicsFrag::fNameList;
std::map<Long64_t, TEpicsFrag> TEpicsFrag::fScalerMap;
Long64_t TEpicsFrag::fSmallestTime = std::numeric_limits<Long64_t>::max();

TEpicsFrag::TEpicsFrag()
{
   // Default Constructor.
   fDaqTimeStamp = 0;
   fDaqId        = -1;
}

TEpicsFrag::~TEpicsFrag() = default;

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
   std::cout<<"------ EPICS "<<largest<<" Varibles Found ------"<<std::endl;

   char buff[20];
   ctime(&fDaqTimeStamp);
   struct tm* timeInfo = localtime(&fDaqTimeStamp);
   strftime(buff, 20, "%b %d %H:%M:%S", timeInfo);

   std::cout<<"  DaqTimeStamp: "<<buff<<std::endl;
   std::cout<<"  DaqId:    	 "<<fDaqId<<std::endl;
   for(size_t i = 0; i < largest; i++) {
      std::cout<<std::setw(3)<<i<<":  ";
      std::cout<<std::setw(30)<<fName.at(i)<<" --- ";
      std::cout<<fData.at(i);
      std::cout<<std::endl;
   }
}

void TEpicsFrag::AddEpicsVariable(const char* name)
{
   fNameList.emplace_back(name);
}

std::string TEpicsFrag::GetEpicsVariableName(const int& i)
{
   try {
      return fNameList.at(i);
   } catch(const std::out_of_range& oor) {
      std::cout<<DRED<<"Could not find variable at position "<<i<<", returning nothing"<<std::endl;
      return "";
   }
}

void TEpicsFrag::PrintVariableNames()
{
   int idx = 0;
   for(const auto& i : fNameList) {
      std::cout<<idx++<<":  "<<i<<std::endl;
   }
}

void TEpicsFrag::SetEpicsNameList(const std::vector<std::string>& name_vec)
{
   fNameList.clear();
   for(const auto& i : name_vec) {
      fNameList.push_back(i);
   }
}

void TEpicsFrag::BuildScalerMap(TTree* tree)
{
   if(tree == nullptr) {
      std::cout<<DRED<<"Could not build map from tree"<<RESET_COLOR<<std::endl;
   }
   // Loop through the tree and insert the scalers into the map
   fScalerMap.clear();
   TEpicsFrag* my_frag = nullptr;
   if(tree->SetBranchAddress("TEpicsFrag", &my_frag) == 0) {
      for(int i = 0; i < tree->GetEntries(); ++i) {
         tree->GetEntry(i);
         if((static_cast<Long64_t>(my_frag->fDaqTimeStamp) - static_cast<Long64_t>(TRunInfo::Get()->RunStart())) < fSmallestTime) {
            fSmallestTime = static_cast<Long64_t>(my_frag->fDaqTimeStamp) - static_cast<Long64_t>(TRunInfo::Get()->RunStart());
         }
         fScalerMap[static_cast<Long64_t>(my_frag->fDaqTimeStamp) -
                    static_cast<Long64_t>(TRunInfo::Get()->RunStart())] = *my_frag;
      }
   } else {
      std::cout<<DRED<<"Could not build map from tree"<<RESET_COLOR<<std::endl;
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
         std::cout<<DRED<<"Could not build the epics map"<<RESET_COLOR<<std::endl;
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
         std::cout<<DRED<<"Could not build the epics map"<<RESET_COLOR<<std::endl;
         return;
      }
   }
   for(auto i : fScalerMap) {
      std::cout<<i.first<<"    "<<i.second.fDaqTimeStamp<<std::endl;
   }
}
