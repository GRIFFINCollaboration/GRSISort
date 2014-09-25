#ifndef TGRSITRANSITION_H
#define TGRSITRANSITION_H

#include <cstdio>

#include <TClass.h>
#include <TObject.h>

class TGRSITransition : public TObject {
   public:
      TGRSITransition();
      ~TGRSITransition();

      double energy;
      double energy_uncertainity;
      double intensity;
      double intensity_uncertainity;

      bool IsSortable() const { return true; }

      int Compare(const TObject *obj) const;

   ClassDef(TGRSITransition,1)
};



#endif

