// Author: Ryan Dunlop    11/15

#ifndef TLMFITTER_H
#define TLMFITTER_H

#include "Globals.h"

#include <vector>
#include <complex>
#include <iostream>
#include <fstream>

#include "Rtypes.h"
#include "TObject.h"
#include "TH1.h"
#include "TF1.h"

////////////////////////////////////////////////////////////////
//                                                            //
// TLMFitter                                                  //
//                                                            //
// This Class can be used to fit weighted-poisson distributed //
// data. It is originally from Numerical recipes, and adapted //
// by G.F Grinyer. It is based on the non-linear 
// Levenberg-Marquardt minimization algorithm.
//                                                            //
////////////////////////////////////////////////////////////////
// Vector Types

typedef double DP;

//Overloaded complex operations to handle mixed float and double
//This takes care of e.g. 1.0/z, z complex<float>

template <class T>
class NRVec {
private:
	int nn;	// size of array. upper index is nn-1
	T *v;
public:
	NRVec();
	explicit NRVec(int n);		// Zero-based array
	NRVec(const T &a, int n);	//initialize to constant value
	NRVec(const T *a, int n);	// Initialize to array
	NRVec(const NRVec &rhs);	// Copy constructor
	NRVec & operator=(const NRVec &rhs);	//assignment
	NRVec & operator=(const T &a);	//assign a to every element
	inline T & operator[](const int i);	//i'th element
	inline const T & operator[](const int i) const;
	inline int size() const;
	~NRVec();
};

template <class T>
NRVec<T>::NRVec() : nn(0), v(0) {}

template <class T>
NRVec<T>::NRVec(int n) : nn(n), v(new T[n]) {}

template <class T>
NRVec<T>::NRVec(const T& a, int n) : nn(n), v(new T[n])
{
	for(int i=0; i<n; i++)
		v[i] = a;
}

template <class T>
NRVec<T>::NRVec(const T *a, int n) : nn(n), v(new T[n])
{
	for(int i=0; i<n; i++)
		v[i] = *a++;
}

template <class T>
NRVec<T>::NRVec(const NRVec<T> &rhs) : nn(rhs.nn), v(new T[nn])
{
	for(int i=0; i<nn; i++)
		v[i] = rhs[i];
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const NRVec<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if vector and rhs were different sizes, vector
//		has been resized to match the size of rhs
{
	if (this != &rhs)
	{
		if (nn != rhs.nn) {
			if (v != 0) delete [] (v);
			nn=rhs.nn;
			v= new T[nn];
		}
		for (int i=0; i<nn; i++)
			v[i]=rhs[i];
	}
	return *this;
}

template <class T>
NRVec<T> & NRVec<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i<nn; i++)
		v[i]=a;
	return *this;
}

template <class T>
inline T & NRVec<T>::operator[](const int i)	//subscripting
{
	return v[i];
}

template <class T>
inline const T & NRVec<T>::operator[](const int i) const	//subscripting
{
	return v[i];
}

template <class T>
inline int NRVec<T>::size() const
{
	return nn;
}

template <class T>
NRVec<T>::~NRVec()
{
	if (v != 0)
		delete[] (v);
}

template <class T>
class NRMat {
private:
	int nn;
	int mm;
	T **v;
public:
	NRMat();
	NRMat(int n, int m);			// Zero-based array
	NRMat(const T &a, int n, int m);	//Initialize to constant
	NRMat(const T *a, int n, int m);	// Initialize to array
	NRMat(const NRMat &rhs);		// Copy constructor
	NRMat & operator=(const NRMat &rhs);	//assignment
	NRMat & operator=(const T &a);		//assign a to every element
	inline T* operator[](const int i);	//subscripting: pointer to row i
	inline const T* operator[](const int i) const;
	inline int nrows() const;
	inline int ncols() const;
	~NRMat();
};

template <class T>
NRMat<T>::NRMat() : nn(0), mm(0), v(0) {}

template <class T>
NRMat<T>::NRMat(int n, int m) : nn(n), mm(m), v(new T*[n])
{
	v[0] = new T[m*n];
	for (int i=1; i< n; i++)
		v[i] = v[i-1] + m;
}

template <class T>
NRMat<T>::NRMat(const T &a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i,j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i-1] + m;
	for (i=0; i< n; i++)
		for (j=0; j<m; j++)
			v[i][j] = a;
}

template <class T>
NRMat<T>::NRMat(const T *a, int n, int m) : nn(n), mm(m), v(new T*[n])
{
	int i,j;
	v[0] = new T[m*n];
	for (i=1; i< n; i++)
		v[i] = v[i-1] + m;
	for (i=0; i< n; i++)
		for (j=0; j<m; j++)
			v[i][j] = *a++;
}

template <class T>
NRMat<T>::NRMat(const NRMat &rhs) : nn(rhs.nn), mm(rhs.mm), v(new T*[nn])
{
	int i,j;
	v[0] = new T[mm*nn];
	for (i=1; i< nn; i++)
		v[i] = v[i-1] + mm;
	for (i=0; i< nn; i++)
		for (j=0; j<mm; j++)
			v[i][j] = rhs[i][j];
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const NRMat<T> &rhs)
// postcondition: normal assignment via copying has been performed;
//		if matrix and rhs were different sizes, matrix
//		has been resized to match the size of rhs
{
	if (this != &rhs) {
		int i,j;
		if (nn != rhs.nn || mm != rhs.mm) {
			if (v != 0) {
				delete[] (v[0]);
				delete[] (v);
			}
			nn=rhs.nn;
			mm=rhs.mm;
			v = new T*[nn];
			v[0] = new T[mm*nn];
		}
		for (i=1; i< nn; i++)
			v[i] = v[i-1] + mm;
		for (i=0; i< nn; i++)
			for (j=0; j<mm; j++)
				v[i][j] = rhs[i][j];
	}
	return *this;
}

template <class T>
NRMat<T> & NRMat<T>::operator=(const T &a)	//assign a to every element
{
	for (int i=0; i< nn; i++)
		for (int j=0; j<mm; j++)
			v[i][j] = a;
	return *this;
}

template <class T>
inline T* NRMat<T>::operator[](const int i)	//subscripting: pointer to row i
{
	return v[i];
}

template <class T>
inline const T* NRMat<T>::operator[](const int i) const
{
	return v[i];
}

template <class T>
inline int NRMat<T>::nrows() const
{
	return nn;
}

template <class T>
inline int NRMat<T>::ncols() const
{
	return mm;
}

template <class T>
NRMat<T>::~NRMat()
{
	if (v != 0) {
		delete[] (v[0]);
		delete[] (v);
	}
}

template <class T>
class NRMat3d {
private:
	int nn;
	int mm;
	int kk;
	T ***v;
public:
	NRMat3d();
	NRMat3d(int n, int m, int k);
	inline T** operator[](const int i);	//subscripting: pointer to row i
	inline const T* const * operator[](const int i) const;
	inline int dim1() const;
	inline int dim2() const;
	inline int dim3() const;
	~NRMat3d();
};

template <class T>
NRMat3d<T>::NRMat3d(): nn(0), mm(0), kk(0), v(0) {}

template <class T>
NRMat3d<T>::NRMat3d(int n, int m, int k) : nn(n), mm(m), kk(k), v(new T**[n])
{
	int i,j;
	v[0] = new T*[n*m];
	v[0][0] = new T[n*m*k];
	for(j=1; j<m; j++)
		v[0][j] = v[0][j-1] + k;
	for(i=1; i<n; i++) {
		v[i] = v[i-1] + m;
		v[i][0] = v[i-1][0] + m*k;
		for(j=1; j<m; j++)
			v[i][j] = v[i][j-1] + k;
	}
}

template <class T>
inline T** NRMat3d<T>::operator[](const int i) //subscripting: pointer to row i
{
	return v[i];
}

template <class T>
inline const T* const * NRMat3d<T>::operator[](const int i) const
{
	return v[i];
}

template <class T>
inline int NRMat3d<T>::dim1() const
{
	return nn;
}

template <class T>
inline int NRMat3d<T>::dim2() const
{
	return mm;
}

template <class T>
inline int NRMat3d<T>::dim3() const
{
	return kk;
}

template <class T>
NRMat3d<T>::~NRMat3d()
{
	if (v != 0) {
		delete[] (v[0][0]);
		delete[] (v[0]);
		delete[] (v);
	}
}

typedef const NRVec<bool> Vec_I_BOOL;
typedef NRVec<bool> Vec_BOOL, Vec_O_BOOL, Vec_IO_BOOL;

typedef const NRVec<char> Vec_I_CHR;
typedef NRVec<char> Vec_CHR, Vec_O_CHR, Vec_IO_CHR;

typedef const NRVec<unsigned char> Vec_I_UCHR;
typedef NRVec<unsigned char> Vec_UCHR, Vec_O_UCHR, Vec_IO_UCHR;

typedef const NRVec<int> Vec_I_INT;
typedef NRVec<int> Vec_INT, Vec_O_INT, Vec_IO_INT;

typedef const NRVec<unsigned int> Vec_I_UINT;
typedef NRVec<unsigned int> Vec_UINT, Vec_O_UINT, Vec_IO_UINT;

typedef const NRVec<long> Vec_I_LNG;
typedef NRVec<long> Vec_LNG, Vec_O_LNG, Vec_IO_LNG;

typedef const NRVec<unsigned long> Vec_I_ULNG;
typedef NRVec<unsigned long> Vec_ULNG, Vec_O_ULNG, Vec_IO_ULNG;

typedef const NRVec<float> Vec_I_SP;
typedef NRVec<float> Vec_SP, Vec_O_SP, Vec_IO_SP;

typedef const NRVec<DP> Vec_I_DP;
typedef NRVec<DP> Vec_DP, Vec_O_DP, Vec_IO_DP;

typedef const NRVec<std::complex<float> > Vec_I_CPLX_SP;
typedef NRVec<std::complex<float> > Vec_CPLX_SP, Vec_O_CPLX_SP, Vec_IO_CPLX_SP;

typedef const NRVec<std::complex<DP> > Vec_I_CPLX_DP;
typedef NRVec<std::complex<DP> > Vec_CPLX_DP, Vec_O_CPLX_DP, Vec_IO_CPLX_DP;

// Matrix Types

typedef const NRMat<bool> Mat_I_BOOL;
typedef NRMat<bool> Mat_BOOL, Mat_O_BOOL, Mat_IO_BOOL;

typedef const NRMat<char> Mat_I_CHR;
typedef NRMat<char> Mat_CHR, Mat_O_CHR, Mat_IO_CHR;

typedef const NRMat<unsigned char> Mat_I_UCHR;
typedef NRMat<unsigned char> Mat_UCHR, Mat_O_UCHR, Mat_IO_UCHR;

typedef const NRMat<int> Mat_I_INT;
typedef NRMat<int> Mat_INT, Mat_O_INT, Mat_IO_INT;

typedef const NRMat<unsigned int> Mat_I_UINT;
typedef NRMat<unsigned int> Mat_UINT, Mat_O_UINT, Mat_IO_UINT;

typedef const NRMat<long> Mat_I_LNG;
typedef NRMat<long> Mat_LNG, Mat_O_LNG, Mat_IO_LNG;

typedef const NRMat<unsigned long> Mat_I_ULNG;
typedef NRMat<unsigned long> Mat_ULNG, Mat_O_ULNG, Mat_IO_ULNG;

typedef const NRMat<float> Mat_I_SP;
typedef NRMat<float> Mat_SP, Mat_O_SP, Mat_IO_SP;

typedef const NRMat<DP> Mat_I_DP;
typedef NRMat<DP> Mat_DP, Mat_O_DP, Mat_IO_DP;

typedef const NRMat<std::complex<float> > Mat_I_CPLX_SP;
typedef NRMat<std::complex<float> > Mat_CPLX_SP, Mat_O_CPLX_SP, Mat_IO_CPLX_SP;

typedef const NRMat<std::complex<DP> > Mat_I_CPLX_DP;
typedef NRMat<std::complex<DP> > Mat_CPLX_DP, Mat_O_CPLX_DP, Mat_IO_CPLX_DP;

// 3D Matrix Types

typedef const NRMat3d<DP> Mat3D_I_DP;
typedef NRMat3d<DP> Mat3D_DP, Mat3D_O_DP, Mat3D_IO_DP;

// Miscellaneous Types

typedef NRVec<unsigned long *> Vec_ULNG_p;
typedef NRVec<NRMat<DP> *> Vec_Mat_DP_p;
typedef NRVec<std::fstream *> Vec_FSTREAM_p;


template<class T>
inline const T SQR(const T a) {return a*a;}

template<class T>
inline const T MAX(const T &a, const T &b)
{return b > a ? (b) : (a);}

inline float MAX(const double &a, const float &b)
{return b > a ? (b) : float(a);}

inline float MAX(const float &a, const double &b)
{return b > a ? float(b) : (a);}

template<class T>
inline const T SIGN(const T &a, const T &b)
{return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}

inline float SIGN(const float &a, const double &b)
{return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}

inline float SIGN(const double &a, const float &b)
{return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);}

template<class T>
inline void SWAP(T &a, T &b)
{T dum=a; a=b; b=dum;}


inline const std::complex<float> operator+(const double &a, const std::complex<float> &b) { return float(a)+b; }

inline const std::complex<float> operator+(const std::complex<float> &a,
									  const double &b) { return a+float(b); }

inline const std::complex<float> operator-(const double &a,
									  const std::complex<float> &b) { return float(a)-b; }

inline const std::complex<float> operator-(const std::complex<float> &a,
									  const double &b) { return a-float(b); }

inline const std::complex<float> operator*(const double &a,
									  const std::complex<float> &b) { return float(a)*b; }

inline const std::complex<float> operator*(const std::complex<float> &a,
									  const double &b) { return a*float(b); }

inline const std::complex<float> operator/(const double &a,
									  const std::complex<float> &b) { return float(a)/b; }

inline const std::complex<float> operator/(const std::complex<float> &a,
									  const double &b) { return a/float(b); }

class TLMFitter : public TObject{
   public:
      TLMFitter(): fIntegrationSteps(1000), fInitChi2Number(3){};
      ~TLMFitter(){};

   private:
      int fIntegrationSteps;
      TH1* fHist;
      TF1* fFunction;
      int fInitChi2Number;

      typedef double DP;
   public:
      template <class T> class NRVec;
      template <class T> class NRMat;
      template <class T> class NRMat3d;
   public:
      void Fit(TH1* hist, TF1* function);

   protected:
	   inline void nrerror(const std::string error_text)
	   // Numerical Recipes standard error handler
	   {
		   //cerr << "Numerical Recipes run-time error..." << endl;
		   //cerr << error_text << endl;
		   //cerr << "...now exiting to system..." << endl;
		   //exit(1);
	   }
   
      void funcs(const DP &x, Vec_IO_DP &a, DP &y, Vec_O_DP &dyda);
      void mrqmin(Vec_I_DP &x, Vec_I_DP &y, Vec_DP &sig, Vec_IO_DP &a,
            Vec_I_BOOL &ia, Mat_O_DP &covar, Mat_O_DP &alpha, DP &chisq, Vec_I_DP &W,
            DP &alamda);

      void mrqcof(Vec_I_DP &x, Vec_I_DP &y, Vec_DP &sig, Vec_IO_DP &a,
            Vec_I_BOOL &ia, Mat_O_DP &alpha, Vec_O_DP &beta, DP &chisq, Vec_I_DP &W,
               DP &chisqexp);
      void gaussj(Mat_IO_DP &a, Mat_IO_DP &b);
      void covsrt(Mat_IO_DP &covar, Vec_I_BOOL &ia, const int mfit);
      int  integrator(Vec_I_DP &x, Vec_I_DP &y, Vec_DP &sig, Vec_I_DP &W,
			   Vec_IO_DP &a, Vec_DP &dyda, int chisqnumber, const double &bin_width, Vec_DP &yfit, const int &bin);

   public:

   ClassDef(TLMFitter,1);
};

#endif // TLMFitter_H
