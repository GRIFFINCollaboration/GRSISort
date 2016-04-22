#ifndef GROOTGLOBALS__H
#define GROOTGLOBALS__H

/** \addtogroup GROOT
 *  @{
 */

#include "TH2.h"
#include "TH1.h"

TH1D *ProjectionX(TH2 *mat,int lowbin=0,int highbin=-1); /*MENU*/
TH1D *ProjectionY(TH2 *mat,int lowbin=0,int highbin=-1); /*MENU*/
TH1D *ProjectionX(TH2 *mat,double lowvalue,double highvalue=-1); /*MENU*/
TH1D *ProjectionY(TH2 *mat,double lowvalue,double highvalue=-1); /*MENU*/

void SaveAll(const char* fname,Option_t *opt="recreate");

int  PeakSearch(TH1 *hst,double sigma=2.0,double thresh=0.01,Option_t *opt = "");
bool ShowPeaks(TH1 **hists,unsigned int Nhists=1);
bool RemovePeaks(TH1 **hists,unsigned int Nhists=1);

TH1 *GrabHist();

void Help();
void Commands();
void Prompt();
/*! @} */
#endif
