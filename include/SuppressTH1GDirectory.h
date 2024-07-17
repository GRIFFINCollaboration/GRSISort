#ifndef SUPPRESSTH1GDIRECTORY_H
#define SUPPRESSTH1GDIRECTORY_H

#include "TH1.h"

// Little helper class for temporarily preventing TH1* from being added to gDirectory.
// On destruction, returns auto-add status to same state as it was before.
// Not really thread-safe.  At all.  Hopefully better than the alternative.
class SuppressTH1GDirectory {
public:
   SuppressTH1GDirectory() : prev_status(TH1::AddDirectoryStatus()) { TH1::AddDirectory(false); }
	SuppressTH1GDirectory(const SuppressTH1GDirectory&) = delete;
	SuppressTH1GDirectory(SuppressTH1GDirectory&&) = delete;
	SuppressTH1GDirectory& operator=(const SuppressTH1GDirectory&) = delete;
	SuppressTH1GDirectory& operator=(SuppressTH1GDirectory&&) = delete;

   ~SuppressTH1GDirectory() { TH1::AddDirectory(prev_status); }

private:
   bool prev_status;
};

#endif
