
#include "Globals.h"

#include <TClass.h>
#include <TPaveStats.h>
#include <TList.h>
#include <TText.h>
#include <TLatex.h>
#include <TH1.h>
#include <Buttons.h>
#include <KeySymbols.h> 

#include "GCanvas.h"

#include <iostream>

int GCanvas::lastx = 0;
int GCanvas::lasty = 0;

GCanvas::GCanvas(Bool_t build)
        :TCanvas(build)  {  
   GCanvasInit();
}


GCanvas::GCanvas(const char* name, const char* title, Int_t form)
        :TCanvas(name,title,form) { 
   GCanvasInit();

}


GCanvas::GCanvas(const char* name, const char* title, Int_t ww, Int_t wh)
        :TCanvas(name,title,ww,wh) { 
   GCanvasInit();

}


GCanvas::GCanvas(const char* name, Int_t ww, Int_t wh, Int_t winid)
        :TCanvas(name,ww,wh,winid) { 
   GCanvasInit();

}


GCanvas::GCanvas(const char* name, const char* title, Int_t wtopx, Int_t wtopy, Int_t ww, Int_t wh)
        :TCanvas(name,title,wtopx,wtopy,ww,wh) { 
   GCanvasInit();
}


GCanvas::~GCanvas() {
   //TCanvas::~TCanvas();           
}

void GCanvas::GCanvasInit() {
   printf("GCanvasInit called.\n");
//   this->SetCrosshair(true);
   //TQObject::Connect("TCanvas", "HandleInput(Int_t,Int_t,Int_t)", "GCanvas",this,"CatchEvent(Int_t,Int_t,Int_t)");
   TQObject::Connect("TCanvas", "ProcessedEvent(Int_t,Int_t,Int_t,TObject*)", "GCanvas",this,"CatchEvent(Int_t,Int_t,Int_t,TObject*)");
}


//void GCanvas::ProcessEvent(Int_t event,Int_t x,Int_t y,TObject *obj) {
//   printf("{GCanvas} ProcessEvent:\n");
//   printf("\tevent: \t0x%08x\n",event);
//   printf("\tobject:\t0x%08x\n",obj);
//   printf("\tx:     \t0x%i\n",x);
//   printf("\ty:     \t0x%i\n",y);
//}


void GCanvas::CatchEvent(Int_t event,Int_t x,Int_t y,TObject *obj) {
   printf("{GCanvas} CatchEvent:\n");
   printf("\tevent: \t0x%08x\n",event);
   printf("\tobject:\t0x%08x",obj);
   //if(obj) {
   //   printf("\t%s\n",obj->IsA()->GetName());
   //   if(x != lastx) {
   //      lastx = x;
   //      UpdateStatsInfo(x,y);
   //   }
   //}
   //else
   //   printf("\n");
   //printf("\tx:     \t%i\n",x);
   //printf("\ty:     \t%i\n",y);

   switch(event) {
      //case kArrowKeyPress:
      case kKeyPress: 
         HandleKeyPress(event,x,y,0);
         break;
       default:
//         TCanvas::HandleInput((EEventType)event,x,y);
         return;;
   };
//   TCanvas::ProcessedEvent(event,x,y,0);
   return;
}

void GCanvas::ExecuteEvent(Int_t event,Int_t x,Int_t y) { 
  printf("exc event called.\n");
}

void GCanvas::HandleInput(EEventType event,Int_t x,Int_t y) {
   //this->SetEditable(false);
   printf(DRED);
   printf("{GCanvas} HandleEvent:\n");
   printf(DYELLOW "\tevent: \t0x%08x\n" DRED,event);
   if(this->GetSelected())
      printf("\tfselected found[0x%08x]\t %s\n",this->GetSelected(),this->GetSelected()->GetName());
   printf(DYELLOW);
   printf(RESET_COLOR);
   gPad->SetEditable(false);
   //If the below switch breaks. You need to upgrade your version of ROOT
   //Version 5.34.24 works.
   switch(event) {
      case kArrowKeyPress:
      case kArrowKeyRelease:
      case kKeyPress: 
      case kKeyRelease:
  //       this->SetEditable(true);
 //        HandleKeyPress(x,y,this->GetSelected());
 //        break;
       default:
         printf(RED"\t\tHANDLE DEFAULT!" RESET_COLOR "\n");
         TCanvas::HandleInput(event,x,y);
         gPad->SetEditable(true);
         return;
   };
   TCanvas::ProcessedEvent(event,x,y,this->GetSelected());
   //this->SetEditable(false);
   gPad->SetEditable(true);
   return;
}


void GCanvas::UpdateStatsInfo(int x, int y) {
   TIter next(this->GetListOfPrimitives());
   TObject *obj;
   while(obj=next()) {
      if(obj->InheritsFrom("TH1")) {
         ((TH1*)obj)->SetBit(TH1::kNoStats);
         printf("found : %s\n",obj->GetName());
         TPaveStats *st = (TPaveStats*)((TH1*)obj)->GetListOfFunctions()->FindObject("stats");
         st->GetListOfLines()->Delete();
         st->AddText(Form("X      %i",x));
         st->AddText(Form("Counts %i",y));
         
         //st->Paint();
         //gPad->Modified();
         //gPad->Update();
      }
   }
}

void GCanvas::HandleKeyPress(int event,int x,int key,TObject *obj) {
   printf(DBLUE);
   //printf("\tevent  \t%i\n",this->GetEvent());
   std::cout << "\tevent  \t" << this->GetEvent() << "\n";
   printf("\tfsel:  \t%s\n",this->GetSelected()->GetName());
   printf("\tx:     \t%i\n",x);
   printf("\tkey:   \t%i\n",key);
   printf(RESET_COLOR);
   switch(key) {
      case kKey_Up:
         printf("UP!\n");    
         if(obj)
            printf("\tobj = %s\n",obj->GetName());
         break;
      case kKey_Down:
         printf("DOWN!\n");         
         if(obj)
            printf("\tobj = %s\n",obj->GetName());
         break;
      case kKey_Right:
         printf("RIGHT!\n");         
         if(obj)
            printf("\tobj = %s\n",obj->GetName());
         break;
      case kKey_Left:
         printf("LEFT!\n");         
         if(obj)
            printf("\tobj = %s\n",obj->GetName());
         break;
   };
}

void GCanvas::Draw(Option_t *opt) {
   printf("GCanvas Draw was called.\n");
   TCanvas::Draw(opt);
   //this->FindObject("TFrame")->SetBit(kCannotMove);
}



