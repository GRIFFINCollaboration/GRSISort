// Author: Ryan Dunlop    11/15

#ifndef TLMFITTER_H
#define TLMFITTER_H

/** \addtogroup Fitting Fitting & Analysis
 *  @{
 */

#include "Globals.h"

#include <vector>
#include <complex>
#include <iostream>
#include <fstream>

#include "Rtypes.h"
#include "TObject.h"
#include "TH1.h"
#include "TF1.h"

/////////////////////////////////////////////////////////////////
///
/// \class TLMFitter
///
/// This Class can be used to fit weighted-poisson distributed
/// data. It is originally from Numerical recipes, and adapted
/// by G.F Grinyer. It is based on the non-linear
/// Levenberg-Marquardt minimization algorithm.
///
/////////////////////////////////////////////////////////////////

// Vector Types

// Overloaded complex operations to handle mixed float and double
// This takes care of e.g. 1.0/z, z complex<float>

template <class T>
class NRVec {
private:
   int nn;   // size of array. upper index is nn-1
   T*  v;

public:
   NRVec();
   explicit NRVec(int n);                         // Zero-based array
   NRVec(const T& a, int n);                      // initialize to constant value
   NRVec(const T* a, int n);                      // Initialize to array
   NRVec(const NRVec& rhs);                       // Copy constructor
   NRVec&          operator=(const NRVec& rhs);   // assignment
   NRVec&          operator=(const T& a);         // assign a to every element
   inline T&       operator[](const int i);       // i'th element
   inline const T& operator[](const int i) const;
   inline int      size() const;
   ~NRVec();
};

template <class T>
NRVec<T>::NRVec() : nn(0), v(0)
{
}

template <class T>
NRVec<T>::NRVec(int n) : nn(n), v(new T[n]())
{
   for(int i = 0; i < n; ++i) {
      v[i] = 0.0;
   }
}

template <class T>
NRVec<T>::NRVec(const T& a, int n) : nn(n), v(new T[n]())
{
   for(int i = 0; i < n; i++) {
      v[i] = a;
   }
}

template <class T>
NRVec<T>::NRVec(const T* a, int n) : nn(n), v(new T[n]())
{
   for(int i = 0; i < n; i++) {
      v[i] = *a++;
   }
}

template <class T>
NRVec<T>::NRVec(const NRVec<T>& rhs) : nn(rhs.nn), v(new T[nn]())
{
   for(int i = 0; i < nn; i++) {
      v[i] = rhs[i];
   }
}

template <class T>
NRVec<T>& NRVec<T>::operator=(const NRVec<T>& rhs)
// postcondition: normal assignment via copying has been performed;
//		if vector and rhs were different sizes, vector
//		has been resized to match the size of rhs
{
   if(this != &rhs) {
      if(nn != rhs.nn) {
         if(v != 0) {
            delete[](v);
         }
         nn = rhs.nn;
         v  = new T[nn];
      }
      for(int i = 0; i < nn; i++) {
         v[i] = rhs[i];
      }
   }
   return *this;
}

template <class T>
NRVec<T>& NRVec<T>::operator=(const T& a)   // assign a to every element
{
   for(int i = 0; i < nn; i++) {
      v[i] = a;
   }
   return *this;
}

template <class T>
inline T& NRVec<T>::operator[](const int i)   // subscripting
{
   return v[i];
}

template <class T>
inline const T& NRVec<T>::operator[](const int i) const   // subscripting
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
   if(v != nullptr) {
      delete[](v);
   }
}

template <class T>
class NRMat {
private:
   int nn;
   int mm;
   T** v;

public:
   NRMat();
   NRMat(int n, int m);                           // Zero-based array
   NRMat(const T& a, int n, int m);               // Initialize to constant
   NRMat(const T* a, int n, int m);               // Initialize to array
   NRMat(const NRMat& rhs);                       // Copy constructor
   NRMat&          operator=(const NRMat& rhs);   // assignment
   NRMat&          operator=(const T& a);         // assign a to every element
   inline T*       operator[](const int i);       // subscripting: pointer to row i
   inline const T* operator[](const int i) const;
   inline int      nrows() const;
   inline int      ncols() const;
   ~NRMat();
};

template <class T>
NRMat<T>::NRMat() : nn(0), mm(0), v(0)
{
}

template <class T>
NRMat<T>::NRMat(int n, int m) : nn(n), mm(m), v(new T*[n]())
{
   v[0] = new T[m * n]();
   for(int i = 1; i < n; i++) {
      v[i] = v[i - 1] + m;
   }

   for(int i = 0; i < n; ++i) {
      for(int j = 0; j < m; ++j) {
         v[i][j] = 0.0;
      }
   }
}

template <class T>
NRMat<T>::NRMat(const T& a, int n, int m) : nn(n), mm(m), v(new T*[n]())
{
   int i, j;
   v[0] = new T[m * n]();
   for(i = 1; i < n; i++) {
      v[i] = v[i - 1] + m;
   }
   for(i = 0; i < n; i++) {
      for(j = 0; j < m; j++) {
         v[i][j] = a;
      }
   }
}

template <class T>
NRMat<T>::NRMat(const T* a, int n, int m) : nn(n), mm(m), v(new T*[n]())
{
   int i, j;
   v[0] = new T[m * n]();
   for(i = 1; i < n; i++) {
      v[i] = v[i - 1] + m;
   }
   for(i = 0; i < n; i++) {
      for(j = 0; j < m; j++) {
         v[i][j] = *a++;
      }
   }
}

template <class T>
NRMat<T>::NRMat(const NRMat& rhs) : nn(rhs.nn), mm(rhs.mm), v(new T*[nn]())
{
   int i, j;
   v[0] = new T[mm * nn]();
   for(i = 1; i < nn; i++) {
      v[i] = v[i - 1] + mm;
   }
   for(i = 0; i < nn; i++) {
      for(j = 0; j < mm; j++) {
         v[i][j] = rhs[i][j];
      }
   }
}

template <class T>
NRMat<T>& NRMat<T>::operator=(const NRMat<T>& rhs)
// postcondition: normal assignment via copying has been performed;
//		if matrix and rhs were different sizes, matrix
//		has been resized to match the size of rhs
{
   if(this != &rhs) {
      int i, j;
      if(nn != rhs.nn || mm != rhs.mm) {
         if(v != 0) {
            delete[](v[0]);
            delete[](v);
         }
         nn   = rhs.nn;
         mm   = rhs.mm;
         v    = new T*[nn];
         v[0] = new T[mm * nn];
      }
      for(i = 1; i < nn; i++) {
         v[i] = v[i - 1] + mm;
      }
      for(i = 0; i < nn; i++) {
         for(j = 0; j < mm; j++) {
            v[i][j] = rhs[i][j];
         }
      }
   }
   return *this;
}

template <class T>
NRMat<T>& NRMat<T>::operator=(const T& a)   // assign a to every element
{
   for(int i = 0; i < nn; i++) {
      for(int j = 0; j < mm; j++) {
         v[i][j] = a;
      }
   }
   return *this;
}

template <class T>
inline T* NRMat<T>::operator[](const int i)   // subscripting: pointer to row i
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
   if(v != nullptr) {
      delete[](v[0]);
      delete[](v);
   }
}

template <class T>
class NRMat3d {
private:
   int  nn;
   int  mm;
   int  kk;
   T*** v;

public:
   NRMat3d();
   NRMat3d(int n, int m, int k);
   inline T**             operator[](const int i);   // subscripting: pointer to row i
   inline const T* const* operator[](const int i) const;
   inline int             dim1() const;
   inline int             dim2() const;
   inline int             dim3() const;
   ~NRMat3d();
};

template <class T>
NRMat3d<T>::NRMat3d() : nn(0), mm(0), kk(0), v(0)
{
}

template <class T>
NRMat3d<T>::NRMat3d(int n, int m, int k) : nn(n), mm(m), kk(k), v(new T**[n])
{
   int i, j;
   v[0]    = new T*[n * m];
   v[0][0] = new T[n * m * k];
   for(j = 1; j < m; j++) {
      v[0][j] = v[0][j - 1] + k;
   }
   for(i = 1; i < n; i++) {
      v[i]    = v[i - 1] + m;
      v[i][0] = v[i - 1][0] + m * k;
      for(j = 1; j < m; j++) {
         v[i][j] = v[i][j - 1] + k;
      }
   }
}

template <class T>
inline T** NRMat3d<T>::operator[](const int i)   // subscripting: pointer to row i
{
   return v[i];
}

template <class T>
inline const T* const* NRMat3d<T>::operator[](const int i) const
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
   if(v != 0) {
      delete[](v[0][0]);
      delete[](v[0]);
      delete[](v);
   }
}

using Vec_I_BOOL = const NRVec<bool>;
typedef NRVec<bool> Vec_BOOL, Vec_O_BOOL, Vec_IO_BOOL;

using Vec_I_CHR = const NRVec<char>;
typedef NRVec<char> Vec_CHR, Vec_O_CHR, Vec_IO_CHR;

using Vec_I_UCHR = const NRVec<unsigned char>;
typedef NRVec<unsigned char> Vec_UCHR, Vec_O_UCHR, Vec_IO_UCHR;

using Vec_I_INT = const NRVec<int>;
typedef NRVec<int> Vec_INT, Vec_O_INT, Vec_IO_INT;

using Vec_I_UINT = const NRVec<unsigned int>;
typedef NRVec<unsigned int> Vec_UINT, Vec_O_UINT, Vec_IO_UINT;

using Vec_I_LNG = const NRVec<long>;
typedef NRVec<long> Vec_LNG, Vec_O_LNG, Vec_IO_LNG;

using Vec_I_ULNG = const NRVec<unsigned long>;
typedef NRVec<unsigned long> Vec_ULNG, Vec_O_ULNG, Vec_IO_ULNG;

using Vec_I_SP = const NRVec<float>;
typedef NRVec<float> Vec_SP, Vec_O_SP, Vec_IO_SP;

using Vec_I_double = const NRVec<double>;
typedef NRVec<double> Vec_double, Vec_O_double, Vec_IO_double;

using Vec_I_CPLX_SP = const NRVec<std::complex<float>>;
typedef NRVec<std::complex<float>> Vec_CPLX_SP, Vec_O_CPLX_SP, Vec_IO_CPLX_SP;

using Vec_I_CPLX_double = const NRVec<std::complex<double>>;
typedef NRVec<std::complex<double>> Vec_CPLX_double, Vec_O_CPLX_double, Vec_IO_CPLX_double;

// Matrix Types

using Mat_I_BOOL = const NRMat<bool>;
typedef NRMat<bool> Mat_BOOL, Mat_O_BOOL, Mat_IO_BOOL;

using Mat_I_CHR = const NRMat<char>;
typedef NRMat<char> Mat_CHR, Mat_O_CHR, Mat_IO_CHR;

using Mat_I_UCHR = const NRMat<unsigned char>;
typedef NRMat<unsigned char> Mat_UCHR, Mat_O_UCHR, Mat_IO_UCHR;

using Mat_I_INT = const NRMat<int>;
typedef NRMat<int> Mat_INT, Mat_O_INT, Mat_IO_INT;

using Mat_I_UINT = const NRMat<unsigned int>;
typedef NRMat<unsigned int> Mat_UINT, Mat_O_UINT, Mat_IO_UINT;

using Mat_I_LNG = const NRMat<long>;
typedef NRMat<long> Mat_LNG, Mat_O_LNG, Mat_IO_LNG;

using Mat_I_ULNG = const NRMat<unsigned long>;
typedef NRMat<unsigned long> Mat_ULNG, Mat_O_ULNG, Mat_IO_ULNG;

using Mat_I_SP = const NRMat<float>;
typedef NRMat<float> Mat_SP, Mat_O_SP, Mat_IO_SP;

using Mat_I_double = const NRMat<double>;
typedef NRMat<double> Mat_double, Mat_O_double, Mat_IO_double;

using Mat_I_CPLX_SP = const NRMat<std::complex<float>>;
typedef NRMat<std::complex<float>> Mat_CPLX_SP, Mat_O_CPLX_SP, Mat_IO_CPLX_SP;

using Mat_I_CPLX_double = const NRMat<std::complex<double>>;
typedef NRMat<std::complex<double>> Mat_CPLX_double, Mat_O_CPLX_double, Mat_IO_CPLX_double;

// 3D Matrix Types

using Mat3D_I_double = const NRMat3d<double>;
typedef NRMat3d<double> Mat3D_double, Mat3D_O_double, Mat3D_IO_double;

// Miscellaneous Types

using Vec_ULNG_p       = NRVec<unsigned long*>;
using Vec_Mat_double_p = NRVec<NRMat<double>*>;
using Vec_FSTREAM_p    = NRVec<std::fstream*>;

template <class T>
inline const T SQR(const T a)
{
   return a * a;
}

template <class T>
inline const T MAX(const T& a, const T& b)
{
   return b > a ? (b) : (a);
}

inline float MAX(const double& a, const float& b)
{
   return b > a ? (b) : float(a);
}

inline float MAX(const float& a, const double& b)
{
   return b > a ? float(b) : (a);
}

template <class T>
inline const T SIGN(const T& a, const T& b)
{
   return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}

inline float SIGN(const float& a, const double& b)
{
   return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}

inline float SIGN(const double& a, const float& b)
{
   return b >= 0 ? (a >= 0 ? a : -a) : (a >= 0 ? -a : a);
}

template <class T>
inline void SWAP(T& a, T& b)
{
   T dum = a;
   a     = b;
   b     = dum;
}

inline const std::complex<float> operator+(const double& a, const std::complex<float>& b)
{
   return float(a) + b;
}

inline const std::complex<float> operator+(const std::complex<float>& a, const double& b)
{
   return a + float(b);
}

inline const std::complex<float> operator-(const double& a, const std::complex<float>& b)
{
   return float(a) - b;
}

inline const std::complex<float> operator-(const std::complex<float>& a, const double& b)
{
   return a - float(b);
}

inline const std::complex<float> operator*(const double& a, const std::complex<float>& b)
{
   return float(a) * b;
}

inline const std::complex<float> operator*(const std::complex<float>& a, const double& b)
{
   return a * float(b);
}

inline const std::complex<float> operator/(const double& a, const std::complex<float>& b)
{
   return float(a) / b;
}

inline const std::complex<float> operator/(const std::complex<float>& a, const double& b)
{
   return a / float(b);
}

class TLMFitter : public TObject {
public:
   TLMFitter() : fIntegrationSteps(100), fInitChi2Number(3){};
   ~TLMFitter() override = default;

private:
   int  fIntegrationSteps;
   TH1* fHist{nullptr};
   TF1* fFunction{nullptr};
   int  fInitChi2Number;
   int  fRangeMin{0};
   int  fRangeMax{0};

public:
   template <class T>
   class NRVec;
   template <class T>
   class NRMat;
   template <class T>
   class NRMat3d;

public:
   void Fit(TH1* hist, TF1* func);

protected:
   void SetFitterRange(int min, int max)
   {
      fRangeMin = min;
      fRangeMax = max;
   }

   inline void nrerror(const std::string& error_text)
   // Numerical Recipes standard error handler
   {
      std::cerr << "Numerical Recipes run-time error..." << std::endl;
      std::cerr << error_text << std::endl;
      std::cerr << "...now exiting to system..." << std::endl;
      exit(1);
   }

   void funcs(const double& x, Vec_IO_double& a, double& y, Vec_O_double& dyda);
   void mrqmin(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_IO_double& a, Vec_I_BOOL& ia, Mat_O_double& covar,
               Mat_O_double& alpha, double& chisq, Vec_I_double& W, double& alamda);

   void mrqcof(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_IO_double& a, Vec_I_BOOL& ia, Mat_O_double& alpha,
               Vec_O_double& beta, double& chisq, Vec_I_double& W, double& chisqexp);
   void gaussj(Mat_IO_double& a, Mat_IO_double& b);
   void covsrt(Mat_IO_double& covar, Vec_I_BOOL& ia, const int mfit);
   int  integrator(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_I_double& W, Vec_IO_double& a,
                   Vec_double& dyda, int chisqnumber, const double& bin_width, Vec_double& yfit, const int& bin);

public:
   ClassDefOverride(TLMFitter, 1);
};
/*! @} */
#endif   // TLMFitter_H
