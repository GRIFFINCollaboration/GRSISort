#include "TChannel.h"
#include <unordered_map>

int main(int argc, char** argv)
{
   if(argc != 3) {
      std::cout << "Usage: AddOneToChannel <int to add> <calfile.cal>\n";
      return 1;
   }

   int numToAdd = atoi(argv[1]);

   std::cout << "Adding " << numToAdd << " to all channels\n";

   // Read Cal file
   TChannel::ReadCalFile(argv[2]);
   std::unordered_map<unsigned int, TChannel*>* chanmap = TChannel::GetChannelMap();

   if(chanmap == nullptr) {
      std::cout << "can't find channel map\n";
      return 1;
   }

   std::vector<TChannel*> chanlist;

   for(auto& iter : *chanmap) {
      TChannel* chan    = iter.second;
      auto*     newchan = new TChannel(chan);
      chanlist.push_back(newchan);
      newchan->SetNumber(TPriorityValue<int>(newchan->GetNumber() + numToAdd, EPriority::kUser));
   }

   TChannel::DeleteAllChannels();

   for(auto& channel : chanlist) {
      TChannel::AddChannel(channel);
   }

   TChannel::WriteCalFile(argv[2]);

   return 0;
}
