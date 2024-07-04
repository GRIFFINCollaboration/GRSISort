#include "TChannel.h"
#include <unordered_map>

int main(int argc, char** argv)
{
   if(argc != 3) {
      printf("Usage: AddOneToChannel <int to add> <calfile.cal>\n");
      return 0;
   }

   int num_to_add = atoi(argv[1]);

   printf("Adding %d to all channels\n", num_to_add);

   // Read Cal file
   TChannel::ReadCalFile(argv[2]);
   std::unordered_map<unsigned int, TChannel*>* chanmap = TChannel::GetChannelMap();

   if(chanmap == nullptr) {
      printf("can't find channel map\n");
      return 0;
   }

   std::vector<TChannel*> chanlist;

   for(auto iter = chanmap->begin(); iter != chanmap->end(); ++iter) {
      TChannel* chan    = iter->second;
      auto*     newchan = new TChannel(chan);
      chanlist.push_back(newchan);
      newchan->SetNumber(TPriorityValue<int>(newchan->GetNumber() + num_to_add, EPriority::kUser));
   }

   TChannel::DeleteAllChannels();

   for(auto& channel : chanlist) {
      TChannel::AddChannel(channel);
   }

   TChannel::WriteCalFile(argv[2]);

   return 1;
}
