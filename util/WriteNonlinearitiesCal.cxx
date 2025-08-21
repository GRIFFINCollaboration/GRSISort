#include "TFile.h"

#include "TChannel.h"

int main(int argc, char** argv)
{
   if(argc != 4) {
      std::cout << "Usage: " << argv[0] << " <root file> <graph names> <cal file>" << std::endl;
      return 1;
   }

   // try to open the root file
   auto* input = new TFile(argv[1]);
   if(!input->IsOpen()) {
      std::cout << "Failed to open '" << argv[1] << "'" << std::endl;
      return 1;
   }

   // read all channels from the root file
   TChannel::ReadEnergyNonlinearities(input, argv[2], true);

   // write result to new cal file
   TChannel::WriteCalFile(argv[3]);

   input->Close();

   return 0;
}
