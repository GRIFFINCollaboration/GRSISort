//
//  TTdrEvent.cxx.
//
//  $Id: TTdrEvent.cxx 91 2012-04-12 18:36:17Z olchansk $
//

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cassert>

#include "TTdrEvent.h"

/// \cond CLASSIMP
ClassImp(TTdrEvent)
/// \endcond

TTdrEvent::TTdrEvent()
{
   // Default constructor
   fData.resize(0);
}

void TTdrEvent::Copy(TObject& rhs) const
{
   // Copies the entire TTdrEvent.
   static_cast<TTdrEvent&>(rhs).fData = fData;
}

TTdrEvent::TTdrEvent(const TTdrEvent& rhs) : TRawEvent()
{
   // Copy ctor.
   rhs.Copy(*this);
}

TTdrEvent::~TTdrEvent()
{
   Clear();
}

TTdrEvent& TTdrEvent::operator=(const TTdrEvent& rhs)
{
   if(&rhs != this) {
      Clear();
   }

   rhs.Copy(*this);
   return *this;
}

void TTdrEvent::Clear(Option_t*)
{
   // Clears the TTdrEvent.
   fData.clear();
}

void TTdrEvent::SetHeader(const char* buffer)
{
	/// Set the TDR header from buffer (assumes that buffer contains at least 24 bytes worth of data)
	// Skip first 8 bytes (TTdrFile::Read already tested that these are "EBYEDATA")
	fHeader.fSequence     = *reinterpret_cast<const uint32_t*>(buffer + 8);
	fHeader.fStream       = *reinterpret_cast<const uint16_t*>(buffer + 12);
	fHeader.fTape         = *reinterpret_cast<const uint16_t*>(buffer + 14);
	fHeader.fHeaderEndian = *reinterpret_cast<const uint16_t*>(buffer + 16);
	fHeader.fDataEndian   = *reinterpret_cast<const uint16_t*>(buffer + 18);
	fHeader.fDataLength   = *reinterpret_cast<const uint32_t*>(buffer + 20);
	//std::cout<<std::hex<<std::setfill('0');
	//std::cout<<"sequence      0x"<<fHeader.fSequence<<std::endl;
	//std::cout<<"stream        0x"<<fHeader.fStream<<std::endl;
	//std::cout<<"tape          0x"<<fHeader.fTape<<std::endl;
	//std::cout<<"header endian 0x"<<fHeader.fHeaderEndian<<std::endl;
	//std::cout<<"data endian   0x"<<fHeader.fDataEndian<<std::endl;
	//std::cout<<"data length   0x"<<fHeader.fDataLength<<" / 0x"<<GetDataSize()<<std::endl;
	//std::cout<<std::dec<<std::setfill(' ');
}

void TTdrEvent::SetData(const std::vector<char>& buffer)
{
   // Sets the data in the TTdrEvent as the data argument passed into
   // this function.
   fData = std::vector<char>(buffer.begin(), buffer.begin() + fHeader.fDataLength);
   SwapBytes(false);
}

uint32_t TTdrEvent::GetDataSize() const
{
   return fData.size();
}

char* TTdrEvent::GetData()
{
   // returns the allocated data.
   return fData.data();
}

void TTdrEvent::Print(const char* option) const
{
   /// Print data held in this class.
   /// \param [in] option If 'a' (for "all") then the raw data will be
   /// printed out too.
   ///

   printf("Event start:\n");
   if(option[0] == 'a') {
      for(size_t i = 0; i < fData.size() / 4; ++i) {
         printf("0x%08x", ((uint32_t*)fData.data())[i]);
         if(i % 10 == 9) {
            printf("\n");
         } else {
            printf(" ");
         }
      }
   }
}

using BYTE = uint8_t;

/// Byte swapping routine.
///
#define QWORD_SWAP(x)                            \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 7); \
      *(((BYTE*)(x)) + 7) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = *(((BYTE*)(x)) + 6); \
      *(((BYTE*)(x)) + 6) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 2); \
      *(((BYTE*)(x)) + 2) = *(((BYTE*)(x)) + 5); \
      *(((BYTE*)(x)) + 5) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 3); \
      *(((BYTE*)(x)) + 3) = *(((BYTE*)(x)) + 4); \
      *(((BYTE*)(x)) + 4) = _tmp;                \
   }

/// Byte swapping routine.
///
#define DWORD_SWAP(x)                            \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 3); \
      *(((BYTE*)(x)) + 3) = _tmp;                \
      _tmp                = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = *(((BYTE*)(x)) + 2); \
      *(((BYTE*)(x)) + 2) = _tmp;                \
   }

/// Byte swapping routine.
///
#define WORD_SWAP(x)                             \
   {                                             \
      BYTE _tmp;                                 \
      _tmp                = *((BYTE*)(x));       \
      *((BYTE*)(x))       = *(((BYTE*)(x)) + 1); \
      *(((BYTE*)(x)) + 1) = _tmp;                \
   }

int TTdrEvent::SwapBytes(bool)
{
   return 1;
}

int TTdrEvent::Process(TDataParser& parser)
{
   /// Process this TTdrEvent using the provided data parser.
   /// Returns the total number of fragments read (good and bad).
   // right now the parser only returns the total number of fragments read
   // so we assume (for now) that all fragments are good fragments
   fGoodFrags = parser.TdrToFragment(fData);
	fData.clear();
   return fGoodFrags;
}

// end
