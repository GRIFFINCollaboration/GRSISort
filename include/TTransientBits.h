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

   void   SetBit(T f, Bool_t flag) { flag ? SetBit(f) : ClearBit(f); }
   void   SetBit(T f) { fBits |= f; }
   void   ClearBit(T f) { fBits &= ~f; }
   Bool_t TestBit(T f) const { return fBits & f; }
   T      TestBits(T f) const { return (fBits & f); }
   template <typename U>
   void SetBit(U f, Bool_t flag) { flag ? SetBit(f) : ClearBit(f); }
   template <typename U>
   void SetBit(U f) { fBits |= static_cast<typename std::underlying_type<U>::type>(f); }
   template <typename U>
   void ClearBit(U f) { fBits &= ~static_cast<typename std::underlying_type<U>::type>(f); }
   template <typename U>
   Bool_t TestBit(U f) const { return fBits & static_cast<typename std::underlying_type<U>::type>(f); }
   template <typename U>
   T TestBits(U f) const { return (fBits & static_cast<typename std::underlying_type<U>::type>(f)); }

   TTransientBits& operator=(const T& rhs)
   {
      fBits = rhs;
      return *this;
   }
   T Value() const { return fBits; }

   void Clear() { fBits = 0; }
   void Print() const { std::count<<fBits<<std::endl; }

   T fBits;

   //	ClassDefT(TTransientBits<T>,0);
};

/*! @} */
#endif /* _TTRANSIENTBITS_H_ */
