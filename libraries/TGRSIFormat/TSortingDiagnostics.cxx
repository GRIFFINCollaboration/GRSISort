#include "TSortingDiagnostics.h"

#include <fstream>
#include <string>

#include "TChannel.h"
#include "TGRSIOptions.h"

TSortingDiagnostics* TSortingDiagnostics::fSortingDiagnostics = nullptr;

TSortingDiagnostics::TSortingDiagnostics() : TObject()
{
   Clear();
}

TSortingDiagnostics::TSortingDiagnostics(const TSortingDiagnostics&) : TObject()
{
   Clear();
}

TSortingDiagnostics::~TSortingDiagnostics() = default;

void TSortingDiagnostics::Copy(TObject& obj) const
{
   static_cast<TSortingDiagnostics&>(obj).fFragmentsOutOfOrder = fFragmentsOutOfOrder;
}

void TSortingDiagnostics::Clear(Option_t*)
{
   fFragmentsOutOfOrder.clear();
}

void TSortingDiagnostics::OutOfOrder(long newFragTS, long oldFragTS, long newEntry)
{
   fFragmentsOutOfOrder[oldFragTS] = std::make_pair(oldFragTS - newFragTS, newEntry);
   // try and find a timestamp before newFragTS
   size_t entry = 0;
   if(!fPreviousTimeStamps.empty()) {
      for(entry = fPreviousTimeStamps.size() - 1; entry > 0; --entry) {
         if(fPreviousTimeStamps[entry] < newFragTS) {
            break;
         }
      }
   }
   long entryDiff = newEntry - (entry * TGRSIOptions::Get()->SortDepth());
   if(entryDiff > fMaxEntryDiff) {
      fMaxEntryDiff = entryDiff;
   }
}

void TSortingDiagnostics::Print(Option_t* opt) const
{
   TString option = opt;
   option.ToUpper();
   std::string color;
   if(fFragmentsOutOfOrder.empty()) {
      if(option.EqualTo("ERROR")) {
         color = DGREEN;
      }
      std::cout<<color<<"No fragments out of order!"<<RESET_COLOR<<std::endl;
      return;
   }
   if(option.EqualTo("ERROR")) {
      color = DRED;
   }
   std::cerr<<color<<NumberOfFragmentsOutOfOrder()<<" fragments were out of order, maximum entry difference was "
            <<fMaxEntryDiff<<"!"<<std::endl
            <<"Please consider increasing the sort depth with --sort-depth="<<fMaxEntryDiff<<RESET_COLOR
            <<std::endl;
}

void TSortingDiagnostics::Draw(Option_t*)
{
}

void TSortingDiagnostics::WriteToFile(const char* fileName) const
{
   std::ofstream statsOut(fileName);
   statsOut<<std::endl
           <<"Number of fragments out of order = "<<NumberOfFragmentsOutOfOrder()<<std::endl
           <<"Maximum entry difference = "<<fMaxEntryDiff<<std::endl
           <<std::endl;
}
