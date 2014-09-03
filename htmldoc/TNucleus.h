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
  static const char *massfile;

 public:
  TNucleus(char*);
  TNucleus(int, int, double, const char*);
  TNucleus(int Z, int N, const char* MassFile = massfile);                   //MassFile = "mass.dat");
	
  static void SetMassFile(const char *tmp) {massfile = tmp;}

  void SetZ(int);
  void SetN(int);
  void SetMassExcess(double);  
  void SetMass(double);  
  void SetMass();  
  void SetSymbol(const char*);  
  int GetZ()              {return fZ;}  
  int GetN()              {return fN;}
  int GetA()              {return fN+fZ;}
  double GetMassExcess()  {return fMassExcess;}
  double GetMass()        {return fMass;}
  const char* GetSymbol() {return fSymbol.c_str();}
  double GetRadius();
  int GetZfromSymbol(char*);
 private:
  int fA, fZ, fN;
  double fMass, fMassExcess;
  string fSymbol;

	ClassDef(TNucleus,1);
};
#endif
