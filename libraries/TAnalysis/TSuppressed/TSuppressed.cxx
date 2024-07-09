#include "TSuppressed.h"

void TSuppressed::Copy(TObject& rhs) const
{
   // Copy function.
   TDetector::Copy(rhs);
}

void TSuppressed::Clear(Option_t* opt)
{
   // Clears the mother, and all of the hits
   TDetector::Clear(opt);
}
