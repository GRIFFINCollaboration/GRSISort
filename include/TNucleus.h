#ifndef TNUCLEUS_H
#define TNUCLEUS_H

#include <iostream>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <cstdlib>
#include <fstream>
#include <string>

#include "TTransition.h"

#include "TObject.h"
#include "TNamed.h"
#include "TSortedList.h"

/////////////////////////////////////////////////////////////////
///
/// \class TNucleus
///
/// This class builds a nucleus and sets all the basic information
/// (mass, Z, symbol, radius, etc.)
///
/////////////////////////////////////////////////////////////////

class TNucleus : public TNamed {

private:
	static const char*  grsipath;
	static std::string& massfile(); ///< The massfile to be used, which includes Z, N, atomic symbol, and mass excess

public:
	TNucleus() {};               ///< Should not be used, here so we can write things to a root file.
	TNucleus(const char* name); ///< Creates a nucleus based on symbol and sets all parameters from mass.dat
	TNucleus(int charge, int neutrons, double mass, const char* symbol); ///< Creates a nucleus with Z, N, mass, and symbol
	TNucleus(int charge, int neutrons,	const char* MassFile = nullptr); ///< Creates a nucleus with Z, N using mass table (default MassFile = "mass.dat")

	~TNucleus() override;

	static void ParseName(const char* input, std::string& symbol, int& number, std::string& element) { std::string tmp(input); return ParseName(tmp, symbol, number, element); }
	static void ParseName(std::string input, std::string& symbol, int& number, std::string& element);
	static std::string SortName(const char* input) { std::string tmp(input); return SortName(tmp); }
	static std::string SortName(std::string input);
	void SetZ(int);              ///< Sets the Z (# of protons) of the nucleus
	void SetN(int);              ///< Sets the N (# of neutrons) of the nucleus
	void SetMassExcess(double);  ///< Sets the mass excess of the nucleus (in MeV)
	void SetMass(double);        ///< Sets the mass manually (in MeV)
	void SetMass();              ///< Sets the mass based on the A and mass excess of nucleus (in MeV)
	void SetSymbol(const char*); ///< Sets the atomic symbol for the nucleus

	void AddTransition(Double_t energy, Double_t intensity, Double_t energy_uncertainty = 0.0, Double_t intensity_uncertainty = 0.0);
	void AddTransition(TTransition* tran);

	int         GetZ() const { return fZ; }                   ///< Gets the Z (# of protons) of the nucleus
	int         GetN() const { return fN; }                   ///< Gets the N (# of neutrons) of the nucleus
	int         GetA() const { return fN + fZ; }              ///< Gets the A (Z + N) of the nucleus
	double      GetMassExcess() const { return fMassExcess; } ///< Gets the mass excess of the nucleus (in MeV)
	double      GetMass() const { return fMass; }             ///< Gets the mass of the nucleus (in MeV)
	const char* GetSymbol() const { return fSymbol.c_str(); } ///< Gets the atomic symbol of the nucleus

	// Returns total kinetic energy in MeV
	double GetEnergyFromBeta(double beta);
	double GetBetaFromEnergy(double energy_MeV);

	TTransition* GetTransition(Int_t idx) { return GetTransitionByIntensity(idx); }
	TTransition* GetTransitionByIntensity(Int_t idx);
	TTransition* GetTransitionByEnergy(Int_t idx);

	Int_t  NTransitions() const { return fTransitionListByIntensity.GetSize(); };
	Int_t  GetNTransitions() const { return fTransitionListByIntensity.GetSize(); };
	double GetRadius() const;
	int    GetZfromSymbol(char*);

	void Print(Option_t* opt = "") const override;
	void WriteSourceFile(const std::string& outfilename = "");

	const TSortedList* GetTransitionList() const { return GetTransitionListByIntensity(); }
	const TSortedList* GetTransitionListByIntensity() const { return &fTransitionListByIntensity; }
	const TSortedList* GetTransitionListByEnergy() const { return &fTransitionListByEnergy; }

	bool operator==(const TNucleus& rhs) const { return ((fA == rhs.fA) && (fN == rhs.fN) && (fZ == rhs.fZ)); }
	bool operator!=(const TNucleus& rhs) const { return !(*this == rhs); }

private:
	void SetName(const char* c = "") override;

	int         fA{0};          ///< Number of nucleons (Z + N)
	int         fN{0};          ///< Number of neutrons (N)
	int         fZ{0};          ///< Number of protons (Z)
	double      fMass{0.};      ///< Mass (in MeV)
	double      fMassExcess{0.};///< Mass excess (in MeV)
	std::string fSymbol;        ///< Atomic symbol (ex. Ba, C, O, N)

	TSortedList fTransitionListByIntensity;
	TSortedList fTransitionListByEnergy;
	bool  LoadTransitionFile();

   /// \cond CLASSIMP
	ClassDefOverride(TNucleus, 2); // Creates a nucleus with corresponding nuclear information
   /// \endcond
};

#endif
