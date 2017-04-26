#include "TBadFragment.h"

TBadFragment::TBadFragment() : TFragment() {
	/// Default constructor
#if MAJOR_ROOT_VERSION < 6
	Class()->IgnoreTObjectStreamer(kTRUE);
#endif
	Clear();
}

TBadFragment::TBadFragment(TFragment& fragment, uint32_t* data, int size, int failedWord, bool multipleErrors)
	: TFragment(fragment) {
	/// Construct a bad fragment from a fragment, the data it was created from, the size of that data, and the word the parser failed on.
	/// The data is only copied up to and including the next header word (high nibble 0x8).
	fData.clear();
	// skipping the first word, we search for the next header
	int numWords;
	for(numWords = 1; numWords < size; ++numWords) {
		if((data[numWords] & 0xf0000000) == 0x80000000) break;
	}
	// only copy data up to the next header (including that header)
	fData.insert(fData.begin(), data, data+numWords+1);
	fFailedWord = failedWord;
	fMultipleErrors = multipleErrors;
}

TBadFragment::TBadFragment(const TBadFragment& rhs) : TFragment(rhs) {
	/// Copy constructor
	fData = rhs.fData;
	fFailedWord = rhs.fFailedWord;
}

TBadFragment::~TBadFragment() {
	/// Destructor, does nothing for now.
}

void TBadFragment::Print(Option_t*) const {
	/// Print out all fields of the fragment using TFragment::Print() and then print the raw data with the failed words highlighted/
	TFragment::Print();

	std::cout<<"Raw data with "<<(fMultipleErrors ? "multiple errors":"single error")<<":"<<std::endl;
	size_t i;
	for(i = 0; i < fData.size(); ++i) {
		if(i == static_cast<size_t>(fFailedWord)) std::cout<<ALERTTEXT;
		std::cout<<"0x"<<std::setw(8)<<std::setfill('0')<<std::hex<<fData[i]<<std::dec;
		if(i == static_cast<size_t>(fFailedWord)) std::cout<<RESET_COLOR;
		if(i%10 == 9) std::cout<<std::endl;
		else          std::cout<<" ";
	}
	if(i%10 != 0) std::cout<<std::endl; // add newline if the last data word didn't have one
}
