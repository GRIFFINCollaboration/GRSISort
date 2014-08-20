//g++ Root2Rad.cxx `root-config --cflags --libs` -oRoot2Rad
//
//    Author:  P.C. Bender, <pcbend@gmail.com>
//    * 
//    *  * Please indicate changes with your initials.
//    *
//
//    Short and sweet program. Finds all the 1d (and 2d) histograms in a tfile by
//    itterating over the keys, than turn each of them into a fortran formatted 
//    .spei/.mat file used by radware.  
//
//    If histograms are larger than 16384 bins, it trys to be smart to decide 
//    whether to truncate or compress the hist to fit the 16384 radwar limit.
//
//    Matrices are all truncated to 4096 or padded to 4096 if there are to few 
//    bins.
//
//
//to compile:
//   g++ Root2Rad.cxx -oRoot2Rad `root-config --cflags --libs`
//
//ro run:
//   ./Root2Rad rootfile.root
//



#include<stdint.h>
#include<fstream>
#include<iostream>

#include<string.h>

#include<TFile.h>
#include<TH1.h>
#include<TH2.h>
#include<TSystem.h>
#include<TList.h>
#include<TClass.h>
#include<TKey.h>
#include<TTimeStamp.h>

#include <sys/stat.h>
#include <sys/types.h>


struct SpeHeader {
  int32_t buffsize;          /*fortran file, each record starts with record size */            // 14
  char label[8]; 
  int32_t size;
  int32_t junk1;
  int32_t junk2;
  int32_t junk3;
  int32_t buffcheck;         /*fortran file, record ends with record size :) */                // 14
} __attribute__((packed));


//the above would be followed by an integer of bin size * 4
//the value of all bins in int sizes
//an integer os bin size * 4           ------- number of char in the histogram.

void WriteHist(TH1*,fstream*);
void WriteMat(TH2*,fstream*);

int main(int argc, char** argv)	{	


	TFile *infile = new TFile();	
	if(argc < 2  || !(infile = TFile::Open(argv[1],"read")) )	{
		printf ( "problem opening file.\nUsage: Root2Rad file.root\n");
		return 1;
	}

   std::string path = infile->GetName();
   path.erase(path.find_last_of('.'));

   struct stat st = {0};

   if(stat(path.c_str(),&st)==-1) {
      mkdir(path.c_str(),0755);
   }


	//std::string outfilename = infile->GetName();
	//outfilename.erase(outfilename.find_last_of('.'));
	//outfilename.append(".spe");
	//fstream outfile;
	//outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
	
	TList *keys = infile->GetListOfKeys();
	keys->Sort();
	TIter next(keys);
	TList *histstowrite = new TList();
	TList *matstowrite = new TList();
	//int counter = 1;
	while( TKey *currentkey = (TKey*)next() ) {
		std::string keytype = currentkey->ReadObj()->IsA()->GetName();
		if(keytype.compare(0,3,"TH1")==0)	{
			//printf("%i currentkey->GetName() = %s\n",counter++, currentkey->GetName());
			//if((counter-1)%4==0)
			//	printf("*****************************\n");
			histstowrite->Add(currentkey->ReadObj());
		} else if(keytype.compare(0,3,"TH2")==0) {
         matstowrite->Add(currentkey->ReadObj());
      }
	}

	//printf("histstowrite->GetSize() = %i\n", histstowrite->GetSize());
	
	TIter nexthist(histstowrite);
	while( TH1 *currenthist = (TH1*)nexthist() ) {
      std::string outfilename = path + "/";
		outfilename.append(currenthist->GetName());
		outfilename.append(".spe");
		fstream outfile;
		outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
		WriteHist(currenthist, &outfile);
		printf("\t%s written to file %s.\n",currenthist->GetName(),outfilename.c_str());
		outfile.close();
	}


	TIter nextmat(matstowrite);
	while( TH2 *currentmat = (TH2*)nextmat() ) {
      std::string outfilename = path + "/";
		outfilename.append(currentmat->GetName());
		outfilename.append(".mat");
		fstream outfile;
		outfile.open(outfilename.c_str(), std::ios::out | std::ios::binary);
		WriteMat(currentmat, &outfile);
		printf("\t%s written to file %s.\n",currentmat->GetName(),outfilename.c_str());
		outfile.close();
	}

	//printf("closing file %s.\n",outfilename.c_str());
	//outfile.close();

	return 0;
}

void WriteMat(TH2 *mat, fstream *outfile) {
   int xbins = mat->GetXaxis()->GetNbins();
   int ybins = mat->GetYaxis()->GetNbins();
   

   for(int y=0;y<4096;y++ ) {
      int16_t buffer[4096] = {0};
      for(int x=0;x<4096;x++ ) {
         if(x<xbins && y<ybins)
            buffer[x] = (int16_t)(mat->GetBinContent(x,y));
         else
            buffer[x] = 0;
      }
   	outfile->write((char*)(&buffer),8192);	
   }
   
}


void WriteHist(TH1 *hist,fstream *outfile)	{
	SpeHeader spehead;
	spehead.buffsize = 24;
  	strncpy(spehead.label,hist->GetName(),8); 

   if(hist->GetRMS() > 16384/2) {
      while(hist->GetNbinsX()>16384)	{
	     	hist = hist->Rebin(2);
   	}
      spehead.size = hist->GetNbinsX();
   } else if(hist->GetNbinsX()>16384) {
      spehead.size = 16384;
   } else {
      spehead.size = hist->GetNbinsX();
   }

   spehead.junk1 = 1;
	spehead.junk2 = 1;
	spehead.junk3 = 1;
	spehead.buffcheck = 24;         /*fortran file, record ends with record size :) */                // 14

	outfile->write((char*)(&spehead),sizeof(SpeHeader));	

	int32_t histsizeinbytes = spehead.size *4;

	outfile->write((char*)&histsizeinbytes,sizeof(int32_t));	
	float bin = 0.0;
	for(int x=1;x<=spehead.size;x++)	{
		if(x<=hist->GetNbinsX())	{
			bin = (float)hist->GetBinContent(x);
			outfile->write((char*)&bin,sizeof(int32_t));	
		}
		else {
			bin = 0.0;
			outfile->write((char*)&bin,sizeof(int32_t));	
		}
	}

	outfile->write((char*)&histsizeinbytes,sizeof(int32_t));	

	return;	
}











