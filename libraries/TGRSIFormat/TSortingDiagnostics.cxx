#include "TSortingDiagnostics.h"

#include <fstream>

#include "TChannel.h"

TSortingDiagnostics* TSortingDiagnostics::fSortingDiagnostics = NULL;

TSortingDiagnostics::TSortingDiagnostics() : TObject() {
	Clear();
}

TSortingDiagnostics::TSortingDiagnostics(const TSortingDiagnostics& rhs) : TObject() {
	Clear();
}

TSortingDiagnostics::~TSortingDiagnostics() {
}

void TSortingDiagnostics::Copy(TObject& obj) const {
	static_cast<TSortingDiagnostics&>(obj).fFragmentsOutOfOrder = fFragmentsOutOfOrder;
}

void TSortingDiagnostics::Clear(Option_t* opt) {
	fFragmentsOutOfOrder.clear();
}

void TSortingDiagnostics::Print(Option_t* opt) const {
	if(fFragmentsOutOfOrder.size() > 0) {
		std::cout<<NumberOfFragmentsOutOfOrder()<<" fragments out of order!"<<std::endl;
	} else {
		std::cout<<"No fragments out of order!"<<std::endl;
	}
}

void TSortingDiagnostics::Draw(Option_t* opt) {
}

void TSortingDiagnostics::WriteToFile(const char* fileName) const {
	std::ofstream statsOut(fileName);
	statsOut<<std::endl
			  <<"Number of fragments out of order = "<<NumberOfFragmentsOutOfOrder()<<std::endl
			  <<std::endl;
}
