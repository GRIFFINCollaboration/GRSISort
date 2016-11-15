

#include<stdio.h>
#include<string.h>

/* ======================================================================= */
void swapb8(char *buf)
{
  char c;
  c = buf[7]; buf[7] = buf[0]; buf[0] = c;
  c = buf[6]; buf[6] = buf[1]; buf[1] = c;
  c = buf[5]; buf[5] = buf[2]; buf[2] = c;
  c = buf[4]; buf[4] = buf[3]; buf[3] = c;
} /* swapb8 */
void swapb4(char *buf)
{
  char c;
  c = buf[3]; buf[3] = buf[0]; buf[0] = c;
  c = buf[2]; buf[2] = buf[1]; buf[1] = c;
} /* swapb4 */
void swapb2(char *buf)
{
  char c;
  c = buf[1]; buf[1] = buf[0]; buf[0] = c;
} /* swapb2 */

/* ====================================================================== */


/* ====================================================================== */
int get_file_rec(FILE *fd, void *data, int maxbytes, int swap_bytes)
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
  unsigned int  reclen, j1, j2;



  if (fread(&reclen, 4, 1, fd) != 1) return 0;
  
	if (reclen == 0) return 0;

  j1 = reclen;

  if ((swap_bytes == 1) ||
      (swap_bytes == 0 && reclen >= 65536)) swapb4((char *) &reclen);

  if (reclen > maxbytes) goto ERR1;

  if (fread(data, reclen, 1, fd) != 1 ||
      fread(&j2, 4, 1, fd) != 1) goto ERR2;

  /* if (j1 != j2) goto ERR2; */

  if (reclen == j1) return reclen;

  return (-reclen);


 ERR1:
  //warn("ERROR: record is too big for get_file_rec\n"
  //    "       max size = %d, record size = %d.\n",
  //     maxbytes, reclen);
	cout << "ERR1 \n";
  return 0;
 ERR2:
  //warn("ERROR during read in get_file_rec.\n");
	cout << "ERR2 \n";  
	return 0;

return 0;
} /* get_file_rec */
/* ====================================================================== */


int load_spe(char *filename,TH1F* histo)
{
	int idim1,idim2,j,rl;
	int swap = -1;
	char cbuf[128];

	FILE *infile;
	char namesp[8];
	//int numch;
	
	infile = fopen(filename,"r");
	
	rl = get_file_rec(infile,cbuf,128,0);
	strncpy(namesp,cbuf,8);
	memcpy(&idim1,cbuf+8,4);
	memcpy(&idim2,cbuf+12,4);

	const int numch = idim1 * idim2;
	float sp[numch];
	
	TH1F *temp = new TH1F("temp",filename,numch,0,numch);

	//histo->SetName(filename);
	//histo->GetXaxis()->SetTitle("Energy");	
	//histo->SetBinsLength(numch); 
	//histo->SetAxisRange(0,numch,"X");
	//histo->SetMinium(0);
	//histo->SetMaxium(numch);

	rl = get_file_rec(infile,sp,4*numch,swap);
	fclose(infile);

	for(j=0;j<numch;j++)
	{
		//if(sp[j]>0){cout << j << "\t" << sp[j] << endl;	}
		//histo->SetBinContent(j,sp[j]);
		temp->Fill(j,sp[j]);
	}	

	*histo = (TH1F*)temp->Clone();
	histo->SetName(filename);

	cout << "loaded\n";
	return 0;
}

