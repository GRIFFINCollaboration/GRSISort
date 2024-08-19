#ifndef TRAWEVENT_H
#define TRAWEVENT_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"
#include "TDataParser.h"

#include "TObject.h"

/////////////////////////////////////////////////////////////////
///
/// \class TRawEvent
///
/// C++ class representing one raw event.
///
/////////////////////////////////////////////////////////////////

/// RAW event

class TRawEvent : public TObject {
public:
   // houskeeping functions
   TRawEvent() = default;                              ///< default constructor
   TRawEvent(const TRawEvent& rhs) : TObject(rhs) {}   ///< copy constructor
   TRawEvent(TRawEvent&&) noexcept            = default;
   TRawEvent& operator=(const TRawEvent&)     = default;
   TRawEvent& operator=(TRawEvent&&) noexcept = default;
   ~TRawEvent()                               = default;                                              ///< destructor
   void Clear(Option_t* = "") override { fGoodFrags = 0; }                                            ///< clear event for reuse
   void Copy(TObject& obj) const override { static_cast<TRawEvent&>(obj).fGoodFrags = fGoodFrags; }   ///< copy helper
   void Print(const char* = "") const override {}                                                     ///< show all event information

   // get event information

   virtual uint32_t GetTimeStamp() const { return 0; }   ///< return the event size
   virtual uint32_t GetDataSize() const { return 0; }    ///< return the event size

   // helpers for event creation

   virtual char* GetData() { return nullptr; }   ///< return pointer to the data buffer

   virtual int SwapBytes(bool) { return 0; }   ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC)

   virtual int  GoodFrags() { return fGoodFrags; }       ///< returns number of good fragments parsed
   virtual void IncrementGoodFrags() { ++fGoodFrags; }   ///< increments the number of good fragments parsed

private:
   int fGoodFrags{0};   ///< number of good fragments parsed
   /// \cond CLASSIMP
   ClassDefOverride(TRawEvent, 0)   // NOLINT(readability-else-after-return)
   /// \endcond
};
/*! @} */
#endif   // TRawEvent.h
