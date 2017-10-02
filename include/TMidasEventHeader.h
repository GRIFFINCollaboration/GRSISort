//
// TMidasStructs.h
//

#ifndef INCLUDE_TMidasEventHeader_H
#define INCLUDE_TMidasEventHeader_H

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
/*! @} */
#endif
// end
