#include <iostream>
#include <algorithm>
#include <cstdio>
#include <cstring>

#include "TH1.h"

/* ======================================================================= */
void swapb8(char* buf)
{
   std::swap(buf[0], buf[7]);
   std::swap(buf[1], buf[6]);
   std::swap(buf[2], buf[5]);
   std::swap(buf[3], buf[4]);
} /* swapb8 */
void swapb4(char* buf)
{
   std::swap(buf[0], buf[3]);
   std::swap(buf[1], buf[2]);
} /* swapb4 */
void swapb2(char* buf)
{
   std::swap(buf[0], buf[1]);
} /* swapb2 */

/* ====================================================================== */

/* ====================================================================== */
int get_file_rec(FILE* fd, void* data, int maxbytes, int swap_bytes)
{
   /* read one fortran-unformatted style binary record into data */
   /* for unix systems, swap_bytes controls how get_file_rec deals with
      swapping the bytes of the record length tags at the start and end
      of the records.  Set swap_bytes to
        0 to try to automatically sense if the bytes need swapping
        1 to force the byte swap, or
       -1 to force no byte swap */
   /* returns number of bytes read in record,
      or number of bytes * -1 if bytes need swapping,
      or 0 for error */
   unsigned int reclen = 0;
   unsigned int j1     = 0;
   unsigned int j2     = 0;

   if(fread(&reclen, 4, 1, fd) != 1) { return 0; }

   if(reclen == 0) { return 0; }

   j1 = reclen;

   if((swap_bytes == 1) || (swap_bytes == 0 && reclen >= 65536)) { swapb4(reinterpret_cast<char*>(&reclen)); }

   if(reclen > maxbytes) {
      std::cout << "ERR1 \n";
      return 0;
   }

   if(fread(data, reclen, 1, fd) != 1 ||
      fread(&j2, 4, 1, fd) != 1) {
      std::cout << "ERR2 \n";
      return 0;
   }

   if(reclen == j1) { return reclen; }

   return (-reclen);
} /* get_file_rec */
/* ====================================================================== */

int load_spe(char* filename, TH1F* histo)
{
   int                   idim1 = 0;
   int                   idim2 = 0;
   int                   j     = 0;
   int                   rl    = 0;
   int                   swap  = -1;
   std::array<char, 128> cbuf;

   FILE*               infile = nullptr;
   std::array<char, 8> namesp;

   infile = fopen(filename, "r");

   rl = get_file_rec(infile, cbuf.data(), 128, 0);
   std::strncpy(namesp.data(), cbuf.data(), 8);
   memcpy(&idim1, cbuf.data() + 8, 4);
   memcpy(&idim2, cbuf.data() + 12, 4);

   const int          numch = idim1 * idim2;
   std::vector<float> sp(numch);

   TH1F* temp = new TH1F("temp", filename, numch, 0, numch);

   rl = get_file_rec(infile, sp.data(), 4 * numch, swap);
   fclose(infile);

   for(j = 0; j < numch; j++) {
      temp->Fill(j, sp[j]);
   }

   histo = static_cast<TH1F*>(temp->Clone());
   histo->SetName(filename);

   std::cout << "loaded\n";

   return 0;
}
