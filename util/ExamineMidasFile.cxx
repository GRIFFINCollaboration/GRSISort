
#include <Globals.h>

#include <cstdio>
#include <map>
#include <fstream>

#include <TStopwatch.h>
#include <TMidasFile.h>
#include <TMidasEvent.h>

void ExamineMidasFile(const char *filename) {
	std::ifstream in(filename, std::ifstream::in | std::ifstream::binary);
	if(!in.is_open()) {
		printf("unable to open file %s\n",filename);
		return;
	}   
	in.seekg(0, std::ifstream::end);
	long long filesize = in.tellg();
	in.close();

	TMidasFile  mfile; 
	std::shared_ptr<TMidasEvent> mevent = std::make_shared<TMidasEvent>();
	std::map<int,int> type_counter;

	mfile.Open(filename);

	long long starttime    = 0;
	long long stoptime     = 0;
	int currenteventnumber = 0;
	long long bytesread = 0;

	bool loop = true;

	TStopwatch sw;
	sw.Start();
	if(mfile.GetFirstEvent() != nullptr) {
		printf( DGREEN );
		mfile.GetFirstEvent()->Print();
		printf( RESET_COLOR );
		starttime = mfile.GetFirstEvent()->GetTimeStamp();
	} else {
		printf("Failed to find any midas event!\n");
		return;
	}
	while(loop) {
		bytesread += mfile.Read(mevent);
		switch(mevent->GetEventId()) {
			case 0x8001: //run stop
				printf( "                                                                             \r");
				printf( DRED );
				mevent->Print();
				printf( RESET_COLOR );
				stoptime = mevent->GetTimeStamp();
				loop = false;
				break;
			default:
				type_counter[mevent->GetEventId()]++;
				break;
		}
		if((++currenteventnumber%15000)== 0) {
			printf( " Looping over event %i have looped %.2fMB/%.2f MB => %.1f MB/s              "  "\r",
					currenteventnumber,(bytesread/1000000.0),(filesize/1000000.0),(bytesread/1000000.0)/sw.RealTime());
			sw.Continue();
		}
	}
	printf( " Looping over event %i have looped %.2fMB/%.2f MB => %.1f MB/s              "  "\r",
			currenteventnumber,(bytesread/1000000.0),(filesize/1000000.0),(bytesread/1000000.0)/sw.RealTime());
	printf("\n\n");
	printf("EventTypes Seen: \n");
	for(std::map<int,int>::iterator it=type_counter.begin();it!=type_counter.end();it++) {
		printf("\tEventId[0x%x]  =  %i\n",it->first,it->second);
	}
	printf("\n");
	printf("Run length =  %lli  seconds\n",stoptime-starttime);
	printf("\n");
	return;
}


#ifndef __CINT__

void PrintUsage() {
	printf("Usage:  ./ExamineMidasFile <runXXXXX.mid>  \n");
	printf("Can take multiple midas files. Currently no other options.\n");
}

int main(int argc, char** argv) {
	if(argc<2) {
		PrintUsage();
		return 1;
	}

	for(int x=1;x<argc;x++) {
		ExamineMidasFile(argv[x]);
	}

	return 0;
}

#endif


