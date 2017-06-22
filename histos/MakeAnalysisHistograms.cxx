#include "TRuntimeObjects.h"

#include "TTigress.h"

extern "C" void MakeAnalysisHistograms(TRuntimeObjects& obj)
{
   auto tig = obj.GetDetector<TTigress>();

   bool has_tig = (tig != nullptr);
   obj.FillHistogram("has_tig", 2, 0, 2, has_tig);
}
