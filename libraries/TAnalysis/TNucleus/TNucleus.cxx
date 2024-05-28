// g++ -c -fPIC Nucleus.cc -I./ `root-config --cflags`

#include "Globals.h"

#include "TNucleus.h"
#include <algorithm>
#include <cstring>
#include <sstream>

#include <TClass.h>
#include <TGraph.h>

/// \cond CLASSIMP
ClassImp(TNucleus)
/// \endcond

static double amu = 931.494043;

std::string& TNucleus::massfile()
{
	static std::string output = std::string(getenv("GRSISYS")) + "/libraries/TAnalysis/SourceData/mass.dat";
	return output;
}

TNucleus::TNucleus(const char* name)
{
	// Creates a nucleus based on symbol (ex. 26Na OR Na26) and sets all parameters from mass.dat
	int           number = 0;
	std::string   symbol;
	std::string   element;
	int           z, n;
	std::string   sym_name;
	double        mass;
	bool          found = false;
	std::string   line;
	std::ifstream infile;
	std::string   massFile;
	ParseName(name, symbol, number, element);
	try {
		massFile = massfile();
		infile.open(massFile.c_str());
		while(!getline(infile, line).fail()) {
			if(line.length() < 1) {
				continue;
			}
			std::stringstream ss(line);
			ss >> n;
			ss >> z;
			ss >> sym_name;
			ss >> mass;
			if(strcasecmp(element.c_str(), sym_name.c_str()) == 0) {
				found = true;
				break;
			}
		}
	} catch(std::out_of_range&) {
		std::cout<<"Could not parse element "<<name<<std::endl<<"Nucleus not Set!"<<std::endl;
		return;
	}
	if(!found) {
		std::cout<<"Warning: Element "<<element<<" not found in the mass table "<<massFile<<"."<<std::endl
		         <<"Nucleus not set!"<<std::endl;
		return;
	}
	infile.close();
	SetZ(z);
	SetN(n);
	SetMassExcess(mass / 1000.0);
	SetMass();
	SetSymbol(symbol.c_str());
	SetName();
	LoadTransitionFile();
}

TNucleus::TNucleus(int charge, int neutrons, double mass, const char* symbol)
{
	// Creates a nucleus with Z, N, mass, and symbol
	fZ      = charge;
	fN      = neutrons;
	fSymbol = symbol;
	fMass   = mass;
	SetName();
	LoadTransitionFile();
}

TNucleus::TNucleus(int charge, int neutrons, const char* MassFile)
{
	// Creates a nucleus with Z, N using mass table (default MassFile = "mass.dat")
	if(MassFile == nullptr) {
		MassFile = massfile().c_str();
	}
	fZ              = charge;
	fN              = neutrons;
	int           i = 0, n, z;
	double        emass;
	char          tmp[256];
	std::ifstream mass_file;
	mass_file.open(MassFile, std::ios::in);
	while(!mass_file.bad() && !mass_file.eof() && i < 3008) {
		mass_file >> n;
		mass_file >> z;
		mass_file >> tmp;
		mass_file >> emass;
		if(n == fN && z == fZ) {
			fMassExcess = emass / 1000.;
			fSymbol     = tmp;
#ifdef debug
			cout<<"Symbol "<<fSymbol<<" tmp "<<tmp<<endl;
#endif
			SetMass();
			SetSymbol(fSymbol.c_str());
			break;
		}
		i++;
		mass_file.ignore(256, '\n');
	}

	mass_file.close();
	std::string name   = fSymbol;
	std::string number = name.substr(0, name.find_first_not_of("0123456789 "));

	name = name.substr(name.find_first_not_of("0123456789 "));
	name.append(number);

	SetName();
	LoadTransitionFile();
}

void TNucleus::SetName(const char*)
{
	std::string name = GetSymbol();
	name.append(std::to_string(GetA()));
	TNamed::SetName(name.c_str());
}

TNucleus::~TNucleus()
{
	fTransitionListByIntensity.Delete();
	fTransitionListByEnergy.Delete();
}

void TNucleus::ParseName(std::string name, std::string& symbol, int& number, std::string& element)
{
	/// Strips any non-alphanumeric character from input, parses rest as number and symbol.
	/// E.g. turns "na26" into "Na" and 26 or "  152  EU... " into "Eu" and 152.
	/// Only uses the first number and first letter, so "na26   eu152" would be turned into "Na" and 26,
	/// but "na26  152eu" would be turned into "Na" and 26152, and "26na  152eu" would be turned into "Na152eu" and 26.
	/// Special inputs are "p" for "H" and 1, "d" for "H" and 2, "t" for "H" and 3, and "a" for "He" and 4.
	/// element simply is the combined string of number and element.
	
	// remove any characters that aren't alphanumeric
	name.erase(std::remove_if(name.begin(), name.end(), [](char c) { return !std::isalnum(c); }), name.end());

	// special single letter cases
	if(name.length() == 1) {
		switch(name[0]) {
			case 'p':
				symbol.assign("H");
				return ;
			case 'd':
				symbol.assign("H");
				return;
			case 't':
				symbol.assign("H");
				return;
			case 'a':
				symbol.assign("He");
				return;
			default:
				std::cout<<"error, type numbersymbol, or symbolnumber, i.e. 30Mg oder Mg30, not "<<name<<std::endl;
				return;
		};
	}
	
	int firstDigit  = name.find_first_of("0123456789");
	int firstLetter = name.find_first_not_of("0123456789");
	if(firstDigit > firstLetter) {
		number = atoi(name.substr(firstDigit).c_str());
		symbol.append(name.substr(firstLetter, firstDigit - firstLetter));
	} else {
		number = atoi(name.substr(firstDigit, firstLetter - firstDigit).c_str());
		symbol.append(name.substr(firstLetter));
	}
	// make certain the symbol starts with upper case and rest is lower case by first transforming everything to lower case and then the first character to upper case.
	std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
	symbol[0] = toupper(symbol[0]);
	element.append(std::to_string(static_cast<long long>(number)));
	element.append(symbol);
}

std::string TNucleus::SortName(std::string input)
{
	/// Strips any non-alphanumeric character from input, parses rest as number and symbol.
	/// E.g. turns "na26" into "26Na" or "  152  EU... " into "152Eu".
	/// Special inputs are "p" for "1H", "d" for "2H", "t" for "2H", and "a" for "4He".
	int         number = 0;
	std::string symbol;
	std::string element;
	ParseName(input, symbol, number, element);

	return element;
}

void TNucleus::SetZ(int charge)
{
	// Sets the Z (# of protons) of the nucleus
	fZ = charge;
}
void TNucleus::SetN(int neutrons)
{
	// Sets the N (# of neutrons) of the nucleus
	fN = neutrons;
}
void TNucleus::SetMassExcess(double mass_ex)
{
	// Sets the mass excess of the nucleus (in MeV)
	fMassExcess = mass_ex;
}

void TNucleus::SetMass(double mass)
{
	// Sets the mass manually (in MeV)
	fMass = mass;
}

void TNucleus::SetMass()
{
	// Sets the mass based on the A and mass excess of nucleus (in MeV)
	fMass = amu * GetA() + GetMassExcess();
}

void TNucleus::SetSymbol(const char* symbol)
{
	// Sets the atomic symbol for the nucleus
	fSymbol = symbol;
}

int TNucleus::GetZfromSymbol(char* symbol)
{
	// Figures out the Z of the nucleus based on the atomic symbol
	char symbols[105][3] = {"H",  "HE", "LI", "BE", "B",  "C",  "N",  "O",  "F",  "NE", "NA", "MG", "AL", "SI", "P",
		"S",  "CL", "AR", "K",  "CA", "SC", "TI", "V",  "CR", "MN", "FE", "CO", "NI", "CU", "ZN",
		"GA", "GE", "AS", "SE", "BR", "KR", "RB", "SR", "Y",  "ZR", "NB", "MO", "TC", "RU", "RH",
		"PD", "AG", "CD", "IN", "SN", "SB", "TE", "F",  "XE", "CS", "BA", "LA", "CE", "PR", "ND",
		"PM", "SM", "EU", "GD", "TB", "DY", "HO", "ER", "TM", "YB", "LU", "HF", "TA", "W",  "RE",
		"OS", "IR", "PT", "AU", "HG", "TI", "PB", "BI", "PO", "AT", "RN", "FR", "RA", "AC", "TH",
		"PA", "U",  "NP", "PU", "AM", "CM", "BK", "CF", "ES", "FM", "MD", "NO", "LR", "RF", "HA"};
	int length = strlen(symbol);
	// cout<<symbol<<"   "<<length<<endl;
	auto* search = new char[length + 1];
	for(int i = 0; i < length; i++) {
		search[i] = toupper(symbol[i]); // make sure symbol is in uppercase
	}
	search[length] = '\0';
	for(int i = 0; i < 105; i++) {
		if(strcmp(search, symbols[i]) == 0) {
			delete[] search;
			SetZ(i + 1);
			return i + 1;
		}
	}

	delete[] search;
	SetZ(0);
	return 0;
}

double TNucleus::GetRadius() const
{
	// Gets the radius of the nucleus (in fm).
	// The radius is calculated using 1.12*A^1/3 - 0.94*A^-1/3
	return 1.12 * pow(GetA(), 1. / 3.) - 0.94 * pow(GetA(), -1. / 3.);
}

void TNucleus::AddTransition(Double_t energy, Double_t intensity, Double_t energy_uncertainty,
		Double_t intensity_uncertainty)
{
	auto* tran = new TTransition();
	tran->SetEnergy(energy);
	tran->SetEnergyUncertainty(energy_uncertainty);
	tran->SetIntensity(intensity);
	tran->SetIntensityUncertainty(intensity_uncertainty);

	AddTransition(tran);
}

void TNucleus::AddTransition(TTransition* tran)
{
	fTransitionListByIntensity.Add(tran);
	auto* tran2 = new TTransition(*tran);
	tran2->SetCompareIntensity(false);
	fTransitionListByEnergy.Add(tran2);
}

TTransition* TNucleus::GetTransitionByIntensity(Int_t idx)
{
	TTransition* tran = static_cast<TTransition*>(fTransitionListByIntensity.At(idx));
	if(tran == nullptr) {
		std::cout<<"Out of Range"<<std::endl;
	}

	return tran;
}

TTransition* TNucleus::GetTransitionByEnergy(Int_t idx)
{
	TTransition* tran = static_cast<TTransition*>(fTransitionListByEnergy.At(idx));
	if(tran == nullptr) {
		std::cout<<"Out of Range"<<std::endl;
	}

	return tran;
}

void TNucleus::Print(Option_t*) const
{
	// Prints out the Name of the nucleus, as well as the numerated transition list
	std::cout<<"Nucleus: "<<GetName()<<std::endl;
	TIter next(&fTransitionListByIntensity);
	int   counter = 0;
	while(TTransition* tran = static_cast<TTransition*>(next())) {
		std::cout<<"\t"<<counter++<<"\t";
		tran->Print();
	}
}

void TNucleus::WriteSourceFile(const std::string& outfilename)
{
	if(outfilename.length() > 0) {
		std::ofstream sourceout;
		sourceout.open(outfilename.c_str());
		for(int i = 0; i < fTransitionListByIntensity.GetSize(); i++) {
			std::string transtr = (static_cast<TTransition*>(fTransitionListByIntensity.At(i)))->PrintToString();
			sourceout<<transtr.c_str();
			sourceout<<std::endl;
		}
		sourceout<<std::endl;
		sourceout.close();
	}
}

bool TNucleus::LoadTransitionFile()
{
	if(fTransitionListByIntensity.GetSize() != 0) {
		return false;
	}
	std::string filename;
	filename           = std::string(getenv("GRSISYS")) + "/libraries/TAnalysis/SourceData/";
	std::string symbol = GetSymbol();
	std::transform(symbol.begin(), symbol.end(), symbol.begin(), ::tolower);
	filename.append(symbol);
	filename.append(std::to_string(GetA()));
	filename.append(".sou");
	std::ifstream transfile;
	transfile.open(filename.c_str());
	if(!transfile.is_open()) {
		std::cout<<"failed to open source file: "<<filename.c_str()<<std::endl;
		return false;
	}

	std::string line;

	while(!getline(transfile, line).fail()) {
		if(line.compare(0, 2, "//") == 0) {
			continue;
		}
		if(line.compare(0, 1, "#") == 0) {
			continue;
		}
		double            temp;
		auto*             tran = new TTransition;
		std::stringstream ss(line);
		int               counter = 0;
		while(!(ss >> temp).fail()) {
			counter++;
			if(counter == 1) {
				tran->SetEnergy(temp);
			} else if(counter == 2) {
				tran->SetEnergyUncertainty(temp);
			} else if(counter == 3) {
				tran->SetIntensity(temp);
			} else if(counter == 4) {
				tran->SetIntensityUncertainty(temp);
			} else {
				break;
			}
		}
		AddTransition(tran);
	}

	fTransitionListByIntensity.Sort();

	return true;
}

double TNucleus::GetEnergyFromBeta(double beta)
{
	double gamma = 1 / std::sqrt(1 - beta * beta);
	return fMass * (gamma - 1);
}

double TNucleus::GetBetaFromEnergy(double energy_MeV)
{
	double gamma = energy_MeV / fMass + 1;
	double beta  = std::sqrt(1 - 1 / (gamma * gamma));
	return beta;
}
