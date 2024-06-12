#include "TF1.h"
#include "TMath.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "THStack.h"
#include "TString.h"
#include "TCanvas.h"
#include "TPad.h"
#include "TGraphPainter.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphBentErrors.h"
#include "TGraphAsymmErrors.h"
#include "TMultiGraph.h"
#include "TRootCanvas.h"
#include "TLine.h"
#include "TStyle.h"
#include "TGaxis.h"
#include "TText.h"
#include "TPaveText.h"
#include "TGraphErrors.h"
#include "TLegend.h"
#include "TLegendEntry.h"

#include <iostream>
#include <iomanip>
#include <cmath>
using namespace std;

void PlotScaledQ()
{

   // canvas
   TCanvas* c1 = new TCanvas("ScaledQ", "ScaledQ", 500, 1000);
   c1->Divide(1, 2);
   // in TGraphPainter. Default np=1
   Int_t np = 6;
   gStyle->SetEndErrorSize(np);
   // TPad (from PlotProWaveforms)
   TPad* pada = (TPad*)c1->GetPad(1);
   TPad* padb = (TPad*)c1->GetPad(2);

   pada->SetTopMargin(0.03);
   padb->SetTopMargin(0.03);
   pada->SetBottomMargin(0.2);
   padb->SetBottomMargin(0.2);
   pada->SetLeftMargin(0.16);
   padb->SetLeftMargin(0.16);
   pada->SetRightMargin(0.05);
   padb->SetRightMargin(0.05);
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~PANEL A~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
   c1->cd(1);

   const Int_t dp = 300;
   // x axis
   double xaxis[dp] = {10,
                       20,
                       30,
                       40,
                       50,
                       60,
                       70,
                       80,
                       90,
                       100,
                       110,
                       120,
                       130,
                       140,
                       150,
                       160,
                       170,
                       180,
                       190,
                       200,
                       210,
                       220,
                       230,
                       240,
                       250,
                       260,
                       270,
                       280,
                       290,
                       300,
                       310,
                       320,
                       330,
                       340,
                       350,
                       360,
                       370,
                       380,
                       390,
                       400,
                       410,
                       420,
                       430,
                       440,
                       450,
                       460,
                       470,
                       480,
                       490,
                       500,
                       510,
                       520,
                       530,
                       540,
                       550,
                       560,
                       570,
                       580,
                       590,
                       600,
                       610,
                       620,
                       630,
                       640,
                       650,
                       660,
                       670,
                       680,
                       690,
                       700,
                       710,
                       720,
                       730,
                       740,
                       750,
                       760,
                       770,
                       780,
                       790,
                       800,
                       810,
                       820,
                       830,
                       840,
                       850,
                       860,
                       870,
                       880,
                       890,
                       900,
                       910,
                       920,
                       930,
                       940,
                       950,
                       960,
                       970,
                       980,
                       990,
                       1000,
                       1010,
                       1020,
                       1030,
                       1040,
                       1050,
                       1060,
                       1070,
                       1080,
                       1090,
                       1100,
                       1110,
                       1120,
                       1130,
                       1140,
                       1150,
                       1160,
                       1170,
                       1180,
                       1190,
                       1200,
                       1210,
                       1220,
                       1230,
                       1240,
                       1250,
                       1260,
                       1270,
                       1280,
                       1290,
                       1300,
                       1310,
                       1320,
                       1330,
                       1340,
                       1350,
                       1360,
                       1370,
                       1380,
                       1390,
                       1400,
                       1410,
                       1420,
                       1430,
                       1440,
                       1450,
                       1460,
                       1470,
                       1480,
                       1490,
                       1500,
                       1510,
                       1520,
                       1530,
                       1540,
                       1550,
                       1560,
                       1570,
                       1580,
                       1590,
                       1600,
                       1610,
                       1620,
                       1630,
                       1640,
                       1650,
                       1660,
                       1670,
                       1680,
                       1690,
                       1700,
                       1710,
                       1720,
                       1730,
                       1740,
                       1750,
                       1760,
                       1770,
                       1780,
                       1790,
                       1800,
                       1810,
                       1820,
                       1830,
                       1840,
                       1850,
                       1860,
                       1870,
                       1880,
                       1890,
                       1900,
                       1910,
                       1920,
                       1930,
                       1940,
                       1950,
                       1960,
                       1970,
                       1980,
                       1990,
                       2000,
                       2010,
                       2020,
                       2030,
                       2040,
                       2050,
                       2060,
                       2070,
                       2080,
                       2090,
                       2100,
                       2110,
                       2120,
                       2130,
                       2140,
                       2150,
                       2160,
                       2170,
                       2180,
                       2190,
                       2200,
                       2210,
                       2220,
                       2230,
                       2240,
                       2250,
                       2260,
                       2270,
                       2280,
                       2290,
                       2300,
                       2310,
                       2320,
                       2330,
                       2340,
                       2350,
                       2360,
                       2370,
                       2380,
                       2390,
                       2400,
                       2410,
                       2420,
                       2430,
                       2440,
                       2450,
                       2460,
                       2470,
                       2480,
                       2490,
                       2500,
                       2510,
                       2520,
                       2530,
                       2540,
                       2550,
                       2560,
                       2570,
                       2580,
                       2590,
                       2600,
                       2610,
                       2620,
                       2630,
                       2640,
                       2650,
                       2660,
                       2670,
                       2680,
                       2690,
                       2700,
                       2710,
                       2720,
                       2730,
                       2740,
                       2750,
                       2760,
                       2770,
                       2780,
                       2790,
                       2800,
                       2810,
                       2820,
                       2830,
                       2840,
                       2850,
                       2860,
                       2870,
                       2880,
                       2890,
                       2900,
                       2910,
                       2920,
                       2930,
                       2940,
                       2950,
                       2960,
                       2970,
                       2980,
                       2990,
                       3000};
   double ex[dp]    = {0};
   // Projected range for 20keV beam energy, and straggling for Mylar tape
   // double seria[dp]={545,310,264,250,249,232,238,239,246};
   // double eya[dp]={87.5,42,31,27,24.5,21.5,20.5,19.5,19.5};
   // Scaled quality factors
   double seria[dp] = {0.554626,
                       0.554026,
                       0.553063,
                       0.551768,
                       0.550169,
                       0.548292,
                       0.546165,
                       0.54381,
                       0.541252,
                       0.53851,
                       0.535605,
                       0.532555,
                       0.529377,
                       0.526087,
                       0.5227,
                       0.519229,
                       0.515686,
                       0.512083,
                       0.50843,
                       0.504737,
                       0.501011,
                       0.497261,
                       0.493494,
                       0.489716,
                       0.485933,
                       0.482151,
                       0.478374,
                       0.474606,
                       0.470851,
                       0.467113,
                       0.463395,
                       0.459699,
                       0.456028,
                       0.452383,
                       0.448768,
                       0.445183,
                       0.44163,
                       0.438109,
                       0.434623,
                       0.431172,
                       0.427757,
                       0.424378,
                       0.421036,
                       0.417732,
                       0.414464,
                       0.411235,
                       0.408044,
                       0.40489,
                       0.401775,
                       0.398698,
                       0.395658,
                       0.392656,
                       0.389691,
                       0.386764,
                       0.383874,
                       0.38102,
                       0.378202,
                       0.375421,
                       0.372675,
                       0.369964,
                       0.367288,
                       0.364647,
                       0.36204,
                       0.359466,
                       0.356925,
                       0.354417,
                       0.351941,
                       0.349497,
                       0.347085,
                       0.344703,
                       0.342352,
                       0.340031,
                       0.337739,
                       0.335477,
                       0.333243,
                       0.331038,
                       0.32886,
                       0.32671,
                       0.324587,
                       0.32249,
                       0.32042,
                       0.318375,
                       0.316355,
                       0.314361,
                       0.312391,
                       0.310445,
                       0.308522,
                       0.306623,
                       0.304747,
                       0.302894,
                       0.301063,
                       0.299253,
                       0.297466,
                       0.295699,
                       0.293953,
                       0.292228,
                       0.290523,
                       0.288838,
                       0.287172,
                       0.285526,
                       0.283898,
                       0.282289,
                       0.280699,
                       0.279126,
                       0.277571,
                       0.276034,
                       0.274514,
                       0.273011,
                       0.271524,
                       0.270054,
                       0.2686,
                       0.267162,
                       0.265739,
                       0.264332,
                       0.26294,
                       0.261563,
                       0.2602,
                       0.258853,
                       0.257519,
                       0.256199,
                       0.254893,
                       0.253601,
                       0.252323,
                       0.251057,
                       0.249804,
                       0.248565,
                       0.247337,
                       0.246123,
                       0.24492,
                       0.24373,
                       0.242552,
                       0.241385,
                       0.24023,
                       0.239086,
                       0.237953,
                       0.236832,
                       0.235721,
                       0.234621,
                       0.233532,
                       0.232453,
                       0.231385,
                       0.230327,
                       0.229278,
                       0.22824,
                       0.227211,
                       0.226192,
                       0.225182,
                       0.224182,
                       0.223191,
                       0.222209,
                       0.221236,
                       0.220272,
                       0.219316,
                       0.218369,
                       0.217431,
                       0.216501,
                       0.215579,
                       0.214666,
                       0.21376,
                       0.212863,
                       0.211973,
                       0.211091,
                       0.210217,
                       0.20935,
                       0.208491,
                       0.207639,
                       0.206794,
                       0.205956,
                       0.205126,
                       0.204302,
                       0.203486,
                       0.202676,
                       0.201873,
                       0.201076,
                       0.200287,
                       0.199503,
                       0.198726,
                       0.197956,
                       0.197191,
                       0.196433,
                       0.195681,
                       0.194935,
                       0.194194,
                       0.19346,
                       0.192732,
                       0.192009,
                       0.191292,
                       0.19058,
                       0.189874,
                       0.189174,
                       0.188479,
                       0.187789,
                       0.187104,
                       0.186425,
                       0.185751,
                       0.185082,
                       0.184418,
                       0.183759,
                       0.183105,
                       0.182456,
                       0.181812,
                       0.181172,
                       0.180538,
                       0.179907,
                       0.179282,
                       0.178661,
                       0.178045,
                       0.177433,
                       0.176825,
                       0.176222,
                       0.175623,
                       0.175028,
                       0.174438,
                       0.173852,
                       0.173269,
                       0.172691,
                       0.172117,
                       0.171548,
                       0.170982,
                       0.170419,
                       0.169861,
                       0.169307,
                       0.168756,
                       0.16821,
                       0.167666,
                       0.167127,
                       0.166591,
                       0.166059,
                       0.165531,
                       0.165006,
                       0.164484,
                       0.163966,
                       0.163451,
                       0.16294,
                       0.162432,
                       0.161928,
                       0.161426,
                       0.160928,
                       0.160433,
                       0.159942,
                       0.159453,
                       0.158968,
                       0.158486,
                       0.158007,
                       0.157531,
                       0.157058,
                       0.156588,
                       0.15612,
                       0.155656,
                       0.155195,
                       0.154737,
                       0.154281,
                       0.153828,
                       0.153378,
                       0.152931,
                       0.152487,
                       0.152045,
                       0.151606,
                       0.15117,
                       0.150736,
                       0.150305,
                       0.149876,
                       0.14945,
                       0.149027,
                       0.148606,
                       0.148188,
                       0.147772,
                       0.147358,
                       0.146947,
                       0.146539,
                       0.146132,
                       0.145729,
                       0.145327,
                       0.144928,
                       0.144531,
                       0.144137,
                       0.143744,
                       0.143354,
                       0.142966,
                       0.142581,
                       0.142197,
                       0.141816,
                       0.141437,
                       0.14106,
                       0.140685,
                       0.140312,
                       0.139941,
                       0.139573,
                       0.139206,
                       0.138842,
                       0.138479,
                       0.138118,
                       0.13776,
                       0.137403,
                       0.137048,
                       0.136696,
                       0.136345,
                       0.135996,
                       0.135649,
                       0.135304};
   double eya[dp]   = {};

   TGraphErrors* plota1 = new TGraphErrors(dp, xaxis, seria, ex, eya);

   // Att
   plota1->SetLineColor(8);
   plota1->SetMarkerStyle(20);
   plota1->SetMarkerColor(8);
   plota1->SetMarkerSize(1);
   plota1->SetLineWidth(2);
   // TMultiGraph
   TMultiGraph* mga = new TMultiGraph();
   mga->Add(plota1);
   mga->Draw("apl");
   // Att
   mga->GetXaxis()->SetTitle("Energy (keV)");
   mga->GetXaxis()->CenterTitle();
   mga->GetXaxis()->SetTitleFont(43);
   mga->GetXaxis()->SetTitleSize(22);
   mga->GetXaxis()->SetTitleOffset(2.5);
   //
   mga->GetYaxis()->CenterTitle();
   mga->GetYaxis()->SetTitleOffset(3.6);
   mga->GetYaxis()->SetTitleFont(43);
   mga->GetYaxis()->SetTitleSize(22);
   mga->GetYaxis()->SetTitle("Quality Factor");
   //
   mga->GetXaxis()->SetLabelFont(43);
   mga->GetXaxis()->SetLabelSize(24);
   mga->GetXaxis()->SetRangeUser(0, 3000);
   mga->GetXaxis()->SetNdivisions(6, kTRUE);
   //
   mga->GetYaxis()->SetLabelFont(43);
   mga->GetYaxis()->SetLabelSize(20);
   mga->GetYaxis()->SetRangeUser(0, 0.75);
   mga->GetYaxis()->SetNdivisions(9, kTRUE);
   // mga->GetYaxis()->SetNoExponent(kTRUE);
   pada->Modified();
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~PANEL B~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
   c1->cd(2);

   // data
   const Int_t dp2 = 9;
   // x axis
   double xaxis2[dp] = {10, 20, 30, 40, 50, 60, 70, 80, 90};
   double ex2[dp]    = {0};
   // Projected range for 20keV beam energy, and straggling for Aluminized Mylar tape
   double seric[dp] = {378, 211, 173, 160, 156, 145, 148, 148, 152};
   double eyc[dp]   = {93.5, 44, 31, 26, 23, 20, 19, 18, 17.5};
   // Projected range for 60keV beam energy, and straggling for Aluminized Mylar tape
   double serid[dp] = {1081, 550, 410, 351, 324, 287, 283, 276, 277};
   double eyd[dp]   = {425, 199, 132, 102, 85, 70, 65, 60, 58};

   TGraphErrors* plotb1 = new TGraphErrors(dp2, xaxis2, seric, ex2, eyc);
   TGraphErrors* plotb2 = new TGraphErrors(dp2, xaxis2, serid, ex2, eyd);

   // Att
   plotb1->SetLineColor(8);
   plotb1->SetMarkerStyle(20);
   plotb1->SetMarkerColor(8);
   plotb1->SetMarkerSize(1);
   plotb1->SetLineWidth(2);
   plotb2->SetLineColor(9);
   plotb2->SetMarkerStyle(22);
   plotb2->SetMarkerColor(9);
   plotb2->SetMarkerSize(1);
   plotb2->SetLineWidth(2);
   // TMultiGraph
   TMultiGraph* mgb = new TMultiGraph();
   mgb->Add(plotb1);
   mgb->Add(plotb2);
   mgb->Draw("apl");
   //
   mgb->GetXaxis()->SetTitle("Beam Z");
   mgb->GetXaxis()->CenterTitle();
   mgb->GetXaxis()->SetTitleFont(43);
   mgb->GetXaxis()->SetTitleSize(22);
   mgb->GetXaxis()->SetTitleOffset(2.5);
   //
   mgb->GetYaxis()->CenterTitle();
   mgb->GetYaxis()->SetTitleOffset(2.0);
   mgb->GetYaxis()->SetTitleFont(43);
   mgb->GetYaxis()->SetTitleSize(22);
   mgb->GetYaxis()->SetTitle("Projected Range (#aa)");
   //
   mgb->GetXaxis()->SetLabelFont(43);
   mgb->GetXaxis()->SetLabelSize(24);
   mgb->GetXaxis()->SetRangeUser(0, 100);
   mgb->GetXaxis()->SetNdivisions(110, kTRUE);
   //
   mgb->GetYaxis()->SetLabelFont(43);
   mgb->GetYaxis()->SetLabelSize(20);
   mgb->GetYaxis()->SetRangeUser(0, 1900);
   mgb->GetYaxis()->SetNdivisions(9, kTRUE);
   // mgb->GetYaxis()->SetNoExponent(kTRUE);
   //
   padb->Modified();
   //~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~

   // TPaveText
   TPaveText* pta = new TPaveText(0.9600, 0.8500, 0.4700, 0.9800, "blNDC");
   TPaveText* ptb = new TPaveText(0.9600, 0.8500, 0.4700, 0.9800, "blNDC");
   //
   pta->SetBorderSize(0);
   pta->SetFillColor(0);
   pta->SetFillStyle(0);
   pta->SetTextFont(43);
   pta->SetTextSize(24);
   TText* texta = pta->AddText("(a)");
   ptb->SetBorderSize(0);
   ptb->SetFillColor(0);
   ptb->SetFillStyle(0);
   ptb->SetTextFont(43);
   ptb->SetTextSize(24);
   TText* textb = ptb->AddText("(b)");
   //
   c1->cd(1);
   pta->Draw();
   c1->cd(2);
   ptb->Draw();
   c1->Modified();
}
