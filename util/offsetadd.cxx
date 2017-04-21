//g++ offsetadd.cxx `root-config --cflags --libs` -I${GRSISYS}/include -L${GRSISYS}/libraries -lMidasFormat -lXMLParser  -ooffsetadd


#include<TMidasFile.h>
#include<TMidasEvent.h>

#include <iostream>

//void WriteEventToFile(TMidasFile*,std::shared_ptr<TMidasEvent>,Option_t);

void ProcessEvent(std::shared_ptr<TMidasEvent>,TMidasFile*);

int main(int argc, char **argv) {

   if(argc!=3) {
      printf("Usage: ./offsetadd <input.mid> <output.mid>\n");
      return 1;
   }

   TMidasFile *infile  = new TMidasFile;
   TMidasFile *outfile = new TMidasFile;
	std::shared_ptr<TMidasEvent> event = std::make_shared<TMidasEvent>();
   
   if(argv[1] == argv[2]){
      printf("ERROR: Cannot overwrite midas file %s\n",argv[1]);
   }

   infile->Open(argv[1]);
   outfile->OutOpen(argv[2]);

   while(infile->Read(event)>0) {
      switch(event->GetEventId()) {
         case 0x8000:
            printf("start of run\n");
            outfile->Write(event);
            event->Print();
            break;
         case 0x8001:
            printf("end of run\n");
            event->Print();
            outfile->Write(event);
            break;
         default:
            ProcessEvent(event,outfile);
            break;
       };
   }
   infile->Close();
   outfile->Close();
   return 0;
}


//void WriteEventToFile(TMidasFile *outfile,std::shared_ptr<TMidasEvent> event) {
//   outfile->Write(event);
//}


void ProcessEvent(std::shared_ptr<TMidasEvent> event,TMidasFile *outfile) {
   if(event->GetEventId() !=1 ) {
      outfile->Write(event,"q");
      return;
   }
   event->SetBankList();
   //int size;
   //int data[1024];
  
   void *ptr;
   int banksize = event->LocateBank(nullptr,"GRF1",&ptr);

   uint32_t type  = 0xffffffff;
   int value = 0xffffffff;

   int dettype = 0;
   int chanadd = 0;

   int timelow  = 0;
   int timehigh = 0;

   int64_t time = 0;

   for(int x=0;x<banksize;x++) {
      value = *((int*)ptr+x);
      type  = value & 0xf0000000; 

      switch(type) {
         case 0x80000000:
            dettype = value & 0x0000000f;
            chanadd = (value &0x0003fff0)>> 4;
            break;
         case 0xa0000000:
            timelow = value & 0x0fffffff;
            break;
         case 0xb0000000:
            timehigh = value & 0x00003fff;
            break;

      };
   }

   //printf("chanadd = 0x%08x
   //event->Print("a");

   //printf("dettype  = 0x%08x\n",dettype);
   //printf("chanadd  = 0x%08x\n",chanadd);
                                    
   //printf("timelow  = 0x%08x\n",timelow);
   //printf("timehigh = 0x%08x\n",timehigh);



   if( (dettype==1) || (dettype ==5) ) { // 1 for GRIFFIN, 5 for PACES
     //do nothing.
   } else {
    outfile->Write(event,"q");
    return;
   }
 
   if(((chanadd&0x0000ff00) == 0x00000000) ||
      ((chanadd&0x0000ff00) == 0x00001000) ||
      ((chanadd&0x0000ff00) == 0x00001100) ||
      ((chanadd&0x0000ff00) == 0x00001200)) {
      //do nothing.
    } else {
      outfile->Write(event,"q");
      return;
    }

   time = timehigh;
   time = time << 28;
   time |= timelow &0x0fffffff;

   //printf("time = 0x%016x\n",time);
   //std::cout << "time    = " << std::hex << time << std::endl;

   // Here's where we change the values of the time stamps!!!!
   switch(chanadd&0x0000ff00) {
      case 0x00000000: // if the first GRIF-16
//         time -= 10919355323; // run 2369 correction
         time -= 87; // run 2394 correction
         break;
      case 0x00000100: // if the second GRIF-16
         break;
      case 0x00001000: // if the third GRIF-16
//         time -= 10919355323; // run 2369 correction
         break;
      case 0x00001100: // if the fourth GRIF-16
//         time -= 10919355239; // run 2369 correction
         break;
      case 0x00001200: // if the fifth GRIF-16
//         time += 7;
         time -= 87; // run 2394 correction
         break;
   };
   if(time<0)
      time += 0x3ffffffffff;
   else if(time>0x3ffffffffff)
      time -= 0x3ffffffffff;

   // moving these inside the next switch, to account for doubly printed words.
   // (hey, it happens.)
   // -JKS, 14 January 2015
   //timelow = time&0x0fffffff;
   //timehigh = (time&0x3fff0000000) >> 28;

//   printf(DRED);
//   event->Print("a");
//   printf(RESET_COLOR);

	std::shared_ptr<TMidasEvent> copyevent = std::make_shared<TMidasEvent>(*event);
   copyevent->SetBankList();

   banksize = copyevent->LocateBank(nullptr, "GRF1", &ptr);
   for(int x=0;x<banksize;x++) {
      value = *((int*)ptr+x);
      type  = value & 0xf0000000; 

      switch(type) {
         case 0xa0000000:
            timelow = time&0x0fffffff;
            timelow += 0xa0000000;
            *((int*)ptr+x) = timelow;
            break;
         case 0xb0000000: {
            timehigh = (time&0x3fff0000000) >> 28;
            int tempdead = value & 0xffffc000;
            timehigh +=tempdead;
            *((int*)ptr+x) = timehigh;
            break;
         }
      };



       //printf( "0x%08x ",*((int*)ptr+x));
       //if(x!=0 && (x%7)==0)
       //   printf("\n");
   }
   //printf("===================\n");

   outfile->Write(copyevent,"q");

//   printf(DBLUE);
//   copyevent->Print("a");
//   printf(RESET_COLOR);

}







