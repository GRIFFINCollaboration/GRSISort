#include "Position.h"

//g++ Position.cxx -c -oPosition `root-config --cflags --libs`


Detector::Detector(){}

Detector::~Detector(){}

void Detector::SetThetaPhi(Double_t theta, Double_t phi){

  theta*=TMath::Pi()/180.0;
  phi*=TMath::Pi()/180.0;

  Double_t cp = 2.0; //Crystal Center Point Might have adepth eventually
//Set Theta's of the center of each DETECTOR face
//Define one Detector position

  fShift[kBlue].SetXYZ(-cp,cp,0);
  fShift[kGreen].SetXYZ(cp,cp,0);
  fShift[kRed].SetXYZ(cp,-cp,0);
  fShift[kWhite].SetXYZ(-cp,-cp,0);

//This sets the Theta and Phi for a detector by setting the central angle and transforming the shitft vectors
   if(fPosition.Mag() < 0.001){ //I need to set the magnitude to some value (Unity?) if it is currently 0.
      fPosition.SetZ(1.00);
   }

   fPosition.SetTheta(theta);
   fPosition.SetPhi(phi);

  for(int i=0;i <4; i++){
     fShift[i].RotateY(theta);
     fShift[i].RotateZ(phi);
  } 
}
/*
void Detector::SetTheta(Double_t theta){
  for(int i=0,i<5,i++){
     fPosition[i].Rotate

*/


Position::Position(){

//Might have to inisialize a magnitude for anything to work here
//Downstream Lampshde 
   detector[0].SetThetaPhi(45.0,67.5);
   detector[1].SetThetaPhi(45.0,157.5); 
   detector[2].SetThetaPhi(45.0,247.5);
   detector[3].SetThetaPhi(45.0,337.5);


//Corona
   detector[4].SetThetaPhi(90.0,22.5);
   detector[5].SetThetaPhi(90.0,67.5);
   detector[6].SetThetaPhi(90.0,112.5);
   detector[7].SetThetaPhi(90.0,157.5);
   detector[8].SetThetaPhi(90.0,202.5);
   detector[9].SetThetaPhi(90.0,247.5);
   detector[10].SetThetaPhi(90.0,292.5);
   detector[11].SetThetaPhi(90.0,337.5);


//Upstream Lampshade
   detector[12].SetThetaPhi(135.0,67.5);
   detector[13].SetThetaPhi(135.0,157.5);
   detector[14].SetThetaPhi(135.0,247.5);
   detector[15].SetThetaPhi(135.0,337.5);

}







TVector3 Position::SetPosition(UShort_t det, UShort_t crystal = Detector::kCenter, Double_t dist = 11.0){

   detector[det].fPosition.SetMag(dist);

   if(crystal == Detector::kCenter)
      return detector[det].fPosition;

   return detector[det].fPosition + detector[det].fShift[crystal]; 

}



int main(){

   Position grifposition;
   double differences[64][64] = {0}; 
 

   for(int i = 0; i < 16; i++){//Loop over first detector
      for(int u = 0; u < 4; u++){//Loop over crystals
         for(int j=0;j<16;j++){//Loop over second detector 
            for(int v=0;v<4;v++){//Loop over second crystal
               differences[j*4+v][i*4+u] = (grifposition.SetPosition(j,v)).Angle(grifposition.SetPosition(i,u))*180.0/TMath::Pi(); 
            }       
   
         }
      }


   }

//  grifposition.detector[4].fPosition.Print();
//  grifposition.detector[8].fPosition.Print();

      std::cout << "\t";
   for(int i = 0; i<64;i++)
      std::cout << i<<"\t";   

   std::cout << std::endl;

   for(int j = 0; j <64;j++){
      std::cout << j <<"\t";
      for(int i = 0; i<64;i++){
         std::cout << differences[j][i]<<"\t";
      }
    std::cout << std::endl;
   }
  

   //Work on outputting this matrix tomorrow to file to compare against GEANT


}
















