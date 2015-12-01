//g++ -c -fPIC Nucleus.cc -I./ `root-config --cflags`

#include "Globals.h"

#include "TNucleus.h"
#include <algorithm>
#include <cstring>
#include <sstream>

#include <TClass.h>

//#define debug

/// \cond CLASSIMP
ClassImp(TNucleus)
/// \endcond

/////////////////////////////////////////////////////////////////
//
// TNucleus
//
// This class builds a nucleus and sets all the basic information
// (mass, Z, symbol, radius, etc.)
//
/////////////////////////////////////////////////////////////////

//const char *TNucleus::massfile = "/home/tiguser/packages/GRSISort/libraries/TGRSIAnalysis/SourceData/mass.dat";

static double amu = 931.494043;
//static double MeV2Kg = 1.77777778e-30;

const char *TNucleus::massfile = "/libraries/TGRSIAnalysis/SourceData/mass.dat";


TNucleus::TNucleus(const char *name){
	//Creates a nucleus based on symbol (ex. 26Na OR Na26) and sets all parameters from mass.dat
	std::string Name = name;
	//SetMassFile();
	int Number = 0;
	std::string symbol;
	std::string element;
	Name.erase(std::remove_if(Name.begin(), Name.end(), (int(*)(int))std::isspace), Name.end());

	if(Name.length()<2) {
    switch(Name[0]){
      case 'p':
        Name.clear();
        Name.assign("h1");
        break;
      case 'd':
        Name.clear();
        Name.assign("h2");
        break;
      case 't':   
        Name.clear();   
        Name.assign("h3");
        break;
      case 'a':   
        Name.clear();   
        Name.assign("he4");
        break;
      default:
       printf("error, type numbersymbol, or symbolnumber, i.e. 30Mg oder Mg30\n");
       return;
    };
  }
	int first_digit  = Name.find_first_of("0123456789 \t\n\r");
	int first_letter = Name.find_first_not_of("0123456789 \t\n\r");
	if(first_digit>first_letter) {
		Number = atoi(Name.substr(first_digit).c_str());
		symbol.append(Name.substr(first_letter,first_digit-first_letter));
   } else {
		Number = atoi(Name.substr(first_digit,first_letter-first_digit).c_str());
		symbol.append(Name.substr(first_letter));
	}

	element.append(std::to_string((long long)Number)); element.append(symbol);
	std::string line;
	std::ifstream infile;
	std::string MassFile = getenv("GRSISYS");
   MassFile.append(massfile);
	infile.open(MassFile.c_str());
	int z,n;
	std::string sym_name;
	double mass;	
 	bool found = false;
	while(getline(infile,line)) {
		if(line.length() <1)
			continue;
//		printf("%s\n",line.c_str());
		std::stringstream ss(line);
		ss>>n; ss>>z; ss>>sym_name; ss>>mass; 		
		if(strcasecmp(element.c_str(),sym_name.c_str()) == 0) {
			found = true;
			break;
		}
	}
	if(!found) {
		printf("Warning: Element %s not found in the mass table %s/%s.\n Nucleus not Set!\n",element.c_str(),getenv("GRSISYS"),massfile);
		return;
	}
	infile.close();
	SetZ(z);
	SetN(n);
  SetMassExcess(mass/1000.0);
  SetMass();
	SetSymbol(symbol.c_str());
  SetName(element.c_str());
  SetSourceData();
}
/*
*/
TNucleus::TNucleus(int charge, int neutrons, double mass, const char* symbol){
// Creates a nucleus with Z, N, mass, and symbol
  //SetMassFile();
  fZ = charge;  
  fN = neutrons;
  fSymbol = symbol;
  fMass = mass;
  SetName(symbol);
  SetSourceData();
}

TNucleus::TNucleus(int charge, int neutrons, const char* MassFile){
// Creates a nucleus with Z, N using mass table (default MassFile = "mass.dat")
  //SetMassFile();
  if(!MassFile) {
	  std::string SMassFile = getenv("GRSISYS");
	  SMassFile.append(massfile);
	  MassFile = SMassFile.c_str();
	}
  fZ = charge;  
  fN = neutrons;
  int i = 0,n,z;
  double emass;
  char tmp[256];
  std::ifstream mass_file;
  mass_file.open(MassFile,std::ios::in);
  while(!mass_file.bad() && !mass_file.eof() && i < 3008){
    mass_file>>n;
    mass_file>>z;
    mass_file>>tmp;
    mass_file>>emass;
    if(n==fN&&z==fZ){
      fMassExcess = emass/1000.;
      fSymbol = tmp;
#ifdef debug
      cout << "Symbol " << fSymbol << " tmp " << tmp <<endl;
#endif
      SetMass();
      SetSymbol(fSymbol.c_str());
      break;
    }
    i++;
    mass_file.ignore(256,'\n');
  }  
  //max_elements=i;

  mass_file.close();
	std::string name = fSymbol;
	std::string number = name.substr(0,name.find_first_not_of("0123456789 "));

	name = name.substr(name.find_first_not_of("0123456789 "));
	name.append(number);
  SetName(name.c_str());
  SetSourceData();
}

//void TNucleus::SetA(int aval){
//  fA = aval;
//}

TNucleus::~TNucleus(){
   TransitionList.Delete();
}

const char* TNucleus::SortName(const char* name){
//Names a nucleus based on symbol (ex. 26Na OR Na26). This is to get a nice naming convention.
	std::string Name = name;
	//SetMassFile();
	int Number = 0;
	std::string symbol;
	std::string element;
	Name.erase(std::remove_if(Name.begin(), Name.end(), (int(*)(int))std::isspace), Name.end());

	if(Name.length()<2) {
    switch(Name[0]){
      case 'p':
        Name.clear();
        Name.assign("h1");
        break;
      case 'd':
        Name.clear();
        Name.assign("h2");
        break;
      case 't':   
        Name.clear();   
        Name.assign("h3");
        break;
      case 'a':   
        Name.clear();   
        Name.assign("he4");
        break;
      default:
       printf("error, type numbersymbol, or symbolnumber, i.e. 30Mg oder Mg30\n");
       return NULL;
    };
  }
	int first_digit  = Name.find_first_of("0123456789 \t\n\r");
	int first_letter = Name.find_first_not_of("0123456789 \t\n\r");
	if(first_digit>first_letter) {
		Number = atoi(Name.substr(first_digit).c_str());
		symbol.append(Name.substr(first_letter,first_digit-first_letter));
   } else {
		Number = atoi(Name.substr(first_digit,first_letter-first_digit).c_str());
		symbol.append(Name.substr(first_letter));
	}
   std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
   symbol[0] = toupper(symbol[0]);
	element.append(std::to_string((long long)Number)); element.append(symbol);

   return element.c_str();
   
}

void TNucleus::SetZ(int charge){
// Sets the Z (# of protons) of the nucleus
  fZ = charge;
}
void TNucleus::SetN(int neutrons){
// Sets the N (# of neutrons) of the nucleus
  fN = neutrons;
}
void TNucleus::SetMassExcess(double mass_ex){
// Sets the mass excess of the nucleus (in MeV)
  fMassExcess = mass_ex;
}


void TNucleus::SetMass(double mass){
// Sets the mass manually (in MeV)
  fMass = mass;
 }

void TNucleus::SetMass(){
// Sets the mass based on the A and mass excess of nucleus (in MeV)
  fMass = amu*GetA()+GetMassExcess();
}

void TNucleus::SetSymbol(const char* symbol){
// Sets the atomic symbol for the nucleus
  fSymbol = symbol;
}

int TNucleus::GetZfromSymbol(char* symbol) {
// Figures out the Z of the nucleus based on the atomic symbol
  char symbols[105][3] = {"H","HE","LI","BE","B","C","N","O","F","NE","NA","MG","AL","SI","P","S","CL","AR","K","CA","SC","TI","V","CR","MN","FE","CO","NI","CU","ZN","GA","GE","AS","SE","BR","KR","RB","SR","Y","ZR","NB","MO","TC","RU","RH","PD","AG","CD","IN","SN","SB","TE","F","XE","CS","BA","LA","CE","PR","ND","PM","SM","EU","GD","TB","DY","HO","ER","TM","YB","LU","HF","TA","W","RE","OS","IR","PT","AU","HG","TI","PB","BI","PO","AT","RN","FR","RA","AC","TH","PA","U","NP","PU","AM","CM","BK","CF","ES","FM","MD","NO","LR","RF","HA"};
  int length = strlen(symbol);
  //cout << symbol << "   " << length << endl;
  char* search = new char[length+1];
  for(int i=0;i<length;i++){
    search[i] = toupper(symbol[i]); // make sure symbol is in uppercase
    }
  search[length] = '\0';
  for(int i=0;i<105;i++){
    if(strcmp(search,symbols[i]) == 0){
      delete[] search;
      SetZ(i+1);
      return i+1;
    }
  }

  delete[] search;
  SetZ(0);
  return 0;
}

double TNucleus::GetRadius() const{
// Gets the radius of the nucleus (in fm).
// The radius is calculated using 1.12*A^1/3 - 0.94*A^-1/3
  return 1.12*pow(this->GetA(),1./3.) - 0.94*pow(this->GetA(),-1./3.);
}

bool TNucleus::SetSourceData() {

   std::string name = GetSymbol();
   if(name.length()==0)
      return false;
  
   if(name[0]<='Z' && name[0]>='A')
      name[0] = name[0]-'A'+'a'; 
   name = name + Form("%i",GetA()) + ".sou";
   std::string path = getenv("GRSISYS");
	path +=  "/libraries/TGRSIAnalysis/SourceData/";
   path +=  name;

   printf("path = %s\n",path.c_str());
	std::ifstream sourcefile;
   sourcefile.open(path.c_str());
   if(!sourcefile.is_open()) {
      printf("unable to set source data for %s.\n",GetName());
      return false;
   }

   TransitionList.Clear();

   std::string line;
   int linenumber = 0;
   while(getline(sourcefile,line)) {
      linenumber++;
      size_t comment = line.find("//");
      if (comment != std::string::npos) 
         line = line.substr(0, comment);
      if(line.length()==0)
         continue;
      TGRSITransition *tran = new TGRSITransition; 
      std::stringstream ss(line);
      ss >> tran->fEnergy;
      ss >> tran->fEnergyUncertainty;
      ss >> tran->fIntensity;
      ss >> tran->fIntensityUncertainty;
      TransitionList.Add(tran);
    //  printf("eng: %.02f\tinten: %.02f\n",((TGRSITransition*)TransitionList.Last())->fEnergy,((TGRSITransition*)TransitionList.Last())->fIntensity);
   }                                                                                                         

   printf("Found %d Transitions for %s\n",TransitionList.GetSize(),GetName());
   return true;

}

void TNucleus::AddTransition(Double_t energy, Double_t intensity, Double_t energyUncertainty, Double_t intensityUncertainty){
   TGRSITransition * tran = new TGRSITransition;
   tran->fEnergy = energy;
   tran->fEnergyUncertainty = energyUncertainty;
   tran->fIntensity = intensity;
   tran->fIntensityUncertainty = intensityUncertainty;

   AddTransition(tran);
}

void TNucleus::AddTransition(TGRSITransition* tran){
   TransitionList.Add(tran);
}

Bool_t TNucleus::RemoveTransition(Int_t idx){
   TGRSITransition *tran;
   tran = static_cast<TGRSITransition*>(TransitionList.RemoveAt(idx));
   if(tran){
      printf("Removed transition: ");
      printf("%d\t eng: %.02f\tinten: %.02f\n",idx,tran->fEnergy,tran->fIntensity);
      delete tran;
      return true;
   }
   else{
      printf("Out of range\n");
      return false;
   }
}

TGRSITransition* TNucleus::GetTransition(Int_t idx){
   TGRSITransition *tran = static_cast<TGRSITransition*>(TransitionList.At(idx));
   if(!tran)
      printf("Out of Range\n");

   return tran;
}

void TNucleus::Print(Option_t *opt) const{
//Prints out the Name of the nucleus, as well as the numerated transition list
   printf("Nucleus: %s\n",GetName());
   for(int i =0; i< TransitionList.GetSize();i++){
      printf("%d\t eng: %.02f\tinten: %.02f\n",i,static_cast<TGRSITransition*>(TransitionList.At(i))->fEnergy,static_cast<TGRSITransition*>(TransitionList.At(i))->fIntensity);

   }
}

void TNucleus::WriteSourceFile(std::string outfilename){
   if(outfilename.length() > 0) {
	  std::ofstream sourceout;
     sourceout.open(outfilename.c_str());
     for(int i=0; i < TransitionList.GetSize(); i++)   {
        std::string transtr = static_cast<TGRSITransition*>(TransitionList.At(i))->PrintToString();
        sourceout << transtr.c_str();
        sourceout << std::endl;
     }
     sourceout << std::endl;
     sourceout.close();
   } 

}



