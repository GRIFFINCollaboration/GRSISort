#ifndef __GFILE_H__
#define __GFILE_H__

/** \addtogroup GROOT
 *  @{
 */

#include "TFile.h"

class GFile : public TFile {
   public:
      GFile();
      GFile(const char *name,Option_t *opt="",const char *ftitle="",Int_t compress=1);
      virtual ~GFile();

   private:

      void GFileInit();

/// \cond CLASSIMP
   ClassDef(GFile,1)
/// \endcond
};
/*! @} */
#endif
