#ifndef TPRESERVEGDIRECTORY_H
#define TPRESERVEGDIRECTORY_H

#include "TDirectory.h"

class TPreserveGDirectory {
public:
   TPreserveGDirectory() = default;
   ~TPreserveGDirectory() { bak->cd(); }

   TPreserveGDirectory(const TPreserveGDirectory&)            = delete;
   TPreserveGDirectory(TPreserveGDirectory&&)                 = delete;
   TPreserveGDirectory& operator=(const TPreserveGDirectory&) = delete;
   TPreserveGDirectory& operator=(TPreserveGDirectory&&)      = delete;

private:
   TDirectory* bak{gDirectory};
};

#endif /* _TPRESERVEGDIRECTORY_H_ */
