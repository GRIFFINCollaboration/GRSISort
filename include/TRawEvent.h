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
   TRawEvent() {}                                                   ///< default constructor
   TRawEvent(const TRawEvent& rhs) : TObject(rhs) {}                ///< copy constructor
   ~TRawEvent() override = default;                                          ///< destructor
   virtual TRawEvent& operator=(const TRawEvent&) { return *this; } ///< assignement operator
   void Clear(Option_t* = "") override {}                            ///< clear event for reuse
   void Copy(TObject&) const override {}                             ///< copy helper
   void Print(const char* = "") const override {}                    ///< show all event information

   // get event information

   virtual uint32_t GetDataSize() const { return 0; } ///< return the event size

   // helpers for event creation

   virtual char* GetData() { return nullptr; } ///< return pointer to the data buffer

   virtual int SwapBytes(bool) { return 0; } ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC)
   virtual int Process(TDataParser& parser) = 0;

	virtual int GoodFrags() { return fGoodFrags; } ///< returns number of good fragments parsed
	
protected:
	int                 fGoodFrags{};     ///< number of good fragments parsed
   /// \cond CLASSIMP
   ClassDefOverride(TRawEvent, 0) // All of the data contained in a Midas Event
   /// \endcond
};
/*! @} */
#endif // TRawEvent.h
