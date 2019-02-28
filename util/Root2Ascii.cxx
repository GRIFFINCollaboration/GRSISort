// g++ Root2Ascii.cxx `root-config --cflags --libs` -o Root2Ascii
//
//    Short and sweet program. Finds all the 1d (and 2d) histograms in a tfile by
//    itterating over the keys, than turn each of them into an ascii file.
//

#include <cstdint>
#include <fstream>
#include <iostream>

#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>

#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TSpline.h"
#include "TFolder.h"
#include "TSystem.h"
#include "TList.h"
#include "TClass.h"
#include "TKey.h"
#include "TTimeStamp.h"

#include "Globals.h"
#include "GHSym.h"

template <class T> int WriteHistogram(T* histogram, std::string& fileName, std::string& format);
template <class T> int WriteGraph(T* graph, std::string& fileName, std::string& format);
int WriteSpline(TSpline3* s, std::string& fileName);

int main(int argc, char** argv)
{
   TFile* infile = nullptr;
   if(argc < 2 || (infile = TFile::Open(argv[1], "read")) == nullptr) {
      std::cout<<"problem opening file."<<std::endl
               <<"Usage: "<<argv[0]<<" file.root (optional: -<output format> <list of histogram names>)"<<std::endl
					<<"format is e = write error column, z = write zeroes, v = values only, p = pm3d format, g = gnuplot xyerrorbars format, and f = fresco input format"<<std::endl;
      return 1;
   }

	std::string format;
	std::vector<std::string> histNames;
   for(int i = 2; i < argc; ++i) {
      if(argv[i][0] == '-') {
         format = &argv[i][1];
      } else {
			histNames.push_back(argv[i]);
      }
   }

   std::string path = infile->GetName();
   path.erase(path.find_last_of('.'));

#if defined(OS_DARWIN)
   struct stat st = {0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0, 0}, 0, 0, 0, 0, 0, 0, {0}};
#elif defined(__clang__)
   struct stat st = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, {0, 0}, {0, 0}, {0, 0}, {0, 0}};
#else
   struct stat st = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

   if(stat(path.c_str(), &st) == -1) {
      mkdir(path.c_str(), 0755);
   }

	TObject* object = nullptr;
	TKey* key = nullptr;
	// look for histogram names to add
	for(auto name : histNames) {
		object = infile->Get(name.c_str());
		// if we couldn't find the object, loop through all object in the file, and if they're folders search in those
		if(object == nullptr) {
			TIter next(infile->GetListOfKeys());
			while((key = static_cast<TKey*>(next())) != nullptr) {
				object = key->ReadObj();
				if(strcmp(object->ClassName(),"TFolder") == 0) {
					TFolder* folder = static_cast<TFolder*>(object);
					object = folder->FindObject(folder->FindFullPathName(name.c_str()));
					break;
				}
			}
			if(key == nullptr || object == nullptr) {
				std::cerr<<"couldn't find histogram "<<name<<" in file "<<infile->GetName()<<std::endl;
				continue;
			}
		}

		//check what type this object is: 1D- or 2D-histogram, spline, graph w/ or w/o errors
      std::string keyType = key->ReadObj()->ClassName();
		std::string fileName = path;
		path += "/";
		path += name;
		path += ".dat";
		if(keyType.compare(0, 3, "TH1") == 0) {
			WriteHistogram(static_cast<TH1*>(object), fileName, format);
		} else if(keyType.compare(0, 3, "TH2") == 0) {
			WriteHistogram(static_cast<TH2*>(object), fileName, format);
		} else if(keyType.compare(0, 7, "TSpline") == 0) {
			WriteSpline(static_cast<TSpline3*>(object), fileName);
		} else if(keyType.compare("TGraph") == 0) {
			WriteGraph(static_cast<TGraph*>(object), fileName, format);
		} else if(keyType.compare("TGraphErrors") == 0) {
			WriteGraph(static_cast<TGraphErrors*>(object), fileName, format);
		} else if(keyType.compare("TGraphAsymmErrors") == 0) {
			WriteGraph(static_cast<TGraphAsymmErrors*>(object), fileName, format);
      } else if(keyType.compare(0, 5, "GHSym") == 0) {
         if(keyType.compare(5, 1, "F") == 0) {
				WriteHistogram(static_cast<GHSymF*>(object)->GetMatrix(), fileName, format);
         } else if(keyType.compare(5, 1, "D") == 0) {
				WriteHistogram(static_cast<GHSymD*>(object)->GetMatrix(), fileName, format);
         } else {
            std::cout<<"unknown GHSym type "<<keyType<<std::endl;
         }
      } else {
         std::cout<<"skipping "<<keyType<<std::endl;
		}
	}

	if(histNames.empty()) {
		// loop through all keys to find any object we can write
		TIter next(infile->GetListOfKeys());
		while((key = static_cast<TKey*>(next())) != nullptr) {
			object = key->ReadObj();
			std::string keyType = object->ClassName();
			std::string fileName = path;
			path += "/";
			path += object->GetName();
			path += ".dat";
			if(keyType.compare(0, 3, "TH1") == 0) {
				WriteHistogram(static_cast<TH1*>(object), fileName, format);
			} else if(keyType.compare(0, 3, "TH2") == 0) {
				WriteHistogram(static_cast<TH2*>(object), fileName, format);
			} else if(keyType.compare(0, 7, "TSpline") == 0) {
				WriteSpline(static_cast<TSpline3*>(object), fileName);
			} else if(keyType.compare("TGraph") == 0) {
				WriteGraph(static_cast<TGraph*>(object), fileName, format);
			} else if(keyType.compare("TGraphErrors") == 0) {
				WriteGraph(static_cast<TGraphErrors*>(object), fileName, format);
			} else if(keyType.compare("TGraphAsymmErrors") == 0) {
				WriteGraph(static_cast<TGraphAsymmErrors*>(object), fileName, format);
			} else if(keyType.compare(0, 5, "GHSym") == 0) {
				if(keyType.compare(5, 1, "F") == 0) {
					WriteHistogram(static_cast<GHSymF*>(object)->GetMatrix(), fileName, format);
				} else if(keyType.compare(5, 1, "D") == 0) {
					WriteHistogram(static_cast<GHSymD*>(object)->GetMatrix(), fileName, format);
				} else {
					std::cout<<"unknown GHSym type "<<keyType<<std::endl;
				}
			} else {
				std::cout<<"skipping "<<keyType<<std::endl;
			}
		}
	}

	infile->Close();

	return 0;
}

template <class T> int WriteHistogram(T* histogram, std::string& fileName, std::string& format) 
{
	//the number of bins is 1 for dimensions that aren't covered by histogram type
	int xNumberOfBins = histogram->GetNbinsX();
	int yNumberOfBins = histogram->GetNbinsY();
	int zNumberOfBins = histogram->GetNbinsZ();

	//possible formats are (combinations possible):
	//p: write 2d histogram to be used as input for gnuplots pm3d mode
	//e: write errors
	//z: zero suppression
	//v: values only (no x,y or z)

	//format flags
	bool writeErrors = false;
	bool writeZeros = true;
	bool writeValuesOnly = false;
	bool writePm3d = false;

	//check the format
	std::transform(format.begin(), format.end(), format.begin(), ::tolower);

	if(format.find("e") != std::string::npos) {
		writeErrors = true;
	}
	if(format.find("z") != std::string::npos) {
		writeZeros = false;
	}
	if(format.find("v") != std::string::npos) {
		writeValuesOnly = true;
	}
	if(format.find("p") != std::string::npos) {
		writePm3d = true;
	}

	if(writePm3d && (writeErrors || writeValuesOnly)) {
		std::cerr<<"Error, can't write pm3d format and errors or only values!"<<std::endl;
		return 1;
	}
	if(writePm3d && (yNumberOfBins == 1 || zNumberOfBins > 1)) {
		std::cerr<<"Error, pm3d format requested but histogram doesn't seem to be a TH2: yNumberOfBins = "<<yNumberOfBins<<", zNumberOfBins = "<<zNumberOfBins<<", class name = "<<histogram->IsA()->GetName()<<std::endl;
		return 2;
	}

	std::ofstream output(fileName);
	if(output.rdstate() != 0) {
		std::cerr<<"Error, couldn't open file "<<fileName<<std::endl;
		return 1;
	}

	if(!writePm3d) {
		//default: write x(,y,z) and counts
		//if no y (or z) axis exist the respective loop will execute only once (BinLow == NumberOfBins == 1)
		//only write the y (or z) if BinHigh > 1
		for(int i = 1; i <= xNumberOfBins; i++) {
			for(int j = 1; j <= yNumberOfBins; j++) {
				for(int k = 1; k <= zNumberOfBins; k++) {
					if(writeZeros || histogram->GetBinContent(i,j,k) > 0) {
						if(!writeValuesOnly) {
							output<<std::setw(8)<<histogram->GetXaxis()->GetBinCenter(i);
							if(yNumberOfBins > 1)
								output<<" "<<std::setw(8)<<histogram->GetYaxis()->GetBinCenter(j);
							if(zNumberOfBins > 1)
								output<<" "<<std::setw(8)<<histogram->GetZaxis()->GetBinCenter(k);
						}
						output<<" "<<std::setw(8)<<histogram->GetBinContent(i,j,k);
						if(writeErrors)
							output<<" "<<std::setw(8)<<histogram->GetBinError(i,j,k);
						output<<std::endl;
					}
				}
			}
		}
	} else if(writePm3d) {
		// special formatting for gnuplots pm3d (set pm3d map ; splot 'fileName')
		// all counts are written 
		// blank lines between different x
		// if all values are zero for multiple consecutive x-values only the first and last are written
		// if values of consecutive y are all the same only the first is printed
		// after each x-set the same set is printed again at the next x-value

		//we need to get the lower edge of the next bins as well
		std::vector<double> x(xNumberOfBins+1);
		std::vector<std::vector<double> > y(xNumberOfBins+1);
		std::vector<std::vector<double> > values(xNumberOfBins+1);

		size_t erasedX = 0;
		std::vector<size_t> erasedY(xNumberOfBins+1,0);

		//get the values
		for(int i = 1; i <= xNumberOfBins+1; ++i) {
			x.at(i-1-erasedX) = histogram->GetXaxis()->GetBinLowEdge(i);

			y.at(i-1-erasedX).resize(yNumberOfBins+1);
			values.at(i-1-erasedX).resize(yNumberOfBins+1);

			//read the first y-value in any case
			y.at(i-1-erasedX).at(0) = histogram->GetYaxis()->GetBinLowEdge(1);
			values.at(i-1-erasedX).at(0) = histogram->GetBinContent(i, 1);

			for(int j = 2; j < yNumberOfBins+1; ++j) {
				//check whether this value is different than the last (or the last value)
				if(histogram->GetBinContent(i,j) != values.at(i-1-erasedX).at(j-1-erasedY.at(i-1-erasedX)-1) || j == yNumberOfBins) {
					y.at(i-1-erasedX).at(j-1-erasedY.at(i-1-erasedX)) = histogram->GetYaxis()->GetBinLowEdge(j);
					values.at(i-1-erasedX).at(j-1-erasedY.at(i-1-erasedX)) = histogram->GetBinContent(i,j);
				} else {
					//erase this y and increase yBinLow
					values.at(i-1-erasedX).erase(values.at(i-1-erasedX).begin()+j-1-erasedY.at(i-1-erasedX));
					y.at(i-1-erasedX).erase(y.at(i-1-erasedX).begin()+j-1-erasedY.at(i-1-erasedX));
					erasedY.at(i-1-erasedX)++;
				}
			}
			//check if this x and the last had the same values (and are not the first x)
			//=> if so erase this x (i) and increase erasedX
			if(1 < i && i < xNumberOfBins+1) {
				if(values.at(i-1-erasedX) == values.at(i-1-erasedX-1)) {
					values.erase(values.begin()+i-1-erasedX);
					y.erase(y.begin()+i-1-erasedX);
					x.erase(x.begin()+i-1-erasedX);
					erasedY.erase(erasedY.begin()+i-1-erasedX);
					erasedX++;
				}
			} else {//if(i > xBinLow)
				output<<"not erasing values since i = "<<i<<" not between 1 and "<<xNumberOfBins+1-erasedX<<std::endl;
			}
		}

		//printing of values (and instructions)
		output<<"#set pm3d map corners2color c1"<<std::endl
			<<"#set palette rgbformulae 22,13,-31"<<std::endl
			<<"#splot 'fileName'"<<std::endl;
		for(size_t i = 0; i < values.size(); i++) {
			for(size_t j = 0; j < values.at(i).size(); j++) {
				output<<std::setw(8)<<x.at(i)<<std::setw(8)<<y.at(i).at(j)<<std::setw(8)<<values.at(i).at(j)<<std::endl;
			}
			output<<std::endl;
			if(i < values.size()-1) {
				for(size_t j = 0; j < values.at(i).size(); j++) {
					output<<std::setw(8)<<x.at(i+1)<<std::setw(8)<<y.at(i).at(j)<<std::setw(8)<<values.at(i).at(j)<<std::endl;
				}
				output<<std::endl;
			} else {
				for(size_t j = 0; j < values.at(i).size(); j++) {
					output<<std::setw(8)<<x.at(i)+histogram->GetXaxis()->GetBinWidth(1)<<std::setw(8)<<y.at(i).at(j)<<std::setw(8)<<values.at(i).at(j)<<std::endl;
				}
				output<<std::endl;
			}
		}
	}

	output.close();

	return 0;
}

template <class T> int WriteGraph(T* graph, std::string& fileName, std::string& format)
{
	std::ofstream output(fileName);
	if(output.rdstate() != 0) {
		std::cerr<<"Error, couldn't open file "<<fileName<<std::endl;
		return 1;
	}

	//possible formats are (combinations possible):
	//g: x,y,xlow,xhigh,ylow,yhigh, and lows and highs are the absolute values (if TGraphAsymmErrors)
	//      or x,y,delta x,delta y (if TGraphErrors)
	//f: write x,y,errror
	//e: write errors
	//z: no zero suppression

	//format flags
	bool writeErrors = false;
	bool writeZeros = false;
	bool gnuplot = false;
	bool fresco = false;

	//check the format
	std::transform(format.begin(), format.end(), format.begin(), ::tolower);

	if(format.find("e") != std::string::npos) {
		writeErrors = true;
	}
	if(format.find("z") != std::string::npos) {
		writeZeros = true;
	}
	if(format.find("g") != std::string::npos) {
		gnuplot = true;
	}
	if(format.find("f") != std::string::npos) {
		fresco = true;
	}

	if(gnuplot && fresco) {
		std::cerr<<"Error, can't write gnuplot and fresco format at once!"<<std::endl;
		return 1;
	}

	double x, y;

	for(int point = 0; point < graph->GetN(); point++) {
		graph->GetPoint(point, x, y);
		if(y != 0 || writeZeros) {
			if(!writeErrors && !gnuplot && !fresco) {
				output<<std::setw(8)<<x<<" "<<std::setw(8)<<y<<std::endl;
			} else if(gnuplot) {
				if(strcmp(graph->IsA()->GetName(),"TGraphAsymmErrors") == 0) {
					//x,y,xlow,xhigh,ylow,yhigh, and lows and highs are the absolute values
					output<<std::setw(8)<<x<<" "<<std::setw(8)<<y<<" "<<std::setw(8)<<x-graph->GetErrorXlow(point)<<" "<<std::setw(8)<<x+graph->GetErrorXhigh(point)<<" "<<std::setw(8)<<y-graph->GetErrorYlow(point)<<" "<<std::setw(8)<<y+graph->GetErrorYhigh(point)<<std::endl;
				} else {
					//x,y,delta x,delta y
					output<<std::setw(8)<<x<<" "<<std::setw(8)<<y<<" "<<std::setw(8)<<graph->GetErrorX(point)<<" "<<std::setw(8)<<graph->GetErrorY(point)<<std::endl;
				}
			} else if(fresco) {
				//x,y,yerror
				output<<std::setw(8)<<x<<" "<<std::setw(8)<<y<<" "<<std::setw(8)<<graph->GetErrorY(point)<<std::endl;
			} else {
				output<<std::setw(8)<<x<<" "<<std::setw(8)<<graph->GetErrorXlow(point)<<" "<<std::setw(8)<<graph->GetErrorXhigh(point)<<" "<<std::setw(8)<<y<<" "<<std::setw(8)<<graph->GetErrorYlow(point)<<" "<<std::setw(8)<<graph->GetErrorYhigh(point)<<std::endl;
			}
		}
	}

	output.close();

	return 0;
}

int WriteSpline(TSpline3* s, std::string& fileName)
{
	std::ofstream output(fileName);
	output.open(fileName);
	if(output.rdstate() != 0) {
		std::cerr<<"Error, couldn't open file "<<fileName<<std::endl;
		return 1;
	}

	for(double x = s->GetXmin(); x <= s->GetXmax(); x += (s->GetXmax() - s->GetXmin())/s->GetNp()) {
		output<<x<<" "<<s->Eval(x)<<std::endl;;
	}

	output.close();

	return 0;
}
