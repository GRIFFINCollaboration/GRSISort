#ifndef TLSTEVENT_H
#define TLSTEVENT_H

/** \addtogroup Sorting
 *  @{
 */

#include "Globals.h"

#include "TRawEvent.h"

/////////////////////////////////////////////////////////////////
///
/// \class TLstEvent
///
/// C++ class representing one lst file.
///
/////////////////////////////////////////////////////////////////

/// LST event

class TLstEvent : public TRawEvent {
	public:
		// houskeeping functions
		TLstEvent(); ///< default constructor
		TLstEvent(const TLstEvent &); ///< copy constructor
		virtual ~TLstEvent(); ///< destructor
		TLstEvent& operator=(const TLstEvent &); ///< assignement operator
		void Clear(Option_t* opt = ""); ///< clear event for reuse
		void Copy(TObject &) const; ///< copy helper
		void Print(const char* option = "") const; ///< show all event information

		// get event information

		uint32_t GetDataSize() const; ///< return the event size

		// helpers for event creation

		char* GetData(); ///< return pointer to the data buffer

		void AllocateData(); ///< allocate data buffer using the existing event header
		void SetData(std::vector<char>& dataBuffer); ///< set an externally allocated data buffer

		int  SwapBytes(bool); ///< convert event data between little-endian (Linux-x86) and big endian (MacOS-PPC) 

		int Process(TDataParser& parser);

	protected:
		std::vector<char> fData;     ///< event data buffer

		/// \cond CLASSIMP
		ClassDef(TLstEvent,0) //All of the data contained in a Midas Event
		/// \endcond
};
/*! @} */
#endif // TMidasEvent.h
