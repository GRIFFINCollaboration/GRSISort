#ifndef GROOTGLOBALS__H
#define GROOTGLOBALS__H

#include <TH2.h>

TH1D *ProjectionX(TH2 *mat,int lowbin=0,int highbin=-1); /*MENU*/
TH1D *ProjectionY(TH2 *mat,int lowbin=0,int highbin=-1); /*MENU*/
TH1D *ProjectionX(TH2 *mat,double lowvalue,double highvalue=-1); /*MENU*/
TH1D *ProjectionY(TH2 *mat,double lowvalue,double highvalue=-1); /*MENU*/

void Help();
void Commands();
void Prompt();

#endif
