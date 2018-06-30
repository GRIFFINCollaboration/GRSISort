#ifndef TTDREVENT_H
#define TTDREVENT_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"

#include "TRawEvent.h"

/////////////////////////////////////////////////////////////////
///
/// \class TTdrEvent
///
/// C++ class representing one tdr file.
///
/////////////////////////////////////////////////////////////////

/// TDR event

struct TTdrEventHeader {
   uint32_t fSequence;     ///< within the file
   uint16_t fStream;       ///< data acquisition stream number (1-4)
   uint16_t fTape;         ///< =1
   uint16_t fHeaderEndian; ///< written as native 1 by tape server
   uint16_t fDataEndian;   ///< written as native 1 in the hardware structure of the data following
	uint32_t fDataLength;   ///< length of data following in bytes
};

class TTdrEvent : public TRawEvent {
public:
   // houskeeping functions
   TTdrEvent();                                        ///< default constructor
   TTdrEvent(const TTdrEvent&);                        ///< copy constructor
   ~TTdrEvent() override;                              ///< destructor
   TTdrEvent& operator=(const TTdrEvent&);             ///< assignement operator
   void Clear(Option_t* opt = "") override;            ///< clear event for reuse
   void Copy(TObject&) const override;                 ///< copy helper
   void Print(const char* option = "") const override; ///< show all event information

   // get event information

   uint32_t GetDataSize() const override; ///< return the event size

   // helpers for event creation

   TTdrEventHeader& GetHeader() { return fHeader; } ///< return pointer to the data buffer
   char* GetData() override; ///< return pointer to the data buffer

   void SetHeader(const char* buffer); ///< set an externally allocated header buffer
   void SetData(const std::vector<char>& buffer); ///< set an externally allocated data buffer

   int SwapBytes(bool) override; ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC)

   int Process(TDataParser& parser) override;

protected:
	TTdrEventHeader   fHeader; ///< event header
   std::vector<char> fData;   ///< event data buffer

   /// \cond CLASSIMP
   ClassDefOverride(TTdrEvent, 0) // All of the data contained in a Midas Event
   /// \endcond
};
/*! @} */
#endif // TTdrEvent.h
