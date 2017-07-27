#ifndef POSITION_H
#define POSITION_H

#include "TVector3.h"
#include <iostream>
#include "TMath.h"

class Detector {
public:
   Detector();
   ~Detector();

   TVector3 fPosition;
   TVector3 fShift[5]; // The difference between the crystal center and detector center

   void SetThetaPhi(Double_t, Double_t);

   enum Colour { kBlue = 0, kGreen = 1, kRed = 2, kWhite = 3, kCenter = 4 };
};

class Position {
public:
   Position();
   ~Position(){};

   Detector detector[16];

   TVector3 SetPosition(UShort_t, UShort_t, double);
};

#endif
