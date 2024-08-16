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
   explicit NRVec(int n);                                       // Zero-based array
   NRVec(const T& a, int n);                                    // initialize to constant value
   NRVec(const T* a, int n);                                    // Initialize to array
   NRVec(const NRVec& rhs);                                     // Copy constructor
   NRVec(NRVec&& rhs) noexcept = default;                       // Move constructor
   NRVec&          operator=(const NRVec& rhs);                 // copy assignment
   NRVec&          operator=(NRVec&& rhs) noexcept = default;   // move assignment
   NRVec&          operator=(const T& a);                       // assign a to every element
   inline T&       operator[](int i);                           // i'th element
   inline const T& operator[](int i) const;
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
         delete[] v;
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
      delete[] (v);
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
   NRMat(int n, int m);                                         // Zero-based array
   NRMat(const T& a, int n, int m);                             // Initialize to constant
   NRMat(const T* a, int n, int m);                             // Initialize to array
   NRMat(const NRMat& rhs);                                     // Copy constructor
   NRMat(NRMat&& rhs) noexcept = default;                       // Move constructor
   NRMat&          operator=(const NRMat& rhs);                 // copy assignment
   NRMat&          operator=(NRMat&& rhs) noexcept = default;   // move assignment
   NRMat&          operator=(const T& a);                       // assign a to every element
   inline T*       operator[](int i);                           // subscripting: pointer to row i
   inline const T* operator[](int i) const;
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
   v[0] = new T[m * n]();
   for(int i = 1; i < n; i++) {
      v[i] = v[i - 1] + m;
   }
   for(int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
         v[i][j] = a;
      }
   }
}

template <class T>
NRMat<T>::NRMat(const T* a, int n, int m) : nn(n), mm(m), v(new T*[n]())
{
   v[0] = new T[m * n]();
   for(int i = 1; i < n; i++) {
      v[i] = v[i - 1] + m;
   }
   for(int i = 0; i < n; i++) {
      for(int j = 0; j < m; j++) {
         v[i][j] = *a++;
      }
   }
}

template <class T>
NRMat<T>::NRMat(const NRMat& rhs) : nn(rhs.nn), mm(rhs.mm), v(new T*[nn]())
{
   v[0] = new T[mm * nn]();
   for(int i = 1; i < nn; i++) {
      v[i] = v[i - 1] + mm;
   }
   for(int i = 0; i < nn; i++) {
      for(int j = 0; j < mm; j++) {
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
      if(nn != rhs.nn || mm != rhs.mm) {
         delete[] v[0];
         delete[] v;
         nn   = rhs.nn;
         mm   = rhs.mm;
         v    = new T*[nn];
         v[0] = new T[mm * nn];
      }
      for(int i = 1; i < nn; i++) {
         v[i] = v[i - 1] + mm;
      }
      for(int i = 0; i < nn; i++) {
         for(int j = 0; j < mm; j++) {
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
      delete[] (v[0]);
      delete[] (v);
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
   inline T**             operator[](int i);   // subscripting: pointer to row i
   inline const T* const* operator[](int i) const;
   inline int             dim1() const;
   inline int             dim2() const;
   inline int             dim3() const;
   NRMat3d(const NRMat3d&)                = default;
   NRMat3d(NRMat3d&&) noexcept            = default;
   NRMat3d& operator=(const NRMat3d&)     = default;
   NRMat3d& operator=(NRMat3d&&) noexcept = default;
   ~NRMat3d();
};

template <class T>
NRMat3d<T>::NRMat3d() : nn(0), mm(0), kk(0), v(0)
{
}

template <class T>
NRMat3d<T>::NRMat3d(int n, int m, int k) : nn(n), mm(m), kk(k), v(new T**[n])
{
   v[0]    = new T*[n * m];
   v[0][0] = new T[n * m * k];
   for(int j = 1; j < m; j++) {
      v[0][j] = v[0][j - 1] + k;
   }
   for(int i = 1; i < n; i++) {
      v[i]    = v[i - 1] + m;
      v[i][0] = v[i - 1][0] + m * k;
      for(int j = 1; j < m; j++) {
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
      delete[] (v[0][0]);
      delete[] (v[0]);
      delete[] (v);
   }
}

using Vec_BOOL    = NRVec<bool>;
using Vec_I_BOOL  = const NRVec<bool>;
using Vec_O_BOOL  = NRVec<bool>;
using Vec_IO_BOOL = NRVec<bool>;

using Vec_CHR    = NRVec<char>;
using Vec_I_CHR  = const NRVec<char>;
using Vec_O_CHR  = NRVec<char>;
using Vec_IO_CHR = NRVec<char>;

using Vec_UCHR    = NRVec<unsigned char>;
using Vec_I_UCHR  = const NRVec<unsigned char>;
using Vec_O_UCHR  = NRVec<unsigned char>;
using Vec_IO_UCHR = NRVec<unsigned char>;

using Vec_INT    = NRVec<int>;
using Vec_I_INT  = const NRVec<int>;
using Vec_O_INT  = NRVec<int>;
using Vec_IO_INT = NRVec<int>;

using Vec_UINT    = NRVec<unsigned int>;
using Vec_I_UINT  = const NRVec<unsigned int>;
using Vec_O_UINT  = NRVec<unsigned int>;
using Vec_IO_UINT = NRVec<unsigned int>;

using Vec_LNG    = NRVec<int64_t>;
using Vec_I_LNG  = const NRVec<int64_t>;
using Vec_O_LNG  = NRVec<int64_t>;
using Vec_IO_LNG = NRVec<int64_t>;

using Vec_ULNG    = NRVec<uint64_t>;
using Vec_I_ULNG  = const NRVec<uint64_t>;
using Vec_O_ULNG  = NRVec<uint64_t>;
using Vec_IO_ULNG = NRVec<uint64_t>;

using Vec_SP    = NRVec<float>;
using Vec_I_SP  = const NRVec<float>;
using Vec_O_SP  = NRVec<float>;
using Vec_IO_SP = NRVec<float>;

using Vec_double    = NRVec<double>;
using Vec_I_double  = const NRVec<double>;
using Vec_O_double  = NRVec<double>;
using Vec_IO_double = NRVec<double>;

using Vec_CPLX_SP    = NRVec<std::complex<float>>;
using Vec_I_CPLX_SP  = const NRVec<std::complex<float>>;
using Vec_O_CPLX_SP  = NRVec<std::complex<float>>;
using Vec_IO_CPLX_SP = NRVec<std::complex<float>>;

using Vec_CPLX_double    = NRVec<std::complex<double>>;
using Vec_I_CPLX_double  = const NRVec<std::complex<double>>;
using Vec_O_CPLX_double  = NRVec<std::complex<double>>;
using Vec_IO_CPLX_double = NRVec<std::complex<double>>;

// Matrix Types

using Mat_BOOL    = NRMat<bool>;
using Mat_I_BOOL  = const NRMat<bool>;
using Mat_O_BOOL  = NRMat<bool>;
using Mat_IO_BOOL = NRMat<bool>;

using Mat_CHR    = NRMat<char>;
using Mat_I_CHR  = const NRMat<char>;
using Mat_O_CHR  = NRMat<char>;
using Mat_IO_CHR = NRMat<char>;

using Mat_UCHR    = NRMat<unsigned char>;
using Mat_I_UCHR  = const NRMat<unsigned char>;
using Mat_O_UCHR  = NRMat<unsigned char>;
using Mat_IO_UCHR = NRMat<unsigned char>;

using Mat_INT    = NRMat<int>;
using Mat_I_INT  = const NRMat<int>;
using Mat_O_INT  = NRMat<int>;
using Mat_IO_INT = NRMat<int>;

using Mat_UINT    = NRMat<unsigned int>;
using Mat_I_UINT  = const NRMat<unsigned int>;
using Mat_O_UINT  = NRMat<unsigned int>;
using Mat_IO_UINT = NRMat<unsigned int>;

using Mat_LNG    = NRMat<int64_t>;
using Mat_I_LNG  = const NRMat<int64_t>;
using Mat_O_LNG  = NRMat<int64_t>;
using Mat_IO_LNG = NRMat<int64_t>;

using Mat_ULNG    = NRMat<uint64_t>;
using Mat_I_ULNG  = const NRMat<uint64_t>;
using Mat_O_ULNG  = NRMat<uint64_t>;
using Mat_IO_ULNG = NRMat<uint64_t>;

using Mat_SP    = NRMat<float>;
using Mat_I_SP  = const NRMat<float>;
using Mat_O_SP  = NRMat<float>;
using Mat_IO_SP = NRMat<float>;

using Mat_double    = NRMat<double>;
using Mat_I_double  = const NRMat<double>;
using Mat_O_double  = NRMat<double>;
using Mat_IO_double = NRMat<double>;

using Mat_CPLX_SP    = NRMat<std::complex<float>>;
using Mat_I_CPLX_SP  = const NRMat<std::complex<float>>;
using Mat_O_CPLX_SP  = NRMat<std::complex<float>>;
using Mat_IO_CPLX_SP = NRMat<std::complex<float>>;

using Mat_CPLX_double    = NRMat<std::complex<double>>;
using Mat_I_CPLX_double  = const NRMat<std::complex<double>>;
using Mat_O_CPLX_double  = NRMat<std::complex<double>>;
using Mat_IO_CPLX_double = NRMat<std::complex<double>>;

// 3D Matrix Types

using Mat3D_double    = const NRMat3d<double>;
using Mat3D_I_double  = const NRMat3d<double>;
using Mat3D_O_double  = const NRMat3d<double>;
using Mat3D_IO_double = const NRMat3d<double>;

// Miscellaneous Types

using Vec_ULNG_p       = NRVec<uint64_t*>;
using Vec_Mat_double_p = NRVec<NRMat<double>*>;
using Vec_FSTREAM_p    = NRVec<std::fstream*>;

template <class T>
inline T SQR(const T a)
{
   return a * a;
}

template <class T>
inline T MAX(const T& a, const T& b)
{
   return b > a ? (b) : (a);
}

inline float MAX(const double& a, const float& b)
{
   return b > a ? (b) : static_cast<float>(a);
}

inline float MAX(const float& a, const double& b)
{
   return b > a ? static_cast<float>(b) : (a);
}

template <class T>
inline T SIGN(const T& a, const T& b)
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

inline std::complex<float> operator+(const double& a, const std::complex<float>& b)
{
   return static_cast<float>(a) + b;
}

inline std::complex<float> operator+(const std::complex<float>& a, const double& b)
{
   return a + static_cast<float>(b);
}

inline std::complex<float> operator-(const double& a, const std::complex<float>& b)
{
   return static_cast<float>(a) - b;
}

inline std::complex<float> operator-(const std::complex<float>& a, const double& b)
{
   return a - static_cast<float>(b);
}

inline std::complex<float> operator*(const double& a, const std::complex<float>& b)
{
   return static_cast<float>(a) * b;
}

inline std::complex<float> operator*(const std::complex<float>& a, const double& b)
{
   return a * static_cast<float>(b);
}

inline std::complex<float> operator/(const double& a, const std::complex<float>& b)
{
   return static_cast<float>(a) / b;
}

inline std::complex<float> operator/(const std::complex<float>& a, const double& b)
{
   return a / static_cast<float>(b);
}

class TLMFitter : public TObject {
public:
   TLMFitter()                                = default;
   TLMFitter(const TLMFitter&)                = default;
   TLMFitter(TLMFitter&&) noexcept            = default;
   TLMFitter& operator=(const TLMFitter&)     = default;
   TLMFitter& operator=(TLMFitter&&) noexcept = default;
   ~TLMFitter()                               = default;

private:
   int  fIntegrationSteps{100};
   TH1* fHist{nullptr};
   TF1* fFunction{nullptr};
   int  fInitChi2Number{3};
   int  fRangeMin{0};
   int  fRangeMax{0};

public:
   template <class T>
   class NRVec;
   template <class T>
   class NRMat;
   template <class T>
   class NRMat3d;

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
   void covsrt(Mat_IO_double& covar, Vec_I_BOOL& ia, int mfit);
   int  integrator(Vec_I_double& x, Vec_I_double& y, Vec_double& sig, Vec_I_double& W, Vec_IO_double& a,
                   Vec_double& dyda, int chisqnumber, const double& bin_width, Vec_double& yfit, const int& bin);

public:
   /// \cond CLASSIMP
   ClassDefOverride(TLMFitter, 1)   // NOLINT
   /// \endcond
};
/*! @} */
#endif   // TLMFitter_H
