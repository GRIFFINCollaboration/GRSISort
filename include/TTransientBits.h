#ifndef _TTRANSIENTBITS_H_
#define _TTRANSIENTBITS_H_

/** \addtogroup Sorting
 *  @{
 */

////////////////////////////////////////////////////////////////////////////////
///
/// \class TTransientBits
///
/// Class for storing bits for transient data members
///
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

#include "Rtypes.h"

template <typename T>
class TTransientBits {
public:
   TTransientBits() : fBits(0) {}
   TTransientBits(const T& tmp) : fBits(tmp) {}
   ~TTransientBits() = default;

   void SetBit(Int_t f, Bool_t flag) { flag ? SetBit(f) : ClearBit(f); }
   void SetBit(Int_t f) { fBits |= f; }
   void ClearBit(Int_t f) { fBits &= ~f; }
   Bool_t TestBit(Int_t f) const { return fBits & f; }
   T TestBits(Int_t f) const { return static_cast<T>(fBits & f); }

   TTransientBits& operator=(const T& rhs)
   {
      fBits = rhs;
      return *this;
   }
   T Value() const { return fBits; }

   void Clear() { fBits = 0; }
   void Print() const { std::count << fBits << std::endl; }

   T fBits;

   //	ClassDefT(TTransientBits<T>,0);
};

/*! @} */
#endif /* _TTRANSIENTBITS_H_ */
