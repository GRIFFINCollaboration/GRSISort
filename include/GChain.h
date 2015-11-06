#ifndef GChain__H
#define GChain__H

/** \addtogroup GROOT
 *  @{
 */

#include "TChain.h"

class GChain : public TChain { 
   public:
      GChain();
      GChain(const char* name, const char* title="");
      virtual ~GChain();

   private:
      void InitGChain();

/// \cond CLASSIMP
      ClassDef(GChain,1)
/// \endcond
};
/*! @} */
#endif
