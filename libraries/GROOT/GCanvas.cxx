
#include <TClass.h>
#include <TPaveStats.h>
#include <TList.h>
#include <TText.h>
#include <TLatex.h>
#include <TH1.h>
#include <Buttons.h>
#include <KeySymbols.h> 

#include "GCanvas.h"

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


}


//void GCanvas::ProcessEvent(Int_t event,Int_t x,Int_t y,TObject *obj) {
//   printf("{GCanvas} ProcessEvent:\n");
//   printf("\tevent: \t0x%08x\n",event);
//   printf("\tobject:\t0x%08x\n",obj);
//   printf("\tx:     \t0x%i\n",x);
//   printf("\ty:     \t0x%i\n",y);
//}


void GCanvas::CatchEvent(Int_t event,Int_t x,Int_t y,TObject *obj) {
   //printf("{GCanvas} CatchEvent:\n");
   //printf("\tevent: \t0x%08x\n",event);
   //printf("\tobject:\t0x%08x",obj);
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
      case kKeyPress: 
         HandleKeyPress(x,y,obj);
         break;

   };
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

void GCanvas::HandleKeyPress(int x,int key,TObject *obj) {

   //printf("\tx:     \t%i\n",x);
   //printf("\ty:     \t%i\n",y);
   switch(key) {
      case kKey_Up:
         printf("UP!\n");         
         printf("\tobj = %s\n",obj->GetName());

         break;
      case kKey_Down:
         printf("DOWN!\n");         
         printf("\tobj = %s\n",obj->GetName());
         break;
      case kKey_Right:
         printf("RIGHT!\n");         
         printf("\tobj = %s\n",obj->GetName());
         break;
      case kKey_Left:
         printf("LEFT!\n");         
         printf("\tobj = %s\n",obj->GetName());
         break;

   };
}
