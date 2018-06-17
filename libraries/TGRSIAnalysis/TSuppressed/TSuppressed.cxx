#include "TSuppressed.h"

ClassImp(TSuppressed)

void TSuppressed::Copy(TObject& rhs) const
{
   // Copy function.
   TGRSIDetector::Copy(rhs);
}

void TSuppressed::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   TGRSIDetector::Clear(opt);
}
