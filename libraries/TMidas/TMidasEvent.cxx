//
//  TMidasEvent.cxx.
//
//  $Id: TMidasEvent.cxx 91 2012-04-12 18:36:17Z olchansk $
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#include "TMidasEvent.h"


ClassImp(TMidasEvent)

TMidasEvent::TMidasEvent()
{
  fData = NULL;
  fAllocatedByUs = false;

  fBanksN = 0;
  fBankList = NULL;

  fEventHeader.fEventId      = 0;
  fEventHeader.fTriggerMask  = 0;
  fEventHeader.fSerialNumber = 0;
  fEventHeader.fTimeStamp    = 0;
  fEventHeader.fDataSize     = 0;
}

void TMidasEvent::Copy(const TMidasEvent& rhs)
{
  fEventHeader = rhs.fEventHeader;

  fData        = (char*)malloc(fEventHeader.fDataSize);
  assert(fData);
  memcpy(fData, rhs.fData, fEventHeader.fDataSize);
  fAllocatedByUs = true;

  fBanksN      = rhs.fBanksN;
  fBankList    = strdup(rhs.fBankList);
  assert(fBankList);
}

TMidasEvent::TMidasEvent(const TMidasEvent &rhs)
{
  Copy(rhs);
}

TMidasEvent::~TMidasEvent()
{
  Clear();
}

TMidasEvent& TMidasEvent::operator=(const TMidasEvent &rhs)
{
  if (&rhs != this)
    Clear();

  this->Copy(rhs);
  return *this;
}

void TMidasEvent::Clear()
{
  if (fBankList)
    free(fBankList);
  fBankList = NULL;

  if (fData && fAllocatedByUs)
    free(fData);
  fData = NULL;

  fAllocatedByUs = false;
  fBanksN = 0;

  fEventHeader.fEventId      = 0;
  fEventHeader.fTriggerMask  = 0;
  fEventHeader.fSerialNumber = 0;
  fEventHeader.fTimeStamp    = 0;
  fEventHeader.fDataSize     = 0;
}

void TMidasEvent::SetData(uint32_t size, char* data)
{
  fEventHeader.fDataSize = size;
  assert(!fAllocatedByUs);
  assert(IsGoodSize());
  fData = data;
  fAllocatedByUs = false;
  SwapBytes(false);
}

uint16_t TMidasEvent::GetEventId() const
{
  return fEventHeader.fEventId;
}

uint16_t TMidasEvent::GetTriggerMask() const
{
  return fEventHeader.fTriggerMask;
}

uint32_t TMidasEvent::GetSerialNumber() const
{
  return fEventHeader.fSerialNumber;
}

uint32_t TMidasEvent::GetTimeStamp() const
{
  return fEventHeader.fTimeStamp;
}

uint32_t TMidasEvent::GetDataSize() const
{
  return fEventHeader.fDataSize;
}

char* TMidasEvent::GetData()
{
  if (!fData)
    AllocateData();
  return fData;
}

TMidas_EVENT_HEADER *TMidasEvent::GetEventHeader()
{
  return &fEventHeader;
}

bool TMidasEvent::IsGoodSize() const
{
  return fEventHeader.fDataSize > 0 && fEventHeader.fDataSize <= 500 * 1024 * 1024;
}

bool TMidasEvent::IsBank32() const
{
  return ((TMidas_BANK_HEADER *)fData)->fFlags & (1<<4);
}

int TMidasEvent::LocateBank(const void *unused, const char *name, void **pdata) const
{
  /// See FindBank()

  int bktype, bklen;

  int status = FindBank(name, &bklen, &bktype, pdata);
  
  if (!status)
    {
      *pdata = NULL;
      return 0;
    }

  return bklen;
}

static const unsigned TID_SIZE[] = {0, 1, 1, 1, 2, 2, 4, 4, 4, 4, 8, 1, 0, 0, 0, 0, 0};
static const unsigned TID_MAX = (sizeof(TID_SIZE)/sizeof(TID_SIZE[0]));

int TMidasEvent::FindBank(const char* name, int *bklen, int *bktype, void **pdata) const
{
  /// Find a data bank.
  /// \param [in] name Name of the data bank to look for.
  /// \param [out] bklen Number of array elements in this bank.
  /// \param [out] bktype Bank data type (MIDAS TID_xxx).
  /// \param [out] pdata Pointer to bank data, Returns NULL if bank not found.
  /// \returns 1 if bank found, 0 otherwise.
  ///

  const TMidas_BANK_HEADER *pbkh = (const TMidas_BANK_HEADER*)fData; 
  TMidas_BANK *pbk;
  //uint32_t dname;

  if (((pbkh->fFlags & (1<<4)) > 0)) {
#if 0
    TMidas_BANK32 *pbk32;
    pbk32 = (TMidas_BANK32 *) (pbkh + 1);
    memcpy(&dname, name, 4);
    do {
      if (*((uint32_t *) pbk32->fName) == dname) {
        *pdata = pbk32 + 1;
        if (TID_SIZE[pbk32->fType & 0xFF] == 0)
          *bklen = pbk32->fDataSize;
        else
          *bklen = pbk32->fDataSize / TID_SIZE[pbk32->fType & 0xFF];

        *bktype = pbk32->fType;
        return 1;
      }
      pbk32 = (TMidas_BANK32 *) ((char*) (pbk32 + 1) +
                          (((pbk32->fDataSize)+7) & ~7));
    } while ((char*) pbk32 < (char*) pbkh + pbkh->fDataSize + sizeof(TMidas_BANK_HEADER));
#endif

    TMidas_BANK32 *pbk32 = NULL;

    while (1) {
      IterateBank32(&pbk32, (char**)pdata);
      //printf("looking for [%s] got [%s]\n", name, pbk32->fName);
      if (pbk32 == NULL)
	break;

      if (name[0]==pbk32->fName[0] &&
	  name[1]==pbk32->fName[1] &&
	  name[2]==pbk32->fName[2] &&
	  name[3]==pbk32->fName[3]) {
	
	if (TID_SIZE[pbk32->fType & 0xFF] == 0)
	  *bklen = pbk32->fDataSize;
	else
	  *bklen = pbk32->fDataSize / TID_SIZE[pbk32->fType & 0xFF];
	
	*bktype = pbk32->fType;
	return 1;
      }
    }
  } else {
    pbk = (TMidas_BANK *) (pbkh + 1);
    do {
      if (name[0]==pbk->fName[0] &&
	  name[1]==pbk->fName[1] &&
	  name[2]==pbk->fName[2] &&
	  name[3]==pbk->fName[3]) {
        *pdata = pbk + 1;
        if (TID_SIZE[pbk->fType & 0xFF] == 0)
          *bklen = pbk->fDataSize;
        else
          *bklen = pbk->fDataSize / TID_SIZE[pbk->fType & 0xFF];

        *bktype = pbk->fType;
        return 1;
      }
      pbk = (TMidas_BANK *) ((char*) (pbk + 1) + (((pbk->fDataSize)+7) & ~7));
    } while ((char*) pbk < (char*) pbkh + pbkh->fDataSize + sizeof(TMidas_BANK_HEADER));
  }
  //
  // bank not found
  //
  *pdata = NULL;
  return 0;
}

void TMidasEvent::Print(const char *option) const
{
  /// Print data held in this class.
  /// \param [in] option If 'a' (for "all") then the raw data will be
  /// printed out too.
  ///
  
  time_t t = (time_t)fEventHeader.fTimeStamp;

  printf("Event start:\n");
  printf("  event id:       0x%04x\n", fEventHeader.fEventId);
  printf("  trigger mask:   0x%04x\n", fEventHeader.fTriggerMask);
  printf("  serial number:%8d\n", fEventHeader.fSerialNumber);
  printf("  time stamp:     %d, %s", fEventHeader.fTimeStamp, ctime(&t));
  printf("  data size:    %8d\n", fEventHeader.fDataSize);
  if ((fEventHeader.fEventId & 0xffff) == 0x8000)
    {
      printf("Begin of run %d\n", fEventHeader.fSerialNumber);
    }
  else if ((fEventHeader.fEventId & 0xffff) == 0x8001)
    {
      printf("End of run %d\n", fEventHeader.fSerialNumber);
    }
  else if (fBanksN <= 0)
    {
      printf("TMidasEvent::Print: Use SetBankList() before Print() to print bank data\n");
    }
  else
    {
      printf("Banks: %s\n", fBankList);

      for (int i = 0; i < fBanksN * 4; i += 4)
	{
	  int bankLength = 0;
	  int bankType = 0;
	  void *pdata = 0;
	  int found = FindBank(&fBankList[i], &bankLength, &bankType, &pdata);
	  
	  printf("Bank %c%c%c%c, length %6d, type %2d\n",
		 fBankList[i], fBankList[i+1], fBankList[i+2], fBankList[i+3],
		 bankLength, bankType);

	  int highlight = -1;
	  if(strlen(option)>1) {
		highlight = atoi(option+1);
		//printf("highlight = %i\n",highlight);
	  }	


	  if (option[0] == 'a' && found)
	    switch (bankType)
	      {
	      case 4: // TID_WORD
		for (int j = 0; j < bankLength; j++) {
		  if(j==highlight)
			  printf(ALERTTEXT "0x%04x" RESET_COLOR "%c", ((uint16_t*)pdata)[j], (j%10==9)?'\n':' ');
		   else
			  printf("0x%04x%c", ((uint16_t*)pdata)[j], (j%10==9)?'\n':' ');
		}
		printf("\n");
		break;
	      case 6: // TID_DWORD
		for (int j = 0; j < bankLength; j++) {
		  if(j==highlight)
			  printf(ALERTTEXT "0x%08x" RESET_COLOR "%c", ((uint16_t*)pdata)[j], (j%10==9)?'\n':' ');
		   else
			  printf("0x%08x%c", ((uint16_t*)pdata)[j], (j%10==9)?'\n':' ');
		}
		printf("\n");
		break;
	      case 7: // TID_nd280 (like a DWORD?)
		for (int j = 0; j < bankLength; j++) {
		  if(j==highlight)
			  printf(ALERTTEXT "0x%08x" RESET_COLOR "%c", ((uint32_t*)pdata)[j], (j%10==9)?'\n':' ');
		   else
			  printf("0x%08x%c", ((uint32_t*)pdata)[j], (j%10==9)?'\n':' ');
		}
		printf("\n");
		break;
	      case 9: // TID_FLOAT
		for (int j = 0; j < bankLength; j++) {
		  if(j==highlight)
			  printf(ALERTTEXT "%.8g" RESET_COLOR "%c", ((float*)pdata)[j], (j%10==9)?'\n':' ');
		   else
			  printf("%.8g%c", ((float*)pdata)[j], (j%10==9)?'\n':' ');
		}
		printf("\n");
		break;
	      case 10: // TID_DOUBLE
		for (int j = 0; j < bankLength; j++) {
		  if(j==highlight)
			  printf(ALERTTEXT "%.16g" RESET_COLOR "%c", ((double*)pdata)[j], (j%10==9)?'\n':' ');
		   else
			  printf("%.16g%c", ((double*)pdata)[j], (j%10==9)?'\n':' ');
		}
		printf("\n");
		break;
	      default:
		printf("TMidasEvent::Print: Do not know how to print bank of type %d\n", bankType);
		break;
	      }
	}
    }
}

void TMidasEvent::AllocateData()
{
  assert(!fAllocatedByUs);
  assert(IsGoodSize());
  fData = (char*)malloc(fEventHeader.fDataSize);
  assert(fData);
  fAllocatedByUs = true;
}

const char* TMidasEvent::GetBankList() const
{
  return fBankList;
}

int TMidasEvent::SetBankList()
{
  if (fEventHeader.fEventId <= 0)
    return 0;

  if (fBankList)
    return fBanksN;

  int listSize = 0;

  fBanksN = 0;

  TMidas_BANK32 *pmbk32 = NULL;
  TMidas_BANK *pmbk = NULL;
  char *pdata = NULL;

  while (1)
    {
      if (fBanksN*4 >= listSize)
	{
	  listSize += 400;
	  fBankList = (char*)realloc(fBankList, listSize);
	}

      if (IsBank32())
	{
	  IterateBank32(&pmbk32, &pdata);
	  if (pmbk32 == NULL)
	    break;
	  memcpy(fBankList+fBanksN*4, pmbk32->fName, 4);
	  fBanksN++;
	}
      else
	{
	  IterateBank(&pmbk, &pdata);
	  if (pmbk == NULL)
	    break;
	  memcpy(fBankList+fBanksN*4, pmbk->fName, 4);
	  fBanksN++;
	}
    }

  fBankList[fBanksN*4] = 0;

  return fBanksN;
}

int TMidasEvent::IterateBank(TMidas_BANK **pbk, char **pdata) const
{
  /// Iterates through banks inside an event. The function can be used
  /// to enumerate all banks of an event.
  /// \param [in] pbk Pointer to the bank header, must be NULL for the
  /// first call to this function. Returns NULL if no more banks
  /// \param [in] pdata Pointer to data area of bank. Returns NULL if no more banks
  /// \returns Size of bank in bytes or 0 if no more banks.
  ///
  const TMidas_BANK_HEADER* event = (const TMidas_BANK_HEADER*)fData;

  if (*pbk == NULL)
    *pbk = (TMidas_BANK *) (event + 1);
  else
    *pbk = (TMidas_BANK *) ((char*) (*pbk + 1) + ((((*pbk)->fDataSize)+7) & ~7));

  *pdata = (char*)((*pbk) + 1);

  if ((char*) *pbk >=  (char*) event + event->fDataSize + sizeof(TMidas_BANK_HEADER))
    {
      *pbk = NULL;
      *pdata = NULL;
      return 0;
    }

  return (*pbk)->fDataSize;
}

int TMidasEvent::IterateBank32(TMidas_BANK32 **pbk, char **pdata) const
{
  /// See IterateBank()

  const TMidas_BANK_HEADER* event = (const TMidas_BANK_HEADER*)fData;
  if (*pbk == NULL)
    *pbk = (TMidas_BANK32 *) (event + 1);
  else {
    uint32_t length = (*pbk)->fDataSize;
    uint32_t length_adjusted = (length+7) & ~7;
    //printf("length %6d 0x%08x, 0x%08x\n", length, length, length_adjusted);
    *pbk = (TMidas_BANK32 *) ((char*) (*pbk + 1) + length_adjusted);
  }

  TMidas_BANK32 *bk4 = (TMidas_BANK32*)(((char*) *pbk) + 4);

  //printf("iterate bank32: pbk 0x%p, align %d, type %d %d, name [%s], next [%s], TID_MAX %d\n", *pbk, (int)( ((uint64_t)(*pbk))&7), (*pbk)->fType, bk4->fType, (*pbk)->fName, bk4->fName, TID_MAX);

  if ((*pbk)->fType > TID_MAX) // bad - unknown bank type - it's invalid MIDAS file?
    {
      if (bk4->fType <= TID_MAX) // okey, this is a malformed T2K/ND280 data file
	{
	  *pbk = bk4;

	  //printf("iterate bank32: pbk 0x%p, align %d, type %d, name [%s]\n", *pbk, (int)(((uint64_t)(*pbk))&7), (*pbk)->fType, (*pbk)->fName);
	}
      else
	{
	  // truncate invalid data
	  *pbk = NULL;
	  *pdata = NULL;
	  return 0;
	}
    }

  *pdata = (char*)((*pbk) + 1);

  if ((char*) *pbk >= (char*)event  + event->fDataSize + sizeof(TMidas_BANK_HEADER))
    {
      *pbk = NULL;
      *pdata = NULL;
      return 0;
    }

  return (*pbk)->fDataSize;
}

typedef uint8_t BYTE;

/// Byte swapping routine.
///
#define QWORD_SWAP(x) { BYTE _tmp;       \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+7);     \
*(((BYTE *)(x))+7) = _tmp;               \
_tmp= *(((BYTE *)(x))+1);                \
*(((BYTE *)(x))+1) = *(((BYTE *)(x))+6); \
*(((BYTE *)(x))+6) = _tmp;               \
_tmp= *(((BYTE *)(x))+2);                \
*(((BYTE *)(x))+2) = *(((BYTE *)(x))+5); \
*(((BYTE *)(x))+5) = _tmp;               \
_tmp= *(((BYTE *)(x))+3);                \
*(((BYTE *)(x))+3) = *(((BYTE *)(x))+4); \
*(((BYTE *)(x))+4) = _tmp; }

/// Byte swapping routine.
///
#define DWORD_SWAP(x) { BYTE _tmp;       \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+3);     \
*(((BYTE *)(x))+3) = _tmp;               \
_tmp= *(((BYTE *)(x))+1);                \
*(((BYTE *)(x))+1) = *(((BYTE *)(x))+2); \
*(((BYTE *)(x))+2) = _tmp; }

/// Byte swapping routine.
///
#define WORD_SWAP(x) { BYTE _tmp;        \
_tmp= *((BYTE *)(x));                    \
*((BYTE *)(x)) = *(((BYTE *)(x))+1);     \
*(((BYTE *)(x))+1) = _tmp; }

void TMidasEvent::SwapBytesEventHeader()
{
  WORD_SWAP(&fEventHeader.fEventId);
  WORD_SWAP(&fEventHeader.fTriggerMask);
  DWORD_SWAP(&fEventHeader.fSerialNumber);
  DWORD_SWAP(&fEventHeader.fTimeStamp);
  DWORD_SWAP(&fEventHeader.fDataSize);
}

int TMidasEvent::SwapBytes(bool force)
{
  TMidas_BANK_HEADER *pbh;
  TMidas_BANK *pbk;
  TMidas_BANK32 *pbk32;
  void *pdata;
  uint16_t type;

  pbh = (TMidas_BANK_HEADER *) fData;

  uint32_t dssw = pbh->fDataSize;

  DWORD_SWAP(&dssw);

  //printf("SwapBytes %d, flags 0x%x 0x%x\n", force, pbh->fFlags, pbh->fDataSize);
  //printf("evh.datasize: 0x%08x, SwapBytes: %d, pbh.flags: 0x%08x, pbh.datasize: 0x%08x swapped 0x%08x\n", fEventHeader.fDataSize, force, pbh->fFlags, pbh->fDataSize, dssw);

  //
  // only swap if flags in high 16-bit
  //
  if (pbh->fFlags < 0x10000 && ! force)
    return 0;

  if (pbh->fDataSize == 0x6d783f3c) // string "<xml..." in wrong-endian format
    return 1;

  if (pbh->fDataSize == 0x3c3f786d) // string "<xml..."
    return 1;

  if (dssw > fEventHeader.fDataSize + 100) // swapped data size looks wrong. do not swap.
    return 1;

  //
  // swap bank header
  //
  DWORD_SWAP(&pbh->fDataSize);
  DWORD_SWAP(&pbh->fFlags);
  //
  // check for 32-bit banks
  //
  bool b32 = IsBank32();

  pbk = (TMidas_BANK *) (pbh + 1);
  pbk32 = (TMidas_BANK32 *) pbk;
  //
  // scan event
  //
  while ((char*) pbk < (char*) pbh + pbh->fDataSize + sizeof(TMidas_BANK_HEADER)) {
    //
    // swap bank header
    //
    if (b32) {
      DWORD_SWAP(&pbk32->fType);
      DWORD_SWAP(&pbk32->fDataSize);
      pdata = pbk32 + 1;
      type = (uint16_t) pbk32->fType;
    } else {
      WORD_SWAP(&pbk->fType);
      WORD_SWAP(&pbk->fDataSize);
      pdata = pbk + 1;
      type = pbk->fType;
    }
    //
    // pbk points to next bank
    //
    if (b32) {
      assert(pbk32->fDataSize < fEventHeader.fDataSize + 100);
      pbk32 = (TMidas_BANK32 *) ((char*) (pbk32 + 1) +
                          (((pbk32->fDataSize)+7) & ~7));
      pbk = (TMidas_BANK *) pbk32;
    } else {
      assert(pbk->fDataSize < fEventHeader.fDataSize + 100);
      pbk = (TMidas_BANK *) ((char*) (pbk + 1) +  (((pbk->fDataSize)+7) & ~7));
      pbk32 = (TMidas_BANK32 *) pbk;
    }

    switch (type) {
    case 4:
    case 5:
      while (pdata < pbk) {
        WORD_SWAP(pdata);
        pdata = ((char*)pdata) + 2;
      }
      break;
    case 6:
    case 7:
    case 8:
    case 9:
      while (pdata < pbk) {
        DWORD_SWAP(pdata);
        pdata = ((char*)pdata) + 4;
      }
      break;
    case 10:
      while (pdata < pbk) {
        QWORD_SWAP(pdata);
        pdata = ((char*)pdata) + 8;
      }
      break;
    }
  }
  return 1;
}

// end
