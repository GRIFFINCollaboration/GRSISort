#ifndef TGRSITRANSITION_H
#define TGRSITRANSITION_H

#include <cstdio>

#include <TClass.h>
#include <TObject.h>

class TGRSITransition : public TObject {
   friend class TNucleus;
   public:
      TGRSITransition();
      ~TGRSITransition();

      bool IsSortable() const { return true; }
      int Compare(const TObject *obj) const;

   protected:
      double energy;
      double energy_uncertainity;
      double intensity;
      double intensity_uncertainity;

   ClassDef(TGRSITransition,1)
};

#endif

