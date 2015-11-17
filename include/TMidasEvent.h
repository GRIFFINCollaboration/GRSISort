#ifndef TMIDASEVENT_H
#define TMIDASEVENT_H

#include "Globals.h"
#include "TMidasEventHeader.h"

#include "TObject.h"

/////////////////////////////////////////////////////////////////
///                                                            //
/// \class TMidasEvent                                         //
///                                                            //
/// Contains the information within a Midas Event. This        //
/// usually includes a header, midas information such as timestamp
/// bank id, etc. And the bank data. The bank data is usually
/// the information supplied by either a scaler or the 
/// experimental DAQ system.
///                                                            //
/////////////////////////////////////////////////////////////////


///
/// C++ class representing one midas event.
///
/// Objects of this class are created by reading
/// midas events from a file, by reading
/// them from a midas shared memory buffer or by
/// receiving them through the mserver
///

/// MIDAS event

class TMidasEvent : public TObject {
 public:
	#include "TMidasBanks.h"

 public:

  // houskeeping functions
  TMidasEvent(); ///< default constructor
  TMidasEvent(const TMidasEvent &); ///< copy constructor
  virtual ~TMidasEvent(); ///< destructor
  TMidasEvent& operator=(const TMidasEvent &); ///< assignement operator
  void Clear(Option_t* opt = ""); ///< clear event for reuse
  void Copy(TObject &) const; ///< copy helper
  void Print(const char* option = "") const; ///< show all event information

  // get event information

  uint16_t GetEventId() const;      ///< return the event id
  uint16_t GetTriggerMask() const;     ///< return the triger mask
  uint32_t GetSerialNumber() const; ///< return the serial number
  uint32_t GetTimeStamp() const; ///< return the time stamp (unix time in seconds)
  uint32_t GetDataSize() const; ///< return the event size

  // get data banks

  const char* GetBankList() const; ///< return a list of data banks
  int FindBank(const char* bankName, int* bankLength, int* bankType, void** bankPtr) const;
  int LocateBank(const void* unused, const char* bankName, void** bankPtr) const;

  bool IsBank32() const; ///< returns "true" if event uses 32-bit banks
  int IterateBank(TMidas_BANK** , char** pdata) const; ///< iterate through 16-bit data banks
  int IterateBank32(TMidas_BANK32** , char** pdata) const; ///< iterate through 32-bit data banks

  // helpers for event creation

  TMidas_EVENT_HEADER* GetEventHeader(); ///< return pointer to the event header
  char* GetData(); ///< return pointer to the data buffer

  void AllocateData(); ///< allocate data buffer using the existing event header
  void SetData(uint32_t dataSize, char* dataBuffer); ///< set an externally allocated data buffer

  int SetBankList(); ///< create the list of data banks, return number of banks
  bool IsGoodSize() const; ///< validate the event length

  void SwapBytesEventHeader(); ///< convert event header between little-endian (Linux-x86) and big endian (MacOS-PPC) 
  int  SwapBytes(bool); ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC) 

protected:

   TMidas_EVENT_HEADER fEventHeader; ///< event header
   char* fData;     ///< event data buffer
   int  fBanksN;    ///< number of banks in this event
   char* fBankList; ///< list of bank names in this event
   bool fAllocatedByUs; ///< "true" if we own the data buffer
	
/// \cond CLASSIMP
	ClassDef(TMidasEvent,0) //All of the data contained in a Midas Event
/// \endcond
};

#endif // TMidasEvent.h
