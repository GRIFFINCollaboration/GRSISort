#include "TBadFragment.h"

TBadFragment::TBadFragment()
{
   /// Default constructor
#if ROOT_VERSION_CODE < ROOT_VERSION(6, 0, 0)
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
   int numWords = 1;
   for(numWords = 1; numWords < size; ++numWords) {
      if((data[numWords] & 0xf0000000) == 0x80000000) {
         ++numWords;   // to include this header
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

TBadFragment::TBadFragment(TFragment& fragment)
   : TFragment(fragment), fFailedWord(-2)
{
   /// Construct a bad fragment from a fragment.
   /// The data is left empty, failed word set to -2, and multiple errors set to false.
}

void TBadFragment::Clear(Option_t* opt)
{
   TFragment::Clear(opt);
   fData.clear();
   fFailedWord     = -1;
   fMultipleErrors = false;
}

void TBadFragment::Print(Option_t*) const
{
   /// Print out all fields of the fragment using TFragment::Print() and then print the raw data with the failed words
   /// highlighted/
   TFragment::Print();

   std::cout << "Raw data with " << (fMultipleErrors ? "multiple errors" : "single error") << " failed on word " << fFailedWord << ":" << std::endl;
   size_t index;
   for(index = 0; index < fData.size(); ++index) {
      if(index == static_cast<size_t>(fFailedWord)) {
         std::cout << ALERTTEXT;
      }
      std::cout << hex(fData[index], 8);
      if(index == static_cast<size_t>(fFailedWord)) {
         std::cout << RESET_COLOR;
      }
      if(index % 10 == 9) {
         std::cout << std::endl;
      } else {
         std::cout << " ";
      }
   }
   if(index % 10 != 0) {
      std::cout << std::endl;   // add newline if the last data word didn't have one
   }
}
