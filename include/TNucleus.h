#ifndef __TNUCLEUS_H
#define __TNUCLEUS_H

#include <iostream>
#include <math.h>
#include <string.h>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <string>

#include "TNamed.h"

using namespace std;

class TNucleus : public TNamed{

 private:
  static const char *massfile; //The massfile to be used, which includes Z, N, atomic symbol, and mass excess

 public:
  TNucleus(char* symbol);
  TNucleus(int Z, int N, double mass, const char* symbol);
  TNucleus(int Z, int N, const char* MassFile = massfile);
	
  static void SetMassFile(const char *tmp) {massfile = tmp;} //Sets the mass file to be used. Default is mass.dat (see source code)

  void SetZ(int);					
  void SetN(int);					
  void SetMassExcess(double);  				
  void SetMass(double);  				
  void SetMass();  					
  void SetSymbol(const char*);  			
  int GetZ()              {return fZ;}  		// Gets the Z (# of protons) of the nucleus
  int GetN()              {return fN;}			// Gets the N (# of neutrons) of the nucleus
  int GetA()              {return fN+fZ;}		// Gets the A (Z + N) of the nucleus
  double GetMassExcess()  {return fMassExcess;}		// Gets the mass excess of the nucleus (in MeV)
  double GetMass()        {return fMass;}		// Gets the mass of the nucleus (in MeV)
  const char* GetSymbol() {return fSymbol.c_str();}	// Gets the atomic symbol of the nucleus
  double GetRadius();					
  int GetZfromSymbol(char*);				
 private:
  int fA; 						// Number of nucleons (Z + N) 
  int fN;						// Number of neutrons (N)
  int fZ;						// Number of protons (Z)
  double fMass;						// Mass (in MeV)
  double fMassExcess;					// Mass excess (in MeV)
  string fSymbol;					// Atomic symbol (ex. Ba, C, O, N)

	ClassDef(TNucleus,1);				// Creates a nucleus with corresponding nuclear information
};
#endif
