#include "TPulseAnalyzer.h"

ClassImp(TPulseAnalyzer)

TPulseAnalyzer::TPulseAnalyzer():wpar(0),frag(0){
	Clear();
}
TPulseAnalyzer::TPulseAnalyzer(TFragment &fragment,double noise_fac):wpar(0),frag(0){
	Clear();
	SetData(fragment,noise_fac);
}

TPulseAnalyzer::~TPulseAnalyzer(){
	if(wpar) delete wpar;
}

void TPulseAnalyzer::Clear(Option_t *opt)  {
	if(wpar) delete wpar;
	wpar=new WaveFormPar;
	
	set=false;	
	N=0;
	FILTER=8;
	T0RANGE=8;
	LARGECHISQ=1E111;

	lineq_dim=0;
	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
	memset(lineq_solution,0,sizeof(lineq_solution));
	memset(copy_matrix,0,sizeof(copy_matrix));
	
}


void TPulseAnalyzer::SetData(TFragment &fragment,double noise_fac)  {
	if(fragment.HasWave()) {
		if(noise_fac>0){
			FILTER=8*noise_fac;
			T0RANGE=8*noise_fac;	
		}
		frag=&fragment;
		N=fragment.wavebuffer.size();
		if(N>0)	set=true;
	}
	
}



////////////////////////////////////////
//	Linear Equation Solver
////////////////////////////////////////

// "Very efficient" apparently, written by Kris S,
// Solve the currently stored n dimentional linear eqaution
int TPulseAnalyzer::solve_lin_eq(){
	memcpy(copy_matrix,lineq_matrix,sizeof(lineq_matrix));
	long double w=determinant(lineq_dim);
	if(w==0.)return 0;
	for(int i=0;i<lineq_dim;i++){
		memcpy(copy_matrix,lineq_matrix,sizeof(lineq_matrix));
		memcpy(copy_matrix[i],lineq_vector,sizeof(lineq_vector));
		lineq_solution[i]=determinant(lineq_dim)/w;
	}
	return 1;
}

//solve the determinant of the currently stored copy_matrix for dimentions m
long double  TPulseAnalyzer::determinant(int m){
	int j,i;
	long double s;
	if(m==1) return copy_matrix[0][0];
	if(copy_matrix[m-1][m-1]==0.)  {
		j=m-1;
		while(copy_matrix[m-1][j]==0 && j>=0) j--;
		if(j<0) 
		return 0.;
		else for(i=0;i<m;i++){
			s=copy_matrix[i][m-1];
			copy_matrix[i][m-1]=copy_matrix[i][j];
			copy_matrix[i][j]=s;
		}
	}
	for(j=m-2;j>=0;j--)
		for(int i=0;i<m;i++)
			copy_matrix[i][j]-=copy_matrix[i][m-1]/copy_matrix[m-1][m-1]*copy_matrix[m-1][j];
	return copy_matrix[m-1][m-1]*determinant(m-1);
}


////////////////////////////////////////
//	Waveform Fits Functions
////////////////////////////////////////

int TPulseAnalyzer::fit_smooth_parabola(int low, int high, double x0, ParPar* pp){
	int i,ndf,k;
	double chisq;
	double x;
	memset(pp,0,sizeof(ParPar));
	lineq_dim=2;
	chisq=0.;
	ndf=0;
	k=(int)rint(x0);

	for(i=low;i<k;i++){
		lineq_matrix[0][0]+=1;
		lineq_vector[0]+=frag->wavebuffer[i];
		ndf++;
		chisq+=frag->wavebuffer[i]*frag->wavebuffer[i];
	}

	for(i=k;i<high;i++){
		x=(i-x0)*(i-x0);
		lineq_matrix[0][0]+=1;
		lineq_matrix[0][1]+=x;
		lineq_matrix[1][1]+=x*x;
		lineq_vector[0]+=frag->wavebuffer[i];
		lineq_vector[1]+=frag->wavebuffer[i]*x;
		ndf++;
		chisq+=frag->wavebuffer[i]*frag->wavebuffer[i];
	}
	lineq_matrix[1][0]=lineq_matrix[0][1];

	if(solve_lin_eq()==0)	{
		pp->chisq=BADCHISQ_MAT;
		return -1;
	}else{
		chisq-=lineq_vector[0]*lineq_solution[0];
		chisq-=lineq_vector[1]*lineq_solution[1];

		pp->constant=lineq_solution[0];
		pp->linear=0.;
		pp->quadratic=lineq_solution[1];
		pp->chisq=chisq;
		pp->ndf=ndf;

		return 1;
	}
	return -1;
}
/*================================================================*/
int TPulseAnalyzer::fit_parabola(int low, int high,ParPar* pp){
  int i,ndf;
  double chisq;
  memset(pp,0,sizeof(ParPar));
  lineq_dim=3;
  chisq=0.;
  ndf=0;
  for(i=low;i<high;i++){
      lineq_matrix[0][0]+=1;
      lineq_matrix[0][1]+=i;
      lineq_matrix[0][2]+=i*i;
      lineq_matrix[1][2]+=i*i*i;
      lineq_matrix[2][2]+=i*i*i*i;
      lineq_vector[0]+=frag->wavebuffer[i];
      lineq_vector[1]+=frag->wavebuffer[i]*i;
      lineq_vector[2]+=frag->wavebuffer[i]*i*i;
      ndf++;
      chisq+=frag->wavebuffer[i]*frag->wavebuffer[i];
    }
  lineq_matrix[1][0]=lineq_matrix[0][1];
  lineq_matrix[1][1]=lineq_matrix[0][2];
  lineq_matrix[2][0]=lineq_matrix[0][2];
  lineq_matrix[2][1]=lineq_matrix[1][2];
 
  if(solve_lin_eq()==0)
    {
      pp->chisq=BADCHISQ_MAT;
      return -1;
    }else{
      chisq-=lineq_vector[0]*lineq_solution[0];
      chisq-=lineq_vector[1]*lineq_solution[1];
      chisq-=lineq_vector[2]*lineq_solution[2];
      pp->constant=lineq_solution[0];
      pp->linear=lineq_solution[1];
      pp->quadratic=lineq_solution[2];
      pp->chisq=chisq;
      pp->ndf=ndf;
      return 1;
    }
}
/*================================================================*/
int TPulseAnalyzer::fit_line(int low, int high,LinePar* lp){
  int i,ndf;
  double chisq;
  memset(lp,0,sizeof(LinePar));
  lineq_dim=2;
  chisq=0.;
  ndf=0;
  for(i=low;i<high;i++){
      lineq_matrix[0][0]+=1;
      lineq_matrix[0][1]+=i;
      lineq_matrix[1][1]+=i*i;
      lineq_vector[0]+=frag->wavebuffer[i];
      lineq_vector[1]+=frag->wavebuffer[i]*i;
      ndf++;
      chisq+=frag->wavebuffer[i]*frag->wavebuffer[i];
   }
  lineq_matrix[1][0]=lineq_matrix[0][1];
 
  if(solve_lin_eq()==0)  {
      lp->chisq=BADCHISQ_MAT;
      return -1;
    }else{
      chisq-=lineq_vector[0]*lineq_solution[0];
      chisq-=lineq_vector[1]*lineq_solution[1];
      lp->slope=lineq_solution[1];
      lp->intercept=lineq_solution[0];
      lp->chisq=chisq;
      lp->ndf=ndf;
      return 1;
    }
}
/*================================================================*/



/*======================================================*/

double  TPulseAnalyzer::get_linear_T0(){
  LinePar lp,lpl;
  int k;//,kmin;
  double chit,chitmin;
  double b,c,t;
  
  chitmin=LARGECHISQ;
  //kmin=0;
  
  for(k=T0RANGE/2;k<wpar->thigh-T0RANGE/8;k++){
      //fit line to the baseline
      fit_line(0,k,&lp);
  
      //fit line to the risetime
      fit_line(k,wpar->thigh,&lpl);
      
      
      chit=lp.chisq+lpl.chisq;
	  
      if(chit<chitmin){
	  chitmin=chit;
	  wpar->b0=lp.intercept;
	  wpar->b1=lp.slope;
	  wpar->s0=lpl.intercept;
	  wpar->s1=lpl.slope;
	  wpar->s2=0.;
	  //kmin=k;
	}
    }	// end of the loop over k
  b=wpar->s1-wpar->b1;
  c=wpar->s0-wpar->b0;
  t=-c/b;

  wpar->t0=-1;
  wpar->temin=0;
  wpar->temax=wpar->thigh;
   if(t<N&&t>0){
	wpar->t0=t;
	wpar->temin=(int)rint(wpar->t0)-2;
	wpar->temax=(int)rint(wpar->t0)+2;
	return (double)(chitmin/(wpar->thigh-5));
   }
  
  return BADCHISQ_LIN_T0;
}
/*================================================================*/
double  TPulseAnalyzer::get_smooth_T0()
{
  ParPar pp,ppmin;
  int k,kmin;
  double chit,chitmin;
  double c,t;
 
  memset(&ppmin,0,sizeof(ParPar));
 
  chitmin=LARGECHISQ;
  kmin=0;
  //corse search first
  for(k=T0RANGE/2;k<wpar->thigh-T0RANGE/2;k++){
      fit_smooth_parabola(0,wpar->thigh,(double)k,&pp);

      chit=pp.chisq;
      if(chit<chitmin)
	{
	  chitmin=chit;
	  kmin=k;
	}
    }	// end of the corse search loop over k
  c=kmin;
  chitmin=LARGECHISQ;
  //fine search next
  for(t=kmin-1;t<kmin+1;t+=0.1){
      fit_smooth_parabola(0,wpar->thigh,t,&pp);
      chit=pp.chisq;
      if(chit<chitmin){
      	  memcpy(&ppmin,&pp,sizeof(ParPar));
	  chitmin=chit;
	  c=t;
	}
    }	// end of the fine search loop over k

  memcpy(&pp,&ppmin,sizeof(ParPar));
  t=c;
  wpar->s0=pp.constant+pp.quadratic*t*t;
  wpar->s1=-2.*pp.quadratic*t;
  wpar->s2=pp.quadratic;
  wpar->b0=pp.constant;
  wpar->b1=0.;

  wpar->t0=-1;
  wpar->temin=0;
  wpar->temax=wpar->thigh;
   if(t<N&&t>0)
      {
	wpar->t0=t;
	wpar->temin=(int)rint(wpar->t0)-2;
	wpar->temax=(int)rint(wpar->t0)+2;
	return (double)(chitmin/(wpar->thigh-2));
      } 
  return BADCHISQ_SMOOTH_T0;
}
/*================================================================*/
double TPulseAnalyzer::get_parabolic_T0(){

  LinePar lp;
  ParPar pp;
  int k;//,kmin;
  double chit,chitmin;
  double a,b,c,d,t;
 
  chitmin=LARGECHISQ;
  //kmin=0;
  for(k=T0RANGE/2;k<wpar->thigh-T0RANGE/2;k++){
      //fit line to the baseline
      fit_line(0,k,&lp);
      
      //fit parabola to the risetime
      fit_parabola(k,wpar->thigh,&pp);
      
      chit=lp.chisq+pp.chisq;
      
      if(chit<chitmin){
	  chitmin=chit;
	  wpar->b0=lp.intercept;
	  wpar->b1=lp.slope;
	  wpar->s0=pp.constant;
	  wpar->s1=pp.linear;
	  wpar->s2=pp.quadratic;
	  //kmin=k;
	}
    }//end loop through k

  
  a=wpar->s2;
  b=wpar->s1-wpar->b1;
  c=wpar->s0-wpar->b0;
  d=b*b-4*a*c;
  
  t=-1.;
  if(a==0.) t=-c/b;
  else{
      if(d>=0){
	  if(d==0.)t=-0.5*b/a;
	  else {
	      d=sqrt(d);
	      t=0.5*(-b+d)/a;
	    }
	}else{
	  return BADCHISQ_PAR_T0;
	}
    }

  wpar->t0=-1;
  wpar->temin=0;
  wpar->temax=wpar->thigh;
   if(t<N&&t>0){
	wpar->t0=t;
	wpar->temin=(int)rint(wpar->t0)-2;
	wpar->temax=(int)rint(wpar->t0)+2;
	return (double)(chitmin/(wpar->thigh-5));
      } 
  return BADCHISQ_PAR_T0;
 	
}


////////////////////////////////////////
//	Waveform Time Fit Run Functions
////////////////////////////////////////

// Overall function which determins limits and fits the 3 trial functions
double  TPulseAnalyzer::fit_newT0(){
	if(!set||N<10)return -1; 
	
	wpar->t0=-1;
	
	double chisq[3],chimin;
	WaveFormPar w[3];
	size_t swp;
	int  i,imin;

	swp=sizeof(WaveFormPar);

	wpar->baseline_range=T0RANGE; //only 8 samples!
	get_baseline();

	get_tmax();

	if(wpar->tmax<PIN_BASELINE_RANGE)
	return BAD_BASELINE_RANGE;

	get_t30();
	get_t50();
	wpar->thigh=wpar->t50;

	for(i=0;i<3;i++)
	chisq[i]=LARGECHISQ;

	chisq[0]=get_smooth_T0(); memcpy(&w[0],wpar,swp);	
	chisq[1]=get_parabolic_T0(); memcpy(&w[1],wpar,swp);	
	chisq[2]=get_linear_T0();// memcpy(&w[2],wpar,swp);	

	chimin=LARGECHISQ;
	imin=0;

	for(i=0;i<3;i++)if(chisq[i]<chimin&&chisq[i]>0){
		chimin=chisq[i];
		imin=i;
	}

	if(imin<2)memcpy(wpar,&w[imin],swp);
	
	get_baseline_fin();
	return wpar->t0;
}
/*================================================================*/

// Measure the baseline and standard deviation of the waveform, over the tick range specified by wpar->baseline_range
void TPulseAnalyzer::get_baseline(){
	wpar->baseline=0.;
	wpar->baselineStDev=0.;

	//error if waveform length N is shorter than baseline range
	if(N<wpar->baseline_range) {
		printf("Baseline range (%d) larger than waveform length!\n",wpar->baseline_range);
		printf("Terminating program\n");
		exit(0);
	}
  
	for(int i=0;i<wpar->baseline_range;i++){
		wpar->baseline+=frag->wavebuffer[i];
		wpar->baselineStDev+=frag->wavebuffer[i]*frag->wavebuffer[i];
	}
  
	wpar->baselineStDev/=wpar->baseline_range;
	wpar->baseline/=wpar->baseline_range; 
	wpar->baselineStDev-=wpar->baseline*wpar->baseline;
	wpar->baselineStDev=sqrt(wpar->baselineStDev); 
	wpar->bflag=1; //flag after establishing baseline
}

/*======================================================*/

// Measure the baseline and standard deviation up to tick wpar->t0 after a fit
void TPulseAnalyzer::get_baseline_fin(){
	wpar->baselinefin=0.;
	wpar->baselineStDevfin=0.;

	//error if waveform length N is shorter than baseline range
	if(N>wpar->t0&&wpar->t0>0){
		for(int i=0;i<wpar->t0;i++){
			wpar->baselinefin+=frag->wavebuffer[i];
			wpar->baselineStDevfin+=frag->wavebuffer[i]*frag->wavebuffer[i];
		}

		wpar->baselineStDevfin/=wpar->t0;
		wpar->baselinefin/=wpar->t0; 
		wpar->baselineStDevfin-=wpar->baselinefin*wpar->baselinefin;
		wpar->baselineStDevfin=sqrt(std::abs(wpar->baselineStDevfin)); 
	}
}

/*======================================================*/

// Find the maximum of the wavefunction, smoothed with a moving average filter
void TPulseAnalyzer::get_tmax(){
	int i,j,sum;
	int D=FILTER/2;

	wpar->max=frag->wavebuffer[0];
	wpar->tmax=0;

	//applies the filter to the waveform
	//     cout<<" "<<wpar->tmax<<" "<< wpar->max<<flush;
	for(i=D;i<N-D;i++)	{
		sum=0;
		for(j=i-D;j<i+D;j++)
		sum+=frag->wavebuffer[j];
		sum/=FILTER; //the value of the filtered waveform at i
		if(sum>wpar->max)	{
			//if the value of the filtered waveform at i is larger than the current maximum, max=value and tmax = i
			wpar->max=sum;
			wpar->tmax=i;
		}
	}
	wpar->mflag=1; //flag after finding tmax
}

/*===========================================================*/
double TPulseAnalyzer::get_tfrac(double frac,double fraclow, double frachigh)
{
  int t;
  double f,flow,fhigh;
  int i,imax,imin;
  long long int a;
  double p,q,r,d;

  if(wpar->bflag!=1)    {
      printf("Baseline not deterimned for the tfraction\n");
      exit(1);
    }

  if(wpar->mflag!=1)    {
      printf("Maximum not deterimned for the tfraction\n");
      exit(1);
    }
    
  t=wpar->tmax;
  
  f=wpar->baseline+frac*(wpar->max-wpar->baseline);
  flow=wpar->baseline+fraclow*(wpar->max-wpar->baseline);
  fhigh=wpar->baseline+frachigh*(wpar->max-wpar->baseline);
  
  while(frag->wavebuffer[t]>f){
      t--;
      if(t<=4) break;
   }
  imin=t;
  while(frag->wavebuffer[imin]>flow){
      imin--;
      if(imin<=1) break;
    }
  
  imax=t;

   while(frag->wavebuffer[imax]<fhigh) {
      imax++;
      if(imax>=N-1) break;
    }

  lineq_dim=3;

  i=imax-imin;
  a=i;
  lineq_matrix[0][0]=a+1;
  lineq_matrix[0][1]=0.5*a;
  lineq_matrix[2][0]=a/6.;
  lineq_matrix[2][2]=-a/30.;
  a*=i;
  lineq_matrix[0][1]+=0.5*a;
  lineq_matrix[2][0]+=0.5*a;
  lineq_matrix[2][1]=0.25*a;
  a*=i;
  lineq_matrix[2][0]+=a/3.;
  lineq_matrix[2][1]+=0.5*a;
  lineq_matrix[2][2]+=a/3.;
  a*=i;
  lineq_matrix[2][1]+=0.25*a;
  lineq_matrix[2][2]+=0.5*a;
  a*=i;
  lineq_matrix[2][2]+=0.2*a;

  lineq_matrix[1][0]=lineq_matrix[0][1];
  lineq_matrix[1][1]=lineq_matrix[2][0];
  lineq_matrix[0][2]=lineq_matrix[2][0];
  lineq_matrix[1][2]=lineq_matrix[2][1];

 for(i=0;i<lineq_dim;i++)
    lineq_vector[i]=0;

  for(i=imin;i<imax+1;i++)    {
      a=i-imin;
      lineq_vector[0]+=frag->wavebuffer[i];
      lineq_vector[1]+=frag->wavebuffer[i]*a;
      lineq_vector[2]+=frag->wavebuffer[i]*a*a;
    }
     
    if(solve_lin_eq()==0){
      return -4;
    }else{
    	p=lineq_solution[0]-f;
    	q=lineq_solution[1];
    	r=lineq_solution[2];

    	if(r!=0) {
    	    d=q*q-4*r*p;
    	    if(d<0){
		return -5;
	    }else{
    		f=-q+sqrt(d);
    		f*=0.5;
    		f/=r;
    		f+=imin;
    		return f;
    	      }
    	  }else{
    	    if(q!=0){
    		f=-p/q;
    		return f;
    	      }else{
    	        return -6;
	      }
    	  }
      }
    return -7;
}

/* ==================================================== */
void TPulseAnalyzer::get_t50(){
  int t;
  
  t=get_tfrac(0.5,0.3,0.8);
  if((t>0)&&(t<MAX_SAMPLES)){
      wpar->t50_flag=1;
      wpar->t50=t;
    } else{
      wpar->t50_flag=-1;
      wpar->t50=-1;
    }
}
/* ==================================================== */
void TPulseAnalyzer::get_t90(){
  int t;
  
  t=get_tfrac(0.9,0.8,0.98);
    
  if((t>0)&&(t<MAX_SAMPLES))    {
      wpar->t90_flag=1;
      wpar->t90=t;
    } else   {
      wpar->t90_flag=-1;
      wpar->t90=-1;
    }
}
/*===========================================================*/
void TPulseAnalyzer::get_t10(){
  int t;

  t=get_tfrac(0.1,0.05,0.2);

  if((t>0)&&(t<MAX_SAMPLES))    {
      wpar->t10_flag=1;
      wpar->t10=t;
    }  else    {
      wpar->t10_flag=-1;
      wpar->t10=-1;
    }
}
/*===========================================================*/
void TPulseAnalyzer::get_t30(){
  int t;

  t=get_tfrac(0.3,0.15,0.45);
  if((t>0)&&(t<MAX_SAMPLES))    {
      wpar->t30_flag=1;
      wpar->t30=t;
    }  else    {
      wpar->t30_flag=-1;
      wpar->t30=-1;
    }
}


/*======================================================*/
// void TPulseAnalyzer::get_sig2noise(){ if(N==0)return;
// 	if(set){
// 		get_baseline();
// 		get_tmax();
// 		wpar->sig2noise=(wpar->max-wpar->baseline)/wpar->baselineStDev;
// 	}
// 	return;
// }
/*======================================================*/
double TPulseAnalyzer::get_sig2noise(){
	if(set){
		if(wpar->t0>0){
			return (wpar->max-wpar->baselinefin)/wpar->baselineStDevfin;
		}
	}
	return -1;
}







// // 		//ONLY FOR DATA WITH PEAK SIZE > 30, ROUGHLY
// // 		//ONLY IF RISE IS NOT IN FIRST 30 CHANNELS (large noise can fool this simple check)
// // 		if(frag.wavebuffer[frag.wavebuffer.size()-1]-frag.wavebuffer[0]>30&&
// // 		frag.wavebuffer[30]-frag.wavebuffer[0]<frag.wavebuffer[frag.wavebuffer.size()-1]-frag.wavebuffer[30]){










/*================================================================*/

// void  TPulseAnalyzer::display_newT0_fit( TApplication* theApp){ if(N==0) return;
// 
//   TH1D *h=new TH1D("Waveform","Waveform",N,0,N); 
//   h->Reset();
//   for(Int_t i=0;i<N;i++)
//     h->Fill(i,frag->wavebuffer[i]);
// 
// //   TCanvas *c=(TCanvas*)gROOT->FindObject("WaveformFit");
// //   if(c!=NULL) delete c;
// //   c = new TCanvas("WaveformFit", "WaveformFit",10,10, 700, 500);
//   
// 
//   TF1* g=new TF1("fit","[0]+[1]*x",0,wpar->temax);
//   TF1* f=new TF1("fit","[0]+[1]*x+[2]*x*x",wpar->temin,wpar->thigh);
//  
//   g->SetParameter(0,wpar->b0);
//   g->SetParameter(1,wpar->b1);
//   g->SetLineColor(kRed);
//   g->Draw("same");
// 
//   f->SetParameter(0,wpar->s0);
//   f->SetParameter(1,wpar->s1);
//   f->SetParameter(2,wpar->s2);
//   //f->SetLineWidth(2);
//   f->SetLineColor(8);
//   f->Draw("same");
//   
//   h->GetListOfFunctions()->Add(f->Clone());
//   h->GetListOfFunctions()->Add(g->Clone());
//   h->DrawCopy();
//   
//   //theApp->Run(kTRUE); 	      
//   h->Delete();
//   f->Delete();
//   g->Delete();
// }

/*======================================================*/
void TPulseAnalyzer::print_WavePar()
{
  printf("== Currently established waveform parameters ============\n");
  printf("baseline         : %10.2f\n",wpar->baseline);
  printf("baseline st. dev.: %10.2f\n",wpar->baselineStDev);
  printf("max              : %10.2f\n",(double)wpar->max);
  printf("tmax             : %10.2f\n",(double)wpar->tmax);
  printf("temin            : %10.2f\n",(double)wpar->temin);
  printf("temax            : %10.2f\n",(double)wpar->temax);
  printf("t0               : %10.2f\n",(double)wpar->t0);
}
