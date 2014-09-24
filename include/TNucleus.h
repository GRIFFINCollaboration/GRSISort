#ifndef __TNUCLEUS_H
#define __TNUCLEUS_H

#include <iostream>
#include <math.h>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <string>

#include <TGRSITransition.h>

#include "TObject.h"
#include "TNamed.h"
#include "TList.h"

using namespace std;




class TNucleus : public TNamed{

 private:
  static const char *massfile;

 public:
  TNucleus(const char*);					// Creates a nucleus based on symbol and sets all parameters from mass.dat
  TNucleus(int, int, double, const char*);		// Creates a nucleus with Z, N, mass, and symbol
  TNucleus(int Z, int N, const char* MassFile = massfile); // Creates a nucleus with Z, N using mass table (default MassFile = "mass.dat")
	
  static void SetMassFile(const char *tmp = NULL);// {massfile = tmp;} //Sets the mass file to be used

  void SetZ(int);					// Sets the Z (# of protons) of the nucleus
  void SetN(int);					// Sets the N (# of neutrons) of the nucleus
  void SetMassExcess(double);  				// Sets the mass excess of the nucleus (in MeV)
  void SetMass(double);  				// Sets the mass manually (in MeV)
  void SetMass();  					// Sets the mass based on the A and mass excess of nucleus (in MeV)
  void SetSymbol(const char*);  			// Sets the atomic symbol for the nucleus
  int GetZ()              {return fZ;}  		// Gets the Z (# of protons) of the nucleus
  int GetN()              {return fN;}			// Gets the N (# of neutrons) of the nucleus
  int GetA()              {return fN+fZ;}		// Gets the A (Z + N) of the nucleus
  double GetMassExcess()  {return fMassExcess;}		// Gets the mass excess of the nucleus (in MeV)
  double GetMass()        {return fMass;}		// Gets the mass of the nucleus (in MeV)
  const char* GetSymbol() {return fSymbol.c_str();}	// Gets the atomic symbol of the nucleus
  double GetRadius();					// Gets the radius of the nucleus (in fm)
  int GetZfromSymbol(char*);				// Figures out the Z of the nucleus based on the atomic symbol




  TList TransitionList;
  bool SetSourceData();


private:
  int fA, fZ, fN;
  double fMass, fMassExcess;
  string fSymbol;

	ClassDef(TNucleus,1);
};



#endif

