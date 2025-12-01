#include <iostream>
#include "TStyle.h"

void grsi_logon()
{
   std::cout << "setting GRSISort environment ... " << std::flush;

   gStyle->Reset();
   //gStyle->SetFillColor(kWhite);
   gStyle->SetPalette(55);
   gStyle->SetPadColor(kWhite);
   gStyle->SetCanvasColor(kWhite);
   gStyle->SetStatColor(kWhite);
   gStyle->SetTitleColor(kWhite, "T");
   gStyle->SetTitleBorderSize(1);
   gStyle->SetTitleAlign(23);   // reference of title, default is 13 = left top, 23 = middle top?
   gStyle->SetTitleX(0.5);      //centered title
   gStyle->SetStatBorderSize(1);
   gStyle->SetFrameFillStyle(4000);
   gStyle->SetHistFillStyle(4000);
   //to turn off highlighting of active pad/frame/canvas
   //gStyle->SetPadBorderMode(0);
   //gStyle->SetFrameBorderMode(0);
   //gStyle->SetCanvasBorderMode(0);
   gStyle->SetOptStat("neuoi");

   std::cout << "done" << std::endl;
}
