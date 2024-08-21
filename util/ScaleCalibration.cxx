#include <unordered_map>

#include "TChannel.h"

int main(int argc, char** argv)
{
   if(argc < 3) {
      printf("Usage: AddOneToChannel <scale by> <calfile.cal> <MNEMONIC>\n");
      return 1;
   }

   std::string mnemonic;
   if(argc == 4) {
      mnemonic.assign(argv[3]);
   }

   std::string calFileName(argv[2], 40);
   std::string outFileName = "out" + calFileName;

   double num_to_scale = atof(argv[1]);

   printf("Multiplying all channels by %lf\n", num_to_scale);

   // Read Cal file
   TChannel::ReadCalFile(argv[2]);
   std::unordered_map<unsigned int, TChannel*>* chanmap = TChannel::GetChannelMap();

   if(chanmap == nullptr) {
      printf("can't find channel map\n");
      return 1;
   }

   std::vector<TChannel*> chanlist;

   for(auto iter : *chanmap) {
      TChannel* chan    = iter.second;
      auto*     newchan = new TChannel(chan);
      chanlist.push_back(newchan);

      if(!mnemonic.empty()) {
         std::string channame = newchan->GetName();

         // Could also do everything below with MNEMONIC Struct. This limits the amount of string processing that needs
         // to be done
         // Because it returns false after every potential failure while the mnemonic class sets all of the strings, and
         // then checks
         // for conditions.
         if(channame.compare(0, mnemonic.length(), mnemonic) != 0) {   // channame.NotBeginsWith(pre)){
            continue;
         }
      }

      std::vector<float> ENGCoeffs = newchan->GetENGCoeff();
      newchan->DestroyENGCal();
      for(float& ENGCoeff : ENGCoeffs) {
         newchan->AddENGCoefficient(ENGCoeff * num_to_scale);
      }
   }

   TChannel::DeleteAllChannels();

   for(auto& chan : chanlist) {
      TChannel::AddChannel(chan);
   }

   TChannel::WriteCalFile(outFileName);

   return 0;
}
