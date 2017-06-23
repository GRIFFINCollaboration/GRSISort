#ifndef TGINT_H
#define TGINT_H

/** \addtogroup GROOT
 *  @{
 */

#include "TRint.h"

class TGint : public TRint {
private:
   TGint(int argc, char** argv, void* options = nullptr, int numOptions = 0, bool noLogo = false,
         const char* appClassName = "grsisort");

public:
   static TGint* fTGint;
   static TGint* instance(int argc = 0, char** argv = nullptr, void* options = nullptr, int numOptions = 0, bool noLogo = false,
                          const char* appClassName = "grsisort");

   ~TGint() override;

   void PrintLogo(bool) override;
   bool HandleTermInput() override;
   int  TabCompletionHook(char*, int*, ostream&);

   /// \cond CLASSIMP
   ClassDefOverride(TGint, 0);
   /// \endcond
};
/*! @} */
#endif
