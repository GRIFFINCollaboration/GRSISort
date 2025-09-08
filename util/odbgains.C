#include <iostream>
#include <fstream>

#include "TChannel.h"

void ODBGains(char* fname, const char* outname = "odbgains.sh")
{
   // Script to make a script to write to ODB automatically from a created cal file.
   // Some assertions and exceptions could be useful here, but for the amount this will
   // be used, I'm not going to spend time doing it.
   // Please send any other complaints to rd.

   if(TChannel::ReadCalFile(fname) == 0) {
      return;
   }

   std::ofstream myfile;
   myfile.open(outname, std::ios::out);

   myfile << "#!/bin/bash\n";
   myfile << "#\n";
   myfile << "# Set the correct gain and offset parameters\n";
   myfile << "# Generated automatically by ODBGains in the GRSISort package\n";
   myfile << "# using " << fname << "\n\n";

   for(int i = 0; i < 64; i++) {   // make this smarter
      TChannel* chan = TChannel::GetChannelByNumber(i);
      if(chan == nullptr) { continue; }

      myfile << "odbedit -c \"set /DAQ/PSC/offset[" << i << "] " << chan->GetENGCoeff().at(0) << "\"\n";
      myfile << "odbedit -c \"set /DAQ/PSC/gain[" << i << "] " << chan->GetENGCoeff().at(1) << "\"\n\n";
   }

   myfile.close();
}
