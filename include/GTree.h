#ifndef GTREE__H
#define GTREE__H

/** \addtogroup GROOT
 *  @{
 */

#include "TTree.h"

class GTree : public TTree {
public:
   GTree();
   GTree(const char* name, const char* title, Int_t splitlevel = 99);
   ~GTree() override;

private:
   void InitGTree();

   /// \cond CLASSIMP
   ClassDefOverride(GTree, 1)
   /// \endcond
};
/*! @} */
#endif
