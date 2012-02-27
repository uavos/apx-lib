/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 * http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MATRIXMATH_H
#define MATRIXMATH_H
#include <iostream>
#include <math.h>
//=============================================================================
namespace matrixmath {
#ifdef USE_FLOAT_TYPE
typedef float   _mat_float;
#else
typedef double  _mat_float;
#endif
//=============================================================================
//=============================================================================
// VECTOR
//=============================================================================
template<const int n=3,class T=_mat_float>
class Vector {
public:
  T v[n];
  typedef unsigned int index_t;
  Vector() {this->fill();}
  Vector(const T &s) { this->fill(s); }
  Vector(const T &s0,const T &s1) {
    (*this)[0] = s0;
    (*this)[1] = s1;
  }
  Vector(const T &s0,const T &s1,const T &s2) {
    (*this)[0] = s0;
    (*this)[1] = s1;
    (*this)[2] = s2;
  }
  Vector(const T &s0,const T &s1,const T &s2,const T &s3) {
    (*this)[0] = s0;
    (*this)[1] = s1;
    (*this)[2] = s2;
    (*this)[3] = s3;
  }

  T *array() {return this->v;}
  const T* array()const {return this->v;}

  void fill(const T &value=T()) {
    for (index_t i=0 ; i<n ; i++)
      this->v[i] = value;
  }

  size_t size() const {return n;}

//Magnitude
  const T mag2() const {return (*this)*(*this);}
  const T mag() const {return sqrt(this->mag2());}

//add vectors
  const Vector operator+(const Vector &that) const {return Vector(*this)+=that;}
  Vector & operator+=(const Vector &that) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i]+=that[i];
    return (*this);
  }

//substract vectors
  const Vector operator-(const Vector &that) const {return Vector(*this)-=that;}
  Vector & operator-=(const Vector &that) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i]-=that[i];
    return (*this);
  }

//Negate a vector
  const Vector operator-() const {
    Vector V;
    for (index_t i=0 ; i<n ; i++)
      V[i] = -(*this)[i];
    return V;
  }

//multiply vectors
  const T operator *(const Vector &that) const {
    T dot = T();
    for (index_t i = 0 ; i<n ; i++)
      dot+=(*this)[i]*that[i];
    return dot;
  }

//acess by index
  T & operator[](unsigned int index) {return this->v[index];}
  const T & operator[](unsigned int index) const {return this->v[index];}

//multiply to scalar
  const Vector operator*(const T &scale) const {return Vector(*this)*=scale;}
  const Vector operator/(const T &scale) const {return Vector(*this)/=scale;}
  Vector & operator*=(const T &scale) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i] *= scale;
    return (*this);
  }
  Vector & operator/=(const T &scale) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i] /= scale;
    return (*this);
  }

//Normalize a vector
  const Vector norm() const {return (*this)/this->mag();}
  Vector & norm_self() {return (*this)/=this->mag();}
}; //Vector class
//------------------------------------------------------------------------------
//Multiplication is commutative.
template<const int n,class T1,class T2>
Vector<n,T1> operator * (const T2 & s,const Vector<n,T1> & v) {return v*s;}

//cross product
template<class T>
const Vector<3,T> cross(const Vector<3,T> &a,const Vector<3,T> &b) {
  return Vector<3,T>(
           a[1]*b[2] - a[2]*b[1],
           a[2]*b[0] - a[0]*b[2],
           a[0]*b[1] - a[1]*b[0]);
}

//Splice extracts a portion of a vector
template<const int size,const int n,class T>
const Vector<size,T> slice(const Vector<n,T> &v,const int start) {
  typedef typename Vector<n,T>::index_t index_t;
  Vector<size,T> out;
  for (index_t i= 0 ; i < size ; i++)
    out[i] = v[i+start];
  return out;
}

//insert a vector into a larger vector and returns the larger vector
template<const int n,class T1,const int size,class T2>
Vector<n,T1> &insert(Vector<n,T1> &v,int start,const Vector<size,T2> &in) {
  typedef typename Vector<n,T1>::index_t index_t;
  for (index_t i=0 ; i<size ; i++)
    v[i + start] = in[i];
  return v;
}
//=============================================================================
// MATRIX
//=============================================================================
template<const int n,const int m,class T = _mat_float> //rows,cols
class Matrix {
public:
  Vector<m,T> M[n];
  typedef typename Vector<m,T>::index_t index_t;
  Matrix() {}
  Matrix(const T & value) {this->fill(value);}
  Matrix(const Vector<m> & v0) {
    M[0] = v0;
  }
  Matrix(const Vector<m> & v0,const Vector<m> & v1,const Vector<m> & v2) {
    M[0] = v0;
    M[1] = v1;
    M[2] = v2;
  }
  Matrix(const Vector<m> & v0,const Vector<m> & v1,const Vector<m> & v2,const Vector<m> & v3) {
    M[0] = v0;
    M[1] = v1;
    M[2] = v2;
    M[3] = v3;
  }
  T* array() {return this->M[0].array();}
  const T* array() const {return this->M[0].array(); }
  size_t rows() const { return n; }
  size_t cols() const { return m; }
  void fill(const T & value = T()) { for (index_t i=0 ; i<n ; i++) this->row(i).fill(value); }

//row
  Vector<m,T> & operator[](index_t row_i) { return this->row(row_i); }
  const Vector<m,T> & operator[](index_t row_i) const { return this->row(row_i); }
  const Vector<m,T> & row(index_t row_i) const { return this->M[row_i]; }
  Vector<m,T> & row(index_t row_i) { return this->M[row_i]; }

//column
  const Vector<n,T> col(index_t col_i) const {
    Vector<n,T> v;
    for (index_t i=0 ; i < n ; i++)
      v[i] = (*this)[i][col_i];
    return v;
  }

//fill column
  void col(index_t col_i,const Vector<n,T> & v) {
    for (index_t i=0 ; i<n ; i++)
      (*this)[i][col_i] = v[i];
  }

//Insert a submatrix into the larger matrix
  template<class T2, int n2, int m2>
  void insert(int base_n, int base_m,const Matrix<n2,m2,T2> &M_i) {
    for (index_t i=0 ; i<n2 ; i++)
      for (index_t j=0 ; j<m2 ; j++)
        (*this)[i+base_n][j+base_m] = M_i[i][j];
  }

//Negate a matrix
  const Matrix operator-() const {
    Matrix Mn;
    for (index_t i=0 ; i<n ; i++)
      for (index_t j=0 ; j<m ; j++) {
        Mn[i][j] = -(*this)[i][j];
      }
    return Mn;
  }

//Add two matrixes, returning the sum of the two
  const Matrix & operator+ (const Matrix & that) const { return Matrix(*this) += that; }
  const Matrix & operator- (const Matrix & that) const { return Matrix(*this) -= that; }

//Update the matrix in place
  Matrix & operator+= (const Matrix & that) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i] += that[i];
    return (*this);
  }
  Matrix & operator-= (const Matrix & that) {
    for (index_t i=0 ; i < n ; i++)
      (*this)[i] -= that[i];
    return (*this);
  }

//multiplication
  template<int p> const Matrix<n,p,T>
  operator*(const Matrix<m,p,T> & B) const {
    Matrix<n,p,T> C;
    const Matrix<n,m,T> & A(*this);
    for (index_t i=0 ; i<n ; i++) {
      const Vector<m,T> & A_i = A[i];
      Vector<p,T> & C_i = C[i];
      for (index_t j=0 ; j<p ; j++) {
        T s = T();
        for (index_t k=0 ; k<m ; k++) {
          s += B[k][j] * A_i[k];
        }
        C_i[j] = s;
      }
    }
    return C;
  }
  Matrix & operator *= (const Matrix & B) { return (*this) = (*this) * B; }

  //Scale
  const Matrix operator * (const T & s) const { return Matrix(*this) *= s; }
  Matrix & operator *= (const T & s) {
    for (index_t i=0 ; i<n ; i++)
      (*this)[i] *= s;
    return (*this);
  }

  //transpose of the matrix
  const Matrix<m,n,T> transpose() const {
    const Matrix<n,m,T> & A(*this);
    Matrix<m,n,T> B;
    for (index_t i=0 ; i<n ; i++) {
      const Vector<m,T> & A_i(A[i]);
      for (index_t j=0 ; j<m ; j++)
        B[j][i] = A_i[j];
    }
    return B;
  }

  //Frobenius norm
  T frobenius_norm() const {
    T v=T();
    const Matrix<n,m,T> & A(*this);
    for (index_t i=0 ; i<n ; i++) {
      const Vector<m,T> & A_i(A[i]);
      v=v+A_i.mag2();
    }
    return sqrt(v);
  }

}; //Matrix
//------------------------------------------------------------------------------
//Vector<n> = Matrix<n,m> * Vector<m>
template<const int n,const int m,class T>
const Vector<n,T> operator*(const Matrix<n,m,T> & A,const Vector<m,T> & b) {
  typedef typename Vector<n,T>::index_t index_t;
  Vector<n,T> c;
  for (index_t i=0 ; i<n ; i++)
    c[i] = A[i] * b;
  return c;
}

//Vector<m> = Vector<n> * Matrix<n,m>
template<const int n,const int m,class T>
const Vector<m,T> operator*(const Vector<n,T> & a,const Matrix<n,m,T> & B) {
  typedef typename Vector<n,T>::index_t index_t;
  Vector<m,T> c;
  for (index_t i=0 ; i<m ; i++) {
    T s(0);
    for (index_t j=0 ; j<n ; j++)
      s += a[j] * B[j][i];
    c[i] = s;
  }
  return c;
}

//three way multiplication
template<const int n,const int m,const int p,const int q,class T>
const Matrix<n,q,T> mult3(const Matrix<n,m,T> & a,const Matrix<m,p,T> & b,const Matrix<p,q,T> & c) {
  if (n*m*p + n*p*q < m*p*q + n*m*q)
    return (a * b) * c;
  else
    return a * (b * c);
}

//sum of the diagonal elements in a square matrix
template<const int N,class T>
_mat_float trace(const Matrix<N,N,T> & m) {
  typedef typename Matrix<N,N,T>::index_t index_t;
  _mat_float t = 0;
  for (index_t i=0 ; i<N ; i++)
    t += m[i][i];
  return t;
}

//diagonal matrix
template<const int N,class T>
const Matrix<N,N,T> diag(const Vector<N,T> & v) {
  typedef typename Matrix<N,N,T>::index_t index_t;
  Matrix<N,N,T> m;
  for (index_t i=0 ; i<N ; i++)
    m[i][i]=v[i];
  return m;
}
template<const int N,class T>
const Matrix<N,N,T> eye() {
  return diag<N,T>(Vector<N,T>(1.0));
}
//=============================================================================
// DEBUG OUTPUT
//=============================================================================
//Output to the stream
template<const int n,const int m,class T>
std::ostream &operator<<(std::ostream & out,const Matrix<n,m,T> &M) {
  typedef typename Matrix<n,m,T>::index_t index_t;
  out << '[' << std::endl;
  for (index_t i=0 ; i < M.rows() ; i++)
    out << M[i] << std::endl;
  out << ']';
  return out;
}
template<class T,int n>
std::ostream &operator<<(std::ostream &out,const Vector<n,T> &v)
{
  typedef typename Vector<n,T>::index_t index_t;
  out << '[';
  for(index_t i=0;i<v.size();i++)
    out<<' '<<v[i];
  out<<" ]";
  return out;
}
//=============================================================================
//Common types
typedef Vector<4> Quat;
typedef Vector<3> Vect;
//=============================================================================
// QUATERNIONS
//=============================================================================
// construct a direction cosine matrix from euler angles in the standard
// rotation sequence [phi][theta][psi] from NED to body frame
// body = tBL(3,3)*NED
extern const Matrix<3,3> eulerDC(const Vector<3> & euler);

// construct a direction cosine matrix from quaternions in the standard
// rotation sequence [phi][theta][psi] from NED to body frame
// body = tBL(3,3)*NED q(4,1)
extern const Matrix<3,3> quatDC(const Quat & q);

//construct the euler omega-cross matrix wx(3,3)
// p, q, r (rad/sec)
extern const Matrix<3,3> eulerWx(const Vector<3> & euler);

// construct the quaternion omega matrix W(4,4)
// p, q, r (rad/sec)
extern const Matrix<4,4> quatW(const Vector<3> euler);

// convert from quaternions to euler angles q(4,1) -> euler[phi;theta;psi] (rad)
extern const Vector<3> quat2euler(const Quat & q);

// convert from euler angles to quaternion vector phi, theta, psi -> q(4,1)
extern const Quat euler2quat(const Vector<3> & euler);

// Functions to compute the partial derivative of the quaterion with
// respect to the Euler angles. These are used for computation of the
// matrix C in the Kalman filter that represents the relationship of
// the measurements to the states.

// Compute the derivative of the Euler angle Phi with respect
// to the quaternion Q. The result is a row vector
// d(phi)/d(q0)
// d(phi)/d(q1)
// d(phi)/d(q2)
// d(phi)/d(q3)
// The DCM is an optimization for repeated calls to the d*_dq
// family of functions. For a single call, you can use quatDC(q ).
extern const Quat dphi_dq(const Quat & q, const Matrix<3,3> & DCM);

// Compute the derivative of the Euler angle Theta with respect
// to the quaternion Q. The result is a row vector
// d(theta)/d(q0)
// d(theta)/d(q1)
// d(theta)/d(q2)
// d(theta)/d(q3)
extern const Quat dtheta_dq(const Quat & q, const Matrix<3,3> & DCM);

// Compute the derivative of the Euler angle Psi with respect
// to the quaternion Q. The result is a row vector
// d(psi)/d(q0)
// d(psi)/d(q1)
// d(psi)/d(q2)
// d(psi)/d(q3)
extern const Quat dpsi_dq(const Quat & q, const Matrix<3,3> & DCM);
//=============================================================================
// MATH
//=============================================================================
// The q-method computes the unit quaternion q = [eta eps1 eps2 eps3]'
// from a set of vector measurements {b1 b2} in the body frame. The
// set {r1 r2} is the corresponding reference vectors in the reference
// frame, and {a1 a2} are weights. It is the solution to an optimization
// problem of minimizing the objective function:
// f(q) = a1*norm(r1-R(q)*b1) + a2*norm(r2-R(q)*b2)
// Input:
// a - weight
// r - reference vector   (NED)
// b - observation vector (BODY)
// 100,1,[0 0 1]',[cos(decl) sin(decl) 0]',-acc/norm(acc),mag/norm(mag)
extern const Quat qmethod(const _mat_float &a1,const _mat_float &a2,const Vect &r1,const Vect &r2,const Vect &b1,const Vect &b2);

// For the quaternion differential equation:
// q_dot = Tquat(q)*w
extern const Matrix<4,3> Tquat(const Quat &q);










//=============================================================================
};//namespace
//=============================================================================
#endif
