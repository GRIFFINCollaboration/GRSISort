#ifndef TTRANSIENTBITS_H
#define TTRANSIENTBITS_H

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
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <algorithm>

#include "Rtypes.h"

template <typename T>
class TTransientBits {
public:
   TTransientBits() : fBits(0) {}
   explicit TTransientBits(const T& tmp) : fBits(tmp) {}
   ~TTransientBits() = default;

   TTransientBits(const TTransientBits&)     = default;
   TTransientBits(TTransientBits&&) noexcept = default;

   TTransientBits& operator=(const TTransientBits&)     = default;
   TTransientBits& operator=(TTransientBits&&) noexcept = default;

   void   SetBit(T bit, Bool_t flag) { flag ? SetBit(bit) : ClearBit(bit); }
   void   SetBit(T bit) { fBits |= bit; }
   void   ClearBit(T bit) { fBits &= ~bit; }
   Bool_t TestBit(T bit) const { return fBits & bit; }
   T      TestBits(T bit) const { return (fBits & bit); }
   template <typename U>
   void SetBit(U bit, Bool_t flag) { flag ? SetBit(bit) : ClearBit(bit); }
   template <typename U>
   void SetBit(U bit) { fBits |= static_cast<typename std::underlying_type<U>::type>(bit); }
   template <typename U>
   void ClearBit(U bit) { fBits &= ~static_cast<typename std::underlying_type<U>::type>(bit); }
   template <typename U>
   Bool_t TestBit(U bit) const { return fBits & static_cast<typename std::underlying_type<U>::type>(bit); }
   template <typename U>
   T TestBits(U bit) const { return (fBits & static_cast<typename std::underlying_type<U>::type>(bit)); }

   TTransientBits& operator=(const T& rhs)
   {
      fBits = rhs;
      return *this;
   }
   T Value() const { return fBits; }

   void Clear() { fBits = 0; }
   void Print() const { std::cout << fBits << std::endl; }

   T fBits;
};

/*! @} */
#endif /* _TTRANSIENTBITS_H_ */
