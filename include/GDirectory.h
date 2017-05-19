#ifndef GROOT_DIRECTORY__H
#define GROOT_DIRECTORY__H

/** \addtogroup GROOT
 *  @{
 */

#include "TDirectory.h"

class GDirectory : public TDirectory {
public:
   GDirectory() : TDirectory() {}
   GDirectory(const char* name, const char* title, Option_t* option = "", TDirectory* motherDir = 0)
      : TDirectory(name, title, option, motherDir)
   {
   }
   virtual ~GDirectory();

   virtual void Build(TFile* motherFile = 0, TDirectory* motherDir = 0);
   virtual void Append(TObject* obj, Bool_t replace = false);

   // void ResetRootDirectory();

   // private:

   /// \cond CLASSIMP
   ClassDef(GDirectory, 1);
   /// \endcond
};
/*! @} */
#endif
