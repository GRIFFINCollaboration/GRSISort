#include "TBadFragment.h"

ClassImp(TBadFragment)

TBadFragment::TBadFragment() : TFragment()
{
	/// Default constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
   Class()->IgnoreTObjectStreamer(kTRUE);
#endif
   Clear();
}

TBadFragment::TBadFragment(TFragment& fragment, uint32_t* data, int size, int failedWord, bool multipleErrors)
   : TFragment(fragment)
{
   /// Construct a bad fragment from a fragment, the data it was created from, the size of that data, and the word the
   /// parser failed on.
   /// The data is only copied up to and including the next header word (high nibble 0x8).
	
   // skipping the first word, we search for the next header
   int numWords;
   for(numWords = 1; numWords < size; ++numWords) {
      if((data[numWords] & 0xf0000000) == 0x80000000) {
			++numWords; // to include this header
         break;
      }
   }
   // only copy data up to the next header (including that header)
	// if the above loop ended w/o finding a header numWords == size,
	// i.e. we copy all the data
   fData.insert(fData.begin(), data, data + numWords);
   fFailedWord     = failedWord;
   fMultipleErrors = multipleErrors;
}

TBadFragment::TBadFragment(TFragment& fragment) : TFragment(fragment)
{
   /// Construct a bad fragment from a fragment.
   /// The data is left empty, failed word set to -2, and multiple errors set to false.
	
   fFailedWord     = -2;
   fMultipleErrors = false;
}

TBadFragment::TBadFragment(const TBadFragment& rhs) : TFragment(rhs)
{
   /// Copy constructor
   fData       = rhs.fData;
   fFailedWord = rhs.fFailedWord;
   fMultipleErrors = rhs.fMultipleErrors;
}

TBadFragment::~TBadFragment()
{
   /// Destructor, does nothing for now.
}

TBadFragment& TBadFragment::operator=(const TBadFragment& rhs)
{
	/// Assignment operator
	TFragment::operator=(rhs);
   fData       = rhs.fData;
   fFailedWord = rhs.fFailedWord;
   fMultipleErrors = rhs.fMultipleErrors;

	return *this;
}

void TBadFragment::Clear(Option_t* opt)
{
	TFragment::Clear(opt);
	fData.clear();
	fFailedWord = -1;
	fMultipleErrors = false;
}

void TBadFragment::Print(Option_t*) const
{
   /// Print out all fields of the fragment using TFragment::Print() and then print the raw data with the failed words
   /// highlighted/
   TFragment::Print();

   std::cout<<"Raw data with "<<(fMultipleErrors ? "multiple errors" : "single error")<<" failed on word "<<fFailedWord<<":"<<std::endl;
   size_t i;
   for(i = 0; i < fData.size(); ++i) {
      if(i == static_cast<size_t>(fFailedWord)) {
         std::cout<<ALERTTEXT;
      }
      std::cout<<"0x"<<std::setw(8)<<std::setfill('0')<<std::hex<<fData[i]<<std::dec<<std::setfill(' ');
      if(i == static_cast<size_t>(fFailedWord)) {
         std::cout<<RESET_COLOR;
      }
      if(i % 10 == 9) {
         std::cout<<std::endl;
      } else {
         std::cout<<" ";
      }
   }
   if(i % 10 != 0) {
      std::cout<<std::endl; // add newline if the last data word didn't have one
   }
}
