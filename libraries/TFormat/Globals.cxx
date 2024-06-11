#include "Globals.h"

void grsi::SetGRSIEnv()
{
   /// function to read user defined options first from the .grsirc file in $GRSISYS, then the .grsirc file in $HOME
   std::string path = getenv("GRSISYS");   // Finds the GRSISYS path to be used by other parts of the grsisort code
   if(path.length() > 0) {
      path += "/";
   }
   // Read in grsirc in the GRSISYS directory to set user defined options on grsisort startup
   path += ".grsirc";
   gEnv->ReadFile(path.c_str(), kEnvChange);
   // read from home directory for user-specific settings (in case $GRSISYS is a multi-user installation)
   path = getenv("HOME");   // Finds the HOME path to be used by other parts of the grsisort code
   if(path.length() > 0) {
      path += "/";
   }
   // Read in grsirc in the HOME directory to set user defined options on grsisort startup - these overwrite previous settings
   path += ".grsirc";
   gEnv->ReadFile(path.c_str(), kEnvChange);
}
