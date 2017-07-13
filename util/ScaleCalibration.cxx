#include "TChannel.h"
#include <map>

int main(int argc, char** argv)
{
   if(argc < 3) {
      printf("Usage: AddOneToChannel <scale by> <calfile.cal> <MNEMONIC>\n");
      return 0;
   }

   std::string mnemonic;
   if(argc == 4) {
      mnemonic.assign(argv[3]);
   }

   std::string calFileName(argv[2], 40);
   std::string outFileName = "out" + calFileName;

   float num_to_scale = atof(argv[1]);

   printf("Multiplying all channels by %lf\n", num_to_scale);

   // Read Cal file
   TChannel::ReadCalFile(argv[2]);
   std::map<unsigned int, TChannel*>::iterator it;
   std::map<unsigned int, TChannel*>*          chanmap = TChannel::GetChannelMap();

   if(chanmap == nullptr) {
      printf("can't find channel map\n");
      return 0;
   }

   std::vector<TChannel*> chanlist;

   for(it = chanmap->begin(); it != chanmap->end(); ++it) {
      TChannel* chan    = it->second;
      auto*     newchan = new TChannel(chan);
      chanlist.push_back(newchan);

      if(mnemonic != "") {
         std::string channame = newchan->GetName();

         // Could also do everything below with MNEMONIC Struct. This limits the amount of string processing that needs
         // to be done
         // Because it returns false after every potential failure while the mnemonic class sets all of the strings, and
         // then checks
         // for conditions.
         if(channame.compare(0, mnemonic.length(), mnemonic) != 0) { // channame.NotBeginsWith(pre)){
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

   for(auto& i : chanlist) {
      TChannel::AddChannel(i);
   }

   TChannel::WriteCalFile(outFileName);

   return 1;
}
