//
// TMidasStructs.h
//

#ifndef INCLUDE_TMidasBanksH
#define INCLUDE_TMidasBanksH

/** \addtogroup Sorting
 *  @{
 */

// This file defines the data structures written
// into MIDAS .mid files. They define the on-disk
// data format, they cannot be arbitrarily changed.

/// Event header

struct TMidas_EVENT_HEADER {
   uint16_t fEventId;      ///< event id
   uint16_t fTriggerMask;  ///< event trigger mask
   uint32_t fSerialNumber; ///< event serial number
   uint32_t fTimeStamp;    ///< event timestamp in seconds
   uint32_t fDataSize;     ///< event size in bytes
};

/// Bank header

struct TMidas_BANK_HEADER {
   uint32_t fDataSize;
   uint32_t fFlags;
};

/// 16-bit data bank

struct TMidas_BANK {
   char     fName[4]; ///< bank name
   uint16_t fType;    ///< type of data (see midas.h TID_xxx)
   uint16_t fDataSize;
};

/// 32-bit data bank

struct TMidas_BANK32 {
   char     fName[4]; ///< bank name
   uint32_t fType;    ///< type of data (see midas.h TID_xxx)
   uint32_t fDataSize;
};
/*! @} */
#endif
