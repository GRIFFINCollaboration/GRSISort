#ifndef GChain__H
#define GChain__H

/** \addtogroup GROOT
 *  @{
 */

#include "TChain.h"

class GChain : public TChain {
public:
   GChain();
   GChain(const char* name, const char* title = "");
   ~GChain() override;

private:
   void InitGChain();

   /// \cond CLASSIMP
   ClassDefOverride(GChain, 1)
   /// \endcond
};
/*! @} */
#endif
