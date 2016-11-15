#ifndef _TPRESERVEGDIRECTORY_H_
#define _TPRESERVEGDIRECTORY_H_

#include "TDirectory.h"

class TPreserveGDirectory{
public:
  TPreserveGDirectory() : bak(gDirectory) { }
  ~TPreserveGDirectory() { bak->cd(); }

private:
  TDirectory* bak;
};

#endif /* _TPRESERVEGDIRECTORY_H_ */
