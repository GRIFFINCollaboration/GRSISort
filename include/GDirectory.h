#ifndef GROOT_DIRECTORY__H
#define GROOT_DIRECTORY__H

/** \addtogroup GROOT
 *  @{
 */

#include "TDirectory.h"

class GDirectory : public TDirectory {
public:
   GDirectory() : TDirectory() {}
   GDirectory(const char* name, const char* title, Option_t* option = "", TDirectory* motherDir = nullptr)
      : TDirectory(name, title, option, motherDir)
   {
   }
   ~GDirectory() override;

   void Build(TFile* motherFile = nullptr, TDirectory* motherDir = nullptr) override;
   void Append(TObject* obj, Bool_t replace = false) override;

   // void ResetRootDirectory();

   // private:

   /// \cond CLASSIMP
   ClassDefOverride(GDirectory, 1);
   /// \endcond
};
/*! @} */
#endif
