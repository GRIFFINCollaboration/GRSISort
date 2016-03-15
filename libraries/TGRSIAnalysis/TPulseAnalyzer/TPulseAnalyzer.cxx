#include "TPulseAnalyzer.h"

/// \cond CLASSIMP
ClassImp(TPulseAnalyzer)
/// \endcond

TPulseAnalyzer::TPulseAnalyzer() : wpar(NULL), spar(NULL), shpar(NULL) {
	Clear();
}

TPulseAnalyzer::TPulseAnalyzer(TFragment &fragment,double noise_fac) : wpar(NULL), spar(NULL), shpar(NULL) {
	Clear();
	SetData(fragment,noise_fac);
}

TPulseAnalyzer::TPulseAnalyzer(std::vector<Short_t> &wave,double noise_fac,std::string name) : wpar(NULL), spar(NULL), shpar(NULL), fName(name) {
	Clear();
	SetData(wave,noise_fac);
}

TPulseAnalyzer::~TPulseAnalyzer(){
	if(wpar) delete wpar;
	if(spar) delete spar;
	if(shpar) delete shpar;
}

void TPulseAnalyzer::Clear(Option_t *opt) {
	SetCsI(false);
	set = false;	
	N = 0;
	FILTER = 8;
	T0RANGE = 8;
	LARGECHISQ = 1E111;
	EPS=0.001;

	lineq_dim = 0;
	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
	memset(lineq_solution,0,sizeof(lineq_solution));
	memset(copy_matrix,0,sizeof(copy_matrix));
}

void TPulseAnalyzer::SetData(TFragment &fragment,double noise_fac) {
	SetCsI(false);
	if(fragment.HasWave()) {
		if(noise_fac > 0) {
			FILTER=8*noise_fac;
			T0RANGE=8*noise_fac;	
		}
		//frag=&fragment;
		wavebuffer=fragment.wavebuffer;
		N=wavebuffer.size();
		if(N>0)	set=true;
	}
}

void TPulseAnalyzer::SetData(std::vector<Short_t> &wave,double noise_fac) {
	SetCsI(false);
	if(wave.size()>0) {
		if(noise_fac > 0) {
			FILTER=8*noise_fac;
			T0RANGE=8*noise_fac;	
		}
		//frag=&fragment;
		wavebuffer=wave;
		N=wavebuffer.size();
		if(N>0)	set=true;
	}
}

////////////////////////////////////////
//	Linear Equation Solver
////////////////////////////////////////

// "Very efficient" apparently, written by Kris S,
// Solve the currently stored n dimentional linear eqaution
int TPulseAnalyzer::solve_lin_eq() {
	memcpy(copy_matrix, lineq_matrix, sizeof(lineq_matrix));
	long double w = determinant(lineq_dim);
	if(w == 0.) return 0;
	for(int i = 0; i < lineq_dim; i++) {
		memcpy(copy_matrix,lineq_matrix,sizeof(lineq_matrix));
		memcpy(copy_matrix[i],lineq_vector,sizeof(lineq_vector));
		lineq_solution[i]=determinant(lineq_dim)/w;
	}
	return 1;
}

//solve the determinant of the currently stored copy_matrix for dimentions m
long double  TPulseAnalyzer::determinant(int m) {
	int j,i;
	long double s;
	if(m == 1) return copy_matrix[0][0];
	if(copy_matrix[m-1][m-1] == 0.) {
		j = m-1;
		while(copy_matrix[m-1][j] == 0 && j >= 0) j--;
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
	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
	lineq_dim=2;
	chisq=0.;
	ndf=0;
	k=(int)rint(x0);

	for(i=low;i<k;i++){
		lineq_matrix[0][0]+=1;
		lineq_vector[0]+=wavebuffer[i];
		ndf++;
		chisq+=wavebuffer[i]*wavebuffer[i];
	}

	for(i=k;i<high;i++){
		x=(i-x0)*(i-x0);
		lineq_matrix[0][0]+=1;
		lineq_matrix[0][1]+=x;
		lineq_matrix[1][1]+=x*x;
		lineq_vector[0]+=wavebuffer[i];
		lineq_vector[1]+=wavebuffer[i]*x;
		ndf++;
		chisq+=wavebuffer[i]*wavebuffer[i];
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

////////////////////////////////////////
//	RF Fit Run Functions
////////////////////////////////////////

double TPulseAnalyzer::fit_rf(double T)
{
	if(!set||N<10) return -1;
	if(spar) delete spar;
	spar=new SinPar;

	spar->t0=-1;

	return 5*get_sin_par(T);
}

////////////////////////////////////////
//	Waveform Time Fit Run Functions
////////////////////////////////////////

// Overall function which determins limits and fits the 3 trial functions
double  TPulseAnalyzer::fit_newT0(){
	if(!set||N<10)return -1; 
	
	if(wpar) delete wpar;
	wpar=new WaveFormPar;
	wpar->t0=-1;

	double chisq[3],chimin;
	WaveFormPar w[3];
	int  i,imin;

	wpar->baseline_range=T0RANGE; //default only 8 samples!
	get_baseline();
	get_tmax();
	
	//if(wpar->tmax<PIN_BASELINE_RANGE)
	//	return -1;
	
	if(!good_baseline())return -1;

	get_t30();
	get_t50();
	wpar->thigh=wpar->t50;
	
	for(i=0;i<3;i++)
		chisq[i]=LARGECHISQ;
	
	size_t swp;
	swp=sizeof(WaveFormPar);
	chisq[0]=get_smooth_T0(); memcpy(&w[0],wpar,swp);	
	chisq[1]=get_parabolic_T0(); memcpy(&w[1],wpar,swp);	
	chisq[2]=get_linear_T0(); memcpy(&w[2],wpar,swp);	

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

/*================================================================*/
int TPulseAnalyzer::fit_parabola(int low, int high,ParPar* pp){
	int i,ndf;
	double chisq;
	memset(pp,0,sizeof(ParPar));
	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
	lineq_dim=3;
	chisq=0.;
	ndf=0;
	for(i=low;i<high;i++){
		lineq_matrix[0][0]+=1;
		lineq_matrix[0][1]+=i;
		lineq_matrix[0][2]+=i*i;
		lineq_matrix[1][2]+=i*i*i;
		lineq_matrix[2][2]+=i*i*i*i;
		lineq_vector[0]+=wavebuffer[i];
		lineq_vector[1]+=wavebuffer[i]*i;
		lineq_vector[2]+=wavebuffer[i]*i*i;
		ndf++;
		chisq+=wavebuffer[i]*wavebuffer[i];
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
	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
	lineq_dim=2;
	chisq=0.;
	ndf=0;
	for(i=low;i<high;i++){
		lineq_matrix[0][0]+=1;
		lineq_matrix[0][1]+=i;
		lineq_matrix[1][1]+=i*i;
		lineq_vector[0]+=wavebuffer[i];
		lineq_vector[1]+=wavebuffer[i]*i;
		ndf++;
		chisq+=wavebuffer[i]*wavebuffer[i];
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
	int k;
	double chit,chitmin;
	double b,c,t;

	chitmin=LARGECHISQ;

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
	int k;
	double chit,chitmin;
	double a,b,c,d,t;

	chitmin=LARGECHISQ;
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
		wpar->baseline+=wavebuffer[i];
		wpar->baselineStDev+=wavebuffer[i]*wavebuffer[i];
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
			wpar->baselinefin+=wavebuffer[i];
			wpar->baselineStDevfin+=wavebuffer[i]*wavebuffer[i];
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

	wpar->max=wavebuffer[0];
	wpar->tmax=0;

	//applies the filter to the waveform
	//     cout<<" "<<wpar->tmax<<" "<< wpar->max<<flush;
	for(i=D;i<N-D;i++)	{
		sum=0;
		for(j=i-D;j<i+D;j++)
			sum+=wavebuffer[j];
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

	while(wavebuffer[t]>f){
		t--;
		if(t<=4) break;
	}
	imin=t;
	while(wavebuffer[imin]>flow){
		imin--;
		if(imin<=1) break;
	}

	imax=t;

	while(wavebuffer[imax]<fhigh) {
		imax++;
		if(imax>=N-1) break;
	}

	memset(lineq_matrix,0,sizeof(lineq_matrix));
	memset(lineq_vector,0,sizeof(lineq_vector));
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
		lineq_vector[0]+=wavebuffer[i];
		lineq_vector[1]+=wavebuffer[i]*a;
		lineq_vector[2]+=wavebuffer[i]*a*a;
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

double TPulseAnalyzer::get_sin_par(double T)
{
  int i;
  double s,sn,snm,s2,s2n,s2nm,c,cn,cnm,c2,c2n,c2nm,w;
  memset(lineq_matrix,0,sizeof(lineq_matrix));
  memset(lineq_vector,0,sizeof(lineq_vector));
  lineq_dim=3;

  w=2*TMath::Pi()/T;

  s=sin(w);
  sn=sin(N*w);
  snm=sin((N-1)*w);
  s2=sin(2*w);
  s2n=sin(2*N*w);
  s2nm=sin(2*(N-1)*w);

  c=cos(w);
  cn=cos(N*w);
  cnm=cos((N-1)*w);
  c2=cos(2*w);
  c2n=cos(2*N*w);
  c2nm=cos(2*(N-1)*w);
 
  lineq_matrix[0][0]=0.5*N-0.25*(1-c2-c2n+c2nm)/(1-c2);
  lineq_matrix[0][1]=0.25*(s2+s2nm-s2n)/(1-c2);
  lineq_matrix[1][0]=lineq_matrix[0][1];
  lineq_matrix[0][2]=0.5*(s+snm-sn)/(1-c);
  lineq_matrix[2][0]=lineq_matrix[0][2];
  lineq_matrix[1][1]=0.5*N+0.25*(1-c2-c2n+c2nm)/(1-c2);
  lineq_matrix[1][2]=0.5*(1-c-cn+cnm)/(1-c);
  lineq_matrix[2][1]=lineq_matrix[1][2];
  lineq_matrix[2][2]=N;

  for(i=0;i<lineq_dim;i++)
    lineq_vector[i]=0;
  
  for(i=0;i<N;i++)
    {
      lineq_vector[0]+=wavebuffer[i]*sin(w*i);
      lineq_vector[1]+=wavebuffer[i]*cos(w*i);
      lineq_vector[2]+=wavebuffer[i];
    }
  if(solve_lin_eq()==0)
    {
      //      printf("No solution for chi^2 fit of sin wave parameters \n");
      return 0;
    }
  spar->A=sqrt(lineq_solution[0]*lineq_solution[0]+lineq_solution[1]*lineq_solution[1]);
  spar->C=lineq_solution[2];
 
  s=-lineq_solution[1]/spar->A;
  c=lineq_solution[0]/spar->A;

  if(s>=0)
    spar->t0=acos(c)*T/(2*TMath::Pi());
  else
    spar->t0=(1-acos(c)/(2*TMath::Pi()))*T;
  
  return spar->t0;
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
	if(set&&wpar){
		if(wpar->t0>0){
			return (wpar->max-wpar->baselinefin)/wpar->baselineStDevfin;
		}
	}
	return -1;
}

short TPulseAnalyzer::good_baseline(){
	if(set&&wpar){
		if(wpar->tmax<T0RANGE)return 0;			
		if((wpar->max-wpar->baseline)<(wpar->baseline-wavebuffer[0])*10)return 0;
		if((wpar->max-wavebuffer[T0RANGE])<(wavebuffer[T0RANGE]-wpar->baseline)*4)return 0;
		return 1;
	}
	return 0;
}

//=====================================================//
//	CsI functions:
//=====================================================//

double TPulseAnalyzer::CsIt0(){
	if(CsIIsSet()){
		return shpar->t[0];
	}
	else{
		if(!set||N<10)return -1.; 
	
		if(wpar) delete wpar;
		wpar=new WaveFormPar;
		if(shpar) delete shpar;
		shpar=new ShapePar;
		//printf("Calculating exclusion zone\n");
		GetCsIExclusionZone();
		//int tmpchisq = GetCsIShape();
		//printf("Calculating shape\n");

		SetCsI();
		return shpar->t[0];
	}
	return -1.0;
}

double TPulseAnalyzer::CsIPID(){

	//printf("Fitting for PID\n");
	if(CsIIsSet()){
		double f = shpar->am[2];
		double s = shpar->am[3];
		double r = s/f*100;

		return r;

	}
	else{
		if(!set||N<10)return -1.; 
	
		if(wpar) delete wpar;
		wpar=new WaveFormPar;
		if(shpar) delete shpar;
		shpar=new ShapePar;

		shpar->t[1] = 4510;
		shpar->t[2] = 64.3;
		shpar->t[3] = 380.0;

		GetCsIExclusionZone();
		//int tmpchisq = GetCsIShape();
	
		double f = shpar->am[2];
		double s = shpar->am[3];
		double r = s/f*100;

		SetCsI();
		
		return r;
	}
	return -1.0;

}

int TPulseAnalyzer::GetCsIShape()
{

  int dim=4;

  long double sum,tau,tau_i,tau_j;
  int i,j,p,q,d;

  memset(lineq_matrix,0,sizeof(lineq_matrix));
  memset(lineq_vector,0,sizeof(lineq_vector));
  memset(lineq_solution,0,sizeof(lineq_solution));
  
  /* q is the low limit of the signal section */
  q=(int)wpar->temax;
  if(q >= N || q<=0)
	return -1.;
 
  /* p is the high limit of the baseline section */
  p=(int)wpar->temin;
  if(p >= N || p<=0)
	return -1.;
  lineq_dim=dim;
  
  //initialize amplitudes to 0
  for(int i=0;i<NSHAPE;i++)
    shpar->am[i]=0.;

	//printf("N:\t%i\n",N);
	//printf("p:\t%i\n",p);
	//printf("lineqdim:\t%i\n",lineq_dim);

  d=N;

  //initialize chi square 0 and ndf = n-k to -k where k=dim
  shpar->chisq=0.;
  shpar->ndf=-lineq_dim;

  /**************************************************************************
  linearized chi square fit is Mu = v where M is a data matrix 
  u, v are vectors; u is the parameter vector (solution)
  note that in this formulation, chisq_min = y_i^2-sum(u_iv_i)
  **************************************************************************/

  //create matrix for linearized fit
  for(i=1;i<lineq_dim;i++)
    {
      tau=GetCsITau(i);
      tau_i=tau;
      sum=-((double)q)/tau+log(1.-exp(-((double)(d-q))/tau));
      sum-=log(1.-exp(-1./tau));
      lineq_matrix[i][0]=exp(sum);
      lineq_matrix[0][i]=exp(sum);
      
      tau/=2.;
      sum=-((double)q)/tau+log(1.-exp(-((double)(d-q))/tau));
      sum-=log(1.-exp(-1./tau));
      lineq_matrix[i][i]=exp(sum);

      for(j=i+1;j<lineq_dim;j++)
  	{
	  tau_j=GetCsITau(j);
  	  tau=(tau_i*tau_j)/(tau_i+tau_j);
  	  sum=-((double)q)/tau+log(1.-exp(-((double)(d-q))/tau));
  	  sum-=log(1.-exp(-1./tau));
  	  lineq_matrix[i][j]=exp(sum);
  	  lineq_matrix[j][i]=exp(sum);
  	}
      
    }
  
  lineq_vector[0]=0;
  lineq_matrix[0][0]=0;

  for(j=q;j<N;j++)
    {
      lineq_vector[0]+=wavebuffer[j];
      lineq_matrix[0][0]+=1;
      shpar->chisq+=wavebuffer[j]*wavebuffer[j];
      shpar->ndf+=1;
    }
  
  if(lineq_dim >= N)
	return -1.;

  for(i=1;i<lineq_dim;i++)
    {
      tau=GetCsITau(i);
      lineq_vector[i]=0;
      for(j=q;j<N;j++)
  	lineq_vector[i]+=wavebuffer[j]*exp(-(double(j))/tau);
    }
  

  for(j=0;j<p;j++)
    {
      lineq_vector[0]+=wavebuffer[j];
      lineq_matrix[0][0]+=1;
      shpar->chisq+=wavebuffer[j]*wavebuffer[j];
      shpar->ndf+=1;
    }
 
  //solve the matrix equation Mu = v -> u = M^(-1)v where M^(-1) is the inverse
  //of M. note this has no solution if M is not invertable! 

  //error if the matrix cannot be inverted
  if(solve_lin_eq()==0)
    	{
	  //printf("Matrix could not be inverted\n");
	  shpar->chisq=BADCHISQ_MAT;
	  shpar->ndf=1;
	  return BADCHISQ_MAT;
	}
  
  //else try and find t0 and calculate amplitudes
  else
    {
      //see the function comments for find_t0 for details
	 
      shpar->t[0]=GetCsIt0();
      
      //if t0 is less than 0, return a T0FAIL
      if(shpar->t[0]<=0)
	{
	  //printf("t0 less than/equal to 0\n");
	  shpar->chisq=BADCHISQ_T0;
	  shpar->ndf=1;
	  return BADCHISQ_T0;
	}

      //calculate amplitudes	       
      shpar->am[0]=lineq_solution[0];

      for(i=1;i<lineq_dim;i++)
	{
	  tau=GetCsITau(i);
	  shpar->am[i]=lineq_solution[i]*exp(-shpar->t[0]/tau);
	}
      //done claculating amplitudes
 
      for(i=0;i<lineq_dim;i++)
  	shpar->chisq-=lineq_solution[i]*lineq_vector[i];

      if(shpar->chisq<0)
      	{
	  shpar->chisq=BADCHISQ_NEG;
	  shpar->ndf=1;
	  return BADCHISQ_NEG;
	}

      for(i=2;i<lineq_dim;i++)
	shpar->am[i]*=-1;

     }
   
  //return BADCHISQ_AMPL if a component amplitude is less than 0
  for(i=0;i<lineq_dim;i++)
    if(shpar->am[i]<0)
      	{
	  shpar->chisq=BADCHISQ_AMPL;
	  shpar->ndf=1;
	  return BADCHISQ_AMPL;
	}

  shpar->type=dim-2;

  return shpar->chisq;
}

void TPulseAnalyzer::GetCsIExclusionZone()
{
	int i,j;
	int D=FILTER/2; //filter half width
	double sum; //sum of waveform across filter

	//initilize the fit parameters for the risetime to 0 for safety
	wpar->afit=0.;
	wpar->bfit=0.;

	//make sure the baseline is established prior to finding the exclusion zone
	wpar->baseline_range=CSI_BASELINE_RANGE;
	get_baseline();

	//Here we determine the x position temax of the upper limit for the exclusion zone.
	//find tmax and define baselineMax
	get_tmax();

	//If tmax is established, continue.
	if(wpar->mflag==1)
	{
		wpar->baselineMax=wpar->baseline+NOISE_LEVEL_CSI;

		//Starting at tmax and working backwards along the waveform get the value of the filtered waveform at i and when the value of the filtered waveform goes below baselineMax, set the upper limit of the exclusion zone temax = i. The exclusion zone cannot be defined in the area of the waveform used to calculate the baseline.
		for(i=wpar->tmax;i>wpar->baseline_range;i--)
		{
			sum=0.;
			for(j=i-D;j<i+D;j++)
				sum+=wavebuffer[j];
			sum/=FILTER;
			if(sum<wpar->baselineMax)
			{
				wpar->temax=i;
				wpar->teflag=1;
				break;
			}
		}

		if(wpar->temax>wpar->tmax || wpar->temax<wpar->baseline_range)
			wpar->teflag=0;	
		//End of the determination of the upper limit of the exclusion zone.

		//Here we determine the x position of the lower limit for the exclusion zone
		/***** Fitting the risetime *****/
		if(wpar->teflag==1)
		{
			//Set baselineMin
			wpar->baselineMin=wpar->baseline-NOISE_LEVEL_CSI;

			//Here we fit a line y=ax+b from temax to temax + 3*FILTER and find the intersection with baselineMin. The x coordinate of this intersection becomes temin.
			//Matrix for the fit
			memset(lineq_matrix,0,sizeof(lineq_matrix));
			memset(lineq_vector,0,sizeof(lineq_vector));
			memset(lineq_solution,0,sizeof(lineq_solution));

			lineq_dim=2;
			//printf("temax %d temax+3*FILTER %d\n",wpar->temax,wpar->temax+3*FILTER);
			for(i=wpar->temax;i<=wpar->temax+3*FILTER;i++)
			{
				lineq_matrix[0][0]+=1;
				lineq_matrix[0][1]+=i;
				lineq_matrix[1][1]+=i*i;

				lineq_vector[0]+=wavebuffer[i];
				//printf("testing lineq_vector[0] %f\n",lineq_vector[0]);

				lineq_vector[1]+=wavebuffer[i]*i;
			}
			lineq_matrix[1][0]=lineq_matrix[0][1];

			//solve_lin_eq returns 0 if the determinant of the matrix is 0 the system is unsolvable. If there is no solution, set temin to the upper limit of the baseline range.
			if(solve_lin_eq()==0)
				wpar->temin=wpar->baseline_range;
			else
			{
				wpar->bfit=lineq_solution[0];
				wpar->afit=lineq_solution[1];
				//solve baselineMin = afit*x + bfit for x to find the crossing point. If the crossing point is outside the acceptable range, set temin to the upper limit of the baseline range.
				wpar->temin=(int)floor((wpar->baselineMin-wpar->bfit)/wpar->afit);
				if(wpar->temin<wpar->baseline_range)
					wpar->temin=wpar->baseline_range;
				if(wpar->temin>wpar->temax)
					wpar->temin=wpar->baseline_range;
			}
		}
	//End of the determination of the lower limit of the exclusion zone.
	}
}

double TPulseAnalyzer::GetCsITau(int i)
{

  if(i==1)
   return shpar->t[1];

  if(i>=2)
    if(i<NSHAPE)
      return shpar->t[i]*shpar->t[1]/(shpar->t[i]+shpar->t[1]);

  return -1.;
}

double TPulseAnalyzer::GetCsIt0()
{

  //printf("Calculating t0\n");

  /*************************************************************************
  This function calculates t0 given the matrix solution from the get_shape.
  In this case, the fit function is written as follows:
  
  f(t) = C + (Af+As)*exp(t0/tRC)*exp(-t/tRC) - Af*exp(t0/tF')*exp(-t/tF')
           - As*exp(t0/tS')*exp(-t/tS')
  
  This can be re-written as:

  f(t) = C' + alpha*exp(-t/tRC) + beta*exp(-t/tF') + gamma*exp(-t/tS')

  Where:
  C = C'
  alpha = (Af+As)*exp(t0/tRC)
  beta  = -Af*exp(t0/tF')
  gamma = -As*exp(t0/tS')

  Ignoring the constant, we have: 

  f'(t0) = alpha*exp(-t0/tRC) + beta*exp(-t0/tF') + gamma*exp(-t0/tS') = 0

  For t<t0, f'(t)< 0, and for t>t0, f'(t)>0. This function finds the
  intersection of f'(t) and 0 by linear interpolation from these endpoints.
  *************************************************************************/

  double fa,fb,fc; //value of the fit function at points a,b,c
  double ta,tb,tc=0; //corresponding time (x-)axis values
  double slope; //linear interpolation slope
  double delta; //checks how close is the interpolated f(t0) is to 0
  double tau;
  int i;

  ta=wpar->baseline_range;
  //ta=wpar->temin;
  fa=0.;
  
  //t0 must be between the baseline and the max
  //calculates fit value (no constant) at the end of the baseline range
  //this is the t<t0 point
  for(i=1;i<lineq_dim;i++)
    {
      tau=GetCsITau(i);
	  //printf("Tau @ 1117:\t%f\n",tau);
      //printf("i %d tau %f\n",i,tau);
      //getc(stdin);
      fa+=lineq_solution[i]*exp(-ta/tau);
    }
  
  tb=wpar->tmax;
  //tb=wpar->temax;
  fb=0.;
  
  //calculates fit value (no constant) at tmax
  //this is the t>t0 point
  for(i=1;i<lineq_dim;i++)
    {
      tau=GetCsITau(i);
	  //printf("Tau @ 1131:\t%f\n",tau);
      fb+=lineq_solution[i]*exp(-tb/tau);
    }

    delta=1;

    if( (fa<0) && (fb>0) )
	{
	  //keep the interpolation going until you get below epsilon
	  /* |f(t0) - 0| = |f(t0)|< epsilon */
	  while(delta>EPS)
	  { 
	    slope=-fa/(fb-fa); //interpolation slope for dependent variable t

	      //"reasonable" interpolation slopes
	      if(slope>0.99)
	      	slope=0.99;

	      if(slope<0.01)
	      	slope=0.01;
	      //its pretty harmless computationally

	      //tc is the estimate for t0
	      tc=ta+slope*(tb-ta);
	      fc=0.;
	      for(i=1;i<lineq_dim;i++)
		{
		  tau=GetCsITau(i);
		  fc+=lineq_solution[i]*exp(-tc/tau);
		}
	      
	      //really should have this, just to be safe
	      if(fc==0)
		{
		  return tc;
		}
	      
	      else if(fc>0)
	  	{
	  	  tb=tc;
	  	  fb=fc; 
	  	}
	      else
	  	{
	  	  ta=tc;
	  	  fa=fc; 
	  	}
	      delta=fabs(fc);
	    }
	}
      else
	{
	  return -1;
	}

	//printf("tc:\t%f\n",tc);

    //set wpar->t0 here
    wpar->t0=tc;

    return tc;
}

void  TPulseAnalyzer::DrawWave(){
	if(N==0||!set) return;
	TH1I h("Waveform",fName.c_str(),N,0,N); 
	for(Int_t i=0;i<N;i++)
	   h.SetBinContent(i+1,wavebuffer[i]);
	h.DrawCopy();
}

void TPulseAnalyzer::DrawRFFit(){

	if(N==0 || !set) return;
	
	DrawWave();
	
	if(spar){
		TF1 f("fit","[2] + [0]*sin(6.283185307 * (x - [1])/(2*8.48409))",0,N);

		f.SetParameter(0,spar->A);
		f.SetParameter(1,spar->t0);
		f.SetParameter(2,spar->C);

		f.DrawCopy("same");

		printf("t0:\t%f, A:\t%f, O:\t%f\n",spar->t0,spar->A,spar->C);
	}
	return;
}

void  TPulseAnalyzer::DrawT0fit(){
	
	if(N==0||!set) return;

	DrawWave();

	if(wpar){
		TF1 g("fit","[0]+[1]*x",0,wpar->temax);
		TF1 f("fit","[0]+[1]*x+[2]*x*x",wpar->temin,wpar->thigh);
		
		g.SetParameter(0,wpar->b0);
		g.SetParameter(1,wpar->b1);
		g.SetLineColor(kRed);

		f.SetParameter(0,wpar->s0);
		f.SetParameter(1,wpar->s1);
		f.SetParameter(2,wpar->s2);
		f.SetLineColor(8);
		
		f.DrawCopy("same");
		g.DrawCopy("same");
		
		printf("t0:\t%f\n",wpar->t0);
	}
	return;
}


void TPulseAnalyzer::DrawCsIExclusion(){
	
	if(N==0||!set) return;

	DrawWave();
	if(wpar){
		TF1 f("base","[0]",0,wpar->baseline_range);
		TF1 g("basemin","[0]",wpar->temin,wpar->temax);
		TF1 h("basemax","[0]",wpar->temin,wpar->temax);
		TF1 r("risetime","[0]*x+[1]",wpar->temin,wpar->temax+3*FILTER);
		
		printf("Baseline:\t%f\n",wpar->baseline);
		printf("Zero crossing:\t%f\n",wpar->t0);

		f.SetParameter(0,wpar->baseline);
		f.SetLineColor(kGreen);

		g.SetParameter(0,wpar->baselineMin);
		g.SetLineColor(kBlue);

		h.SetParameter(0,wpar->baselineMax);
		h.SetLineColor(kBlack);

		r.SetParameter(0,wpar->afit);
		r.SetParameter(1,wpar->bfit);
		r.SetLineColor(kRed);		

		f.DrawCopy("same");
		g.DrawCopy("same");
		h.DrawCopy("same");
		r.DrawCopy("same");
	}
	return;

}

void TPulseAnalyzer::DrawCsIFit(){

	if(N==0||!set||!CsIIsSet()) return;

	DrawWave();
	if(wpar){
		TF1 shape("shape",TGRSIFunctions::CsIFitFunction,0,N,9);

		shape.SetParameter(0,shpar->t[0]);
		shape.SetParameter(1,shpar->t[1]);
		shape.SetParameter(2,shpar->t[2]);
		shape.SetParameter(3,shpar->t[3]);
		shape.SetParameter(4,shpar->t[4]);
		shape.SetParameter(5,shpar->am[0]);
		shape.SetParameter(6,shpar->am[2]);
		shape.SetParameter(7,shpar->am[3]);
		shape.SetParameter(8,shpar->am[4]);
		shape.SetLineColor(kRed);

		printf("t0:\t%f,\ttRC:\t%f,\ttF:\t%f,\ttS:\t%f,\tTGamma:\t%f\n",(double)shpar->t[0],(double)shpar->t[1],(double)shpar->t[2],(double)shpar->t[3],(double)shpar->t[4]);
		printf("Baseline:\t%f,\tFast:\t%f,\tSlow:\t%f,\tGamma:\t%f\n",(double)shpar->am[0],(double)shpar->am[2],(double)shpar->am[3],(double)shpar->am[4]);

		shape.DrawCopy("same");
	}
	return;

}

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
