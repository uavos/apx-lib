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
#include <cmath>
//=============================================================================
namespace matrixmath {

typedef float _mat_float;
typedef int index_t;

//=============================================================================
// VECTOR
//=============================================================================
template<const index_t n = 3, class T = _mat_float>
class Vector
{
public:
    typedef T _array[n];
    _array v;

#ifdef MATH_CHECK_NAN
    void fixNAN(Vector *value) const
    {
        for (index_t i = 0; i < n; i++)
            fixNAN(&(*value)[i]);
    }
    void fixNAN(T *value) const
    {
        if (!f_isvalid(*value))
            (*value) = 0;
    }
#else
#define fixNAN(...)
#endif

    Vector() { this->fill(); }
    Vector(const T &s) { this->fill(s); }
    Vector(const T &s0, const T &s1)
    {
        (*this)[0] = s0;
        (*this)[1] = s1;
        fixNAN(this);
    }
    Vector(const T &s0, const T &s1, const T &s2)
    {
        (*this)[0] = s0;
        (*this)[1] = s1;
        (*this)[2] = s2;
        fixNAN(this);
    }
    Vector(const T &s0, const T &s1, const T &s2, const T &s3)
    {
        (*this)[0] = s0;
        (*this)[1] = s1;
        (*this)[2] = s2;
        (*this)[3] = s3;
        fixNAN(this);
    }

    inline T *array() { return this->v; }
    inline const T *array() const { return this->v; }

    void fill(const T &value = T())
    {
        for (index_t i = 0; i < n; i++)
            this->v[i] = value;
        fixNAN(this);
    }

    size_t size() const { return n; }

    bool isNull(void)
    {
        for (index_t i = 0; i < n; i++)
            if ((*this)[i] != 0.0)
                return false;
        return true;
    }
    /*bool isWithin(const T &value){
    for (index_t i=0 ; i < n ; i++)
      if(std::abs((*this)[i])>value)return false;
    return true;
  }
  bool isWithin(const Vector &cmp){
    for (index_t i=0 ; i < n ; i++)
      if(std::abs((*this)[i])>cmp[i])return false;
    return true;
  }*/

    //compare
    bool operator==(const Vector &cmp) const
    {
        for (index_t i = 0; i < n; i++)
            if ((*this)[i] != cmp[i])
                return false;
        return true;
    }
    bool operator!=(const Vector &cmp) const { return !((*this) == cmp); }
    //Magnitude
    inline const T mag2() const { return (*this) * (*this); }
    inline const T mag() const { return std::sqrt(this->mag2()); }

    //add vectors
    inline const Vector operator+(const Vector &that) const { return Vector(*this) += that; }
    Vector &operator+=(const Vector &that)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] += that[i];
        fixNAN(this);
        return (*this);
    }

    //substract vectors
    inline const Vector operator-(const Vector &that) const { return Vector(*this) -= that; }
    Vector &operator-=(const Vector &that)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] -= that[i];
        fixNAN(this);
        return (*this);
    }

    //Negate a vector
    inline const Vector operator-() const
    {
        Vector V;
        for (index_t i = 0; i < n; i++)
            V[i] = -(*this)[i];
        fixNAN(&V);
        return V;
    }

    //multiply vectors (dot product)
    inline const T operator*(const Vector &that) const
    {
        T dot = T();
        for (index_t i = 0; i < n; i++)
            dot += (*this)[i] * that[i];
        fixNAN(&dot);
        return dot;
    }

    //acess by index
    T &operator[](index_t index) { return this->v[index]; }
    const T &operator[](index_t index) const { return this->v[index]; }

    //multiply to scalar
    inline const Vector operator*(const T &scale) const { return Vector(*this) *= scale; }
    inline const Vector operator/(const T &scale) const
    {
        if (scale == 0)
            return Vector(*this) *= scale;
        return Vector(*this) /= scale;
    }
    inline Vector &operator*=(const T &scale)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] *= scale;
        fixNAN(this);
        return (*this);
    }
    inline Vector &operator/=(const T &scale)
    {
        if (scale == 0)
            (*this).fill(0);
        else
            for (index_t i = 0; i < n; i++)
                (*this)[i] /= scale;
        fixNAN(this);
        return (*this);
    }

    //Normalize a vector
    inline const Vector norm() const
    {
        T m = this->mag();
        if (m == 0)
            return (*this) * m;
        return (*this) / m;
    }
    inline Vector &norm_self()
    {
        T m = this->mag();
        if (m == 0)
            return (*this) *= m;
        return (*this) /= m;
    }

    //limits & angles
    Vector &limit(const float &min, const float &max)
    {
        for (index_t i = 0; i < n; i++) {
            if ((*this)[i] < min)
                (*this)[i] = min;
            else if ((*this)[i] > max)
                (*this)[i] = max;
        }
        return (*this);
    }
    Vector &limit(const float &lim = 1) { return limit(-lim, lim); }
    Vector &bound(const float &span = 180.0)
    {
        const _mat_float dspan = span * 2.0;
        for (index_t i = 0; i < n; i++) {
            (*this)[i] = (*this)[i] - std::floor((*this)[i] / dspan + 0.5) * dspan;
        }
        fixNAN(this);
        return (*this);
    }

}; //Vector class
//------------------------------------------------------------------------------
//Multiplication is commutative.
template<const index_t n, class T1, class T2>
Vector<n, T1> operator*(const T2 &s, const Vector<n, T1> &v)
{
    return v * s;
}

//cross product
template<class T>
const Vector<3, T> cross(const Vector<3, T> &a, const Vector<3, T> &b)
{
    return Vector<3, T>(a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]);
}

//Splice extracts a portion of a vector
template<const index_t size, const index_t n, class T>
const Vector<size, T> slice(const Vector<n, T> &v, const index_t start)
{
    Vector<size, T> out;
    for (index_t i = 0; i < size; i++)
        out[i] = v[i + start];
    return out;
}

//insert a vector index_to a larger vector and returns the larger vector
template<const index_t n, class T1, const index_t size, class T2>
Vector<n, T1> &insert(Vector<n, T1> &v, index_t start, const Vector<size, T2> &in)
{
    for (index_t i = 0; i < size; i++)
        v[i + start] = in[i];
    return v;
}
//=============================================================================
// MATRIX
//=============================================================================
template<const index_t n, const index_t m, class T = _mat_float> //rows,cols
class Matrix
{
public:
    Vector<m, T> M[n];
    Matrix() {}
    Matrix(const T &value) { this->fill(value); }
    Matrix(const Vector<m> &v0) { M[0] = v0; }
    Matrix(const Vector<m> &v0, const Vector<m> &v1, const Vector<m> &v2)
    {
        M[0] = v0;
        M[1] = v1;
        M[2] = v2;
    }
    Matrix(const Vector<m> &v0, const Vector<m> &v1, const Vector<m> &v2, const Vector<m> &v3)
    {
        M[0] = v0;
        M[1] = v1;
        M[2] = v2;
        M[3] = v3;
    }
    T *array() { return this->M[0].array(); }
    const T *array() const { return this->M[0].array(); }
    size_t rows() const { return n; }
    size_t cols() const { return m; }
    void fill(const T &value = T())
    {
        for (index_t i = 0; i < n; i++)
            this->row(i).fill(value);
    }

    //fill elements of matrix
    void fill(index_t base_n, index_t base_m, index_t cnt_n, index_t cnt_m, const T &value = T())
    {
        cnt_n += base_n;
        cnt_m += base_m;
        for (index_t i = base_n; i < cnt_n; i++)
            for (index_t j = base_m; j < cnt_m; j++)
                (*this)[i][j] = value;
    }
    //Insert a submatrix index_to the larger matrix
    template<class T2, index_t n2, index_t m2>
    void fill(index_t base_n, index_t base_m, const Matrix<n2, m2, T2> &M_i)
    {
        for (index_t i = 0; i < n2; i++)
            for (index_t j = 0; j < m2; j++)
                (*this)[i + base_n][j + base_m] = M_i[i][j];
    }
    //fill column
    void fill(index_t col_i, const Vector<n, T> &v)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i][col_i] = v[i];
    }

    //row
    Vector<m, T> &operator[](index_t row_i) { return this->row(row_i); }
    const Vector<m, T> &operator[](index_t row_i) const { return this->row(row_i); }
    const Vector<m, T> &row(index_t row_i) const { return this->M[row_i]; }
    Vector<m, T> &row(index_t row_i) { return this->M[row_i]; }

    //column
    const Vector<n, T> col(index_t col_i) const
    {
        Vector<n, T> v;
        for (index_t i = 0; i < n; i++)
            v[i] = (*this)[i][col_i];
        return v;
    }

    //Negate a matrix
    void negate()
    {
        for (index_t i = 0; i < n; i++)
            for (index_t j = 0; j < m; j++)
                (*this)[i][j] = -(*this)[i][j];
    }

    //Add two matrixes, returning the sum of the two
    //const Matrix operator+ (const Matrix & that) const { Matrix sr(*this); sr+= that; return  sr; }
    //const Matrix operator- (const Matrix & that) const { Matrix sr(*this); sr-= that; return  sr; }

    //Update the matrix in place
    Matrix &operator+=(const Matrix &that)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] += that[i];
        return (*this);
    }
    Matrix &operator-=(const Matrix &that)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] -= that[i];
        return (*this);
    }

    //multiplication
    /*template<index_t p>
  const Matrix<n,p,T> operator*(const Matrix<m,p,T> & B) const {
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
  }*/

    //=================================
    //optimized operations
    //=================================
    template<index_t p>
    void mult(const Matrix<n, p, T> &A, const Matrix<p, m, T> &B)
    {
        Matrix &C(*this);
        for (index_t i = 0; i < n; i++) {
            const Vector<p, T> &A_i = A[i];
            Vector<m, T> &C_i = C[i];
            for (index_t j = 0; j < m; j++) {
                T s = T();
                for (index_t k = 0; k < p; k++) {
                    s += B[k][j] * A_i[k];
                }
                C_i[j] = s;
            }
        }
    }
    template<index_t p> //this=A*B_T
    void mult_T(const Matrix<n, p, T> &A, const Matrix<m, p, T> &B)
    {
        Matrix &C(*this);
        for (index_t i = 0; i < n; i++) {
            const Vector<p, T> &A_i = A[i];
            Vector<m, T> &C_i = C[i];
            for (index_t j = 0; j < m; j++) {
                T s = T();
                for (index_t k = 0; k < p; k++) {
                    s += B[j][k] * A_i[k];
                }
                C_i[j] = s;
            }
        }
    }
    template<index_t p, index_t pm> //this=A*B*A_T
    void mult_3T(const Matrix<n, p, T> &A, const Matrix<pm, p, T> &B, Matrix<n, p, T> &tmp)
    {
        tmp.mult(A, B);
        mult_T(tmp, A);
    }

    //Matrix<n,n> = Vector<n> * transpose(Vector<n>)
    void mult(const Vector<n, T> &a, const Vector<n, T> &b)
    {
        for (index_t i = 0; i < n; i++)
            for (index_t j = 0; j < n; j++)
                (*this)[i][j] = a[i] * b[j];
    }

    void transposed(const Matrix<m, n, T> &A)
    {
        Matrix<n, m, T> &B(*this);
        for (index_t i = 0; i < n; i++) {
            const Vector<m, T> &A_i(A[i]);
            for (index_t j = 0; j < m; j++)
                B[j][i] = A_i[j];
        }
    }

    inline void eulerWx(const Vector<3> &euler)
    {
        const _mat_float &p = euler[0];
        const _mat_float &q = euler[1];
        const _mat_float &r = euler[2];
        Matrix<3, 3> &C(*this);
        C[0] = Vector<3>(0, -r, q);
        C[1] = Vector<3>(r, 0, -p);
        C[2] = Vector<3>(-q, p, 0);
    }

    const Matrix<3, 3> Wmtrx(const Vector<3> &eps, const Vector<3> &v, Matrix<3, 3> &tmp, Matrix<3, 3> &tmp2)
    {
        const _mat_float eps2 = eps * eps;
        const _mat_float eta = eps2 >= 1 ? 0 : std::sqrt(1 - eps2);
        Matrix<3, 3> &W(*this);
        if (eta != 0) {
            tmp.eulerWx(v * (-2.0 / eta));
            tmp2.mult(eps, eps);
            W.mult(tmp, tmp2);
        } else
            W.fill();
        tmp.eulerWx(v * (2.0 * eta));
        W += tmp;
        W.eye_add(2.0 * (v * eps));
        tmp.mult(eps, v);
        tmp *= 2;
        W += tmp;
        tmp.mult(v, eps);
        tmp *= -4;
        W += tmp;
        return W;
    }

    /*void Wmtrx(const Vector<3> &eps,const Vector<3> &v)
  {
    const _mat_float eps2=eps*eps;
    const _mat_float eta=eps2>=1?0:std::sqrt(1-eps2);
    Matrix<3,3> &W(*this);
    Matrix<3,3> tmp;//=eulerWx(v);
    const _mat_float & p = v[0];
    const _mat_float & q = v[1];
    const _mat_float & r = v[2];
    return Matrix<3,3>tmp(
      Vector<3>(0, -r, q),
                       Vector<3>(r, 0, -p),
                       Vector<3>(-q, p, 0)
    );

    W=tmp;
    W*=(2.0*eta);
    tmp*=(eta==0?0:(-2.0/eta));
    tmp*=mult_T(eps,eps);
    W+=tmp;
    W.eye_add(2.0*(v*eps));
    tmp=mult_T(eps,v);
    tmp*=2;
    W+=tmp;
    tmp=mult_T(v,eps);
    tmp*=-4;
    W+=tmp;
    return W;
  }*/

    //Scale
    //const Matrix operator * (const T & s) const { Matrix sr(*this); sr*= s; return  sr; }
    Matrix &operator*=(const T &s)
    {
        for (index_t i = 0; i < n; i++)
            (*this)[i] *= s;
        return (*this);
    }

    //transpose of the matrix
    /*const Matrix<m,n,T> transposed() const {
    const Matrix<n,m,T> & A(*this);
    Matrix<m,n,T> B;
    for (index_t i=0 ; i<n ; i++) {
      const Vector<m,T> & A_i(A[i]);
      for (index_t j=0 ; j<m ; j++)
        B[j][i] = A_i[j];
    }
    return B;
  }*/

    //Frobenius norm
    T frobenius_norm() const
    {
        T v = T();
        const Matrix<n, m, T> &A(*this);
        for (index_t i = 0; i < n; i++) {
            const Vector<m, T> &A_i(A[i]);
            v = v + A_i.mag2();
        }
        return std::sqrt(v);
    }
    // Make a square identity matrix
    void eye(const T &value = T(1))
    {
        Matrix<n, m, T> &A(*this);
        for (index_t i = 0; i < n; i++)
            for (index_t j = 0; j < m; j++)
                A[i][j] = i == j ? value : T();
    }
    // Add a square identity matrix
    void eye_add(const T &value = T(1))
    {
        Matrix<n, m, T> &A(*this);
        for (index_t i = 0; i < n; i++)
            A[i][i] += value;
    }
    // construct a direction cosine matrix from quaternions in the standard
    // rotation sequence [phi][theta][psi] from NED to body frame
    // body = tBL(3,3)*NED q(4,1)
    void quatDC(const Vector<4> &q)
    {
        const _mat_float &q0 = q[0];
        const _mat_float &q1 = q[1];
        const _mat_float &q2 = q[2];
        const _mat_float &q3 = q[3];
        Matrix<n, m, T> &A(*this);
        Vector<3> &A0 = A[0];
        Vector<3> &A1 = A[1];
        Vector<3> &A2 = A[2];
        A0[0] = 1.0 - 2 * (q2 * q2 + q3 * q3);
        A0[1] = 2 * (q1 * q2 + q0 * q3);
        A0[2] = 2 * (q1 * q3 - q0 * q2);
        A1[0] = 2 * (q1 * q2 - q0 * q3);
        A1[1] = 1.0 - 2 * (q1 * q1 + q3 * q3);
        A1[2] = 2 * (q2 * q3 + q0 * q1);
        A2[0] = 2 * (q1 * q3 + q0 * q2);
        A2[1] = 2 * (q2 * q3 - q0 * q1);
        A2[2] = 1.0 - 2 * (q1 * q1 + q2 * q2);
    }
    void quatDC_T(const Vector<4> &q)
    { //transposed
        const _mat_float &q0 = q[0];
        const _mat_float &q1 = q[1];
        const _mat_float &q2 = q[2];
        const _mat_float &q3 = q[3];
        Matrix<n, m, T> &A(*this);
        Vector<3> &A0 = A[0];
        Vector<3> &A1 = A[1];
        Vector<3> &A2 = A[2];
        A0[0] = 1.0 - 2 * (q2 * q2 + q3 * q3);
        A0[1] = 2 * (q1 * q2 - q0 * q3);
        A0[2] = 2 * (q1 * q3 + q0 * q2);
        A1[0] = 2 * (q1 * q2 + q0 * q3);
        A1[1] = 1.0 - 2 * (q1 * q1 + q3 * q3);
        A1[2] = 2 * (q2 * q3 - q0 * q1);
        A2[0] = 2 * (q1 * q3 - q0 * q2);
        A2[1] = 2 * (q2 * q3 + q0 * q1);
        A2[2] = 1.0 - 2 * (q1 * q1 + q2 * q2);
    }

    // Compute the LU factorization of a square matrix * A is modified, so we pass by value.
    void LU(Matrix<n, n, T> &L, Matrix<n, n, T> &U)
    {
        Matrix<n, n, T> &A(*this);
        for (index_t k = 0; k < n - 1; k++) {
            for (index_t i = k + 1; i < n; i++) {
                A[i][k] = A[i][k] / A[k][k];
                for (index_t j = k + 1; j < n; j++) {
                    A[i][j] -= A[i][k] * A[k][j];
                }
            }
        }
        L.eye(); // Separate the L matrix
        for (index_t j = 0; j < n - 1; j++)
            for (index_t i = j + 1; i < n; i++)
                L[i][j] = A[i][j]; // Separate the M matrix
        U.fill();
        for (index_t i = 0; i < n; i++)
            for (index_t j = i; j < n; j++)
                U[i][j] = A[i][j];
    }

}; //Matrix
//------------------------------------------------------------------------------
//Vector<n> = Matrix<n,m> * Vector<m>
template<const index_t n, const index_t m, class T>
const Vector<n, T> operator*(const Matrix<n, m, T> &A, const Vector<m, T> &b)
{
    Vector<3, T> c;
    for (index_t i = 0; i < n; i++)
        c[i] = A[i] * b;
    return c;
}

//Vector<m> = Vector<n> * Matrix<n,m>
/*template<const index_t n,const index_t m,class T>
const Vector<m,T> operator*(const Vector<n,T> & a,const Matrix<n,m,T> & B) {
  Vector<m,T> c;
  for (index_t i=0 ; i<m ; i++) {
    T s(0);
    for (index_t j=0 ; j<n ; j++)
      s += a[j] * B[j][i];
    c[i] = s;
  }
  return c;
}*/

//sum of the diagonal elements in a square matrix
/*template<const index_t N,class T>
_mat_float trace(const Matrix<N,N,T> & m) {
  _mat_float t = 0;
  for (index_t i=0 ; i<N ; i++)
    t += m[i][i];
  return t;
}*/

//diagonal matrix
/*template<const index_t N,class T>
const Matrix<N,N,T> diag(const Vector<N,T> & v) {
  Matrix<N,N,T> m;
  for (index_t i=0 ; i<N ; i++)
    m[i][i]=v[i];
  return m;
}*/

// Make a square identity matrix
/*template< const index_t n, class T >
const Matrix<n,n,T> eye() {
    Matrix<n,n,T> A;
    for ( index_t i=0 ; i<n ; i++ ) A[i][i] = T(1);
    return A;
}*/
// Compute the LU factorization of a square matrix * A is modified, so we pass by value.
/*template<const index_t n, class T >
void LU( Matrix<n,n,T> &A, Matrix<n,n,T> & L, Matrix<n,n,T> & U ) {
    for ( index_t k=0 ; k<n-1 ; k++ ) {
        for ( index_t i=k+1 ; i<n ; i++ ) {
            A[i][k] = A[i][k] / A[k][k];
            for ( index_t j=k+1 ; j<n ; j++ ) {
                A[i][j] -= A[i][k] * A[k][j];
            }
        }
    }
    L.eye();
    for ( index_t j=0 ; j<n-1 ; j++ )
      for ( index_t i=j+1 ; i<n ; i++ )
        L[i][j] = A[i][j];
    U.fill();
    for ( index_t i=0 ; i<n ; i++ )
      for ( index_t j=i ; j<n ; j++ )
        U[i][j] = A[i][j];
}*/
// Invert a matrix using LU. * Special case for a 1x1 and 2x2 matrix first
/*template< class T >
const Matrix<1,1,T> invert( const Matrix<1,1,T> & A ) {
    Matrix<1,1,T> B;
    B[0][0] = T(1) / A[0][0];
    return B;
}
template< class T >
const Matrix<2,2,T> invert( const Matrix<2,2,T> & A ) {
    Matrix<2,2,T> B;
    const T det( A[0][0] * A[1][1] - A[0][1] * A[1][0] );
    B[0][0] = A[1][1] / det;
    B[0][1] = -A[0][1] / det;
    B[1][0] = -A[1][0] / det;
    B[1][1] = A[0][0] / det;
    return B;
}*/
/*template< const index_t n, class T >
const Vector<n,T> solve_upper( const Matrix<n,n,T> & A, const Vector<n,T> & b ) {
    Vector<n,T> x;
    for ( index_t i=n-1 ; i>=0 ; i-- ) {
        T s( b[i] );
        const Vector<n,T> & A_i( A[i] );
        for ( index_t j=i+1 ; j<n ; ++j ) {
            s -= A_i[j]*x[j];
        } x[i] = s / A_i[i];
    } return x;
}*/
/*template< const index_t n, class T >
const Vector<n,T> solve_lower( const Matrix<n,n,T> & A, const Vector<n,T> & b ) {
    Vector<n,T> x;
    for ( index_t i=0; i<n; ++i) {
        T s( b[i] );
        const Vector<n,T> & A_i( A[i] );
        for ( index_t j=0; j<i; ++j) {
            s -= A_i[j] * x[j];
        } x[i] = s / A_i[i];
    } return x;
}*/
/*template< const index_t n, class T >
const Matrix<n,n,T> invert( const Matrix<n,n,T> & M ) {
    typedef Matrix<n,n,T> Matrix;
    typedef Vector<n,T> Vector;
    Matrix L;
    Matrix U;
    Matrix invU;
    Matrix invL;
    Vector identCol;
    LU( M, L, U );
    for ( index_t i=0 ; i<n ; i++ ) {
        identCol[i] = T(1);
        invU.col( i, solve_upper( U, identCol ) );
        invL.col( i, solve_lower( L, identCol ) );
        identCol[i] = T(0);
    } return invU * invL;
}*/
//=============================================================================
// DEBUG OUTPUT
//=============================================================================
//Output to the stream
/*template<const index_t n,const index_t m,class T>
std::ostream &operator<<(std::ostream & out,const Matrix<n,m,T> &M) {
  out << '[' << std::endl;
  for (index_t i=0 ; i < M.rows() ; i++)
    out << M[i] << std::endl;
  out << ']';
  return out;
}
template<class T,index_t n>
std::ostream &operator<<(std::ostream &out,const Vector<n,T> &v)
{
  out << '[';
  for(index_t i=0;i<v.size();i++)
    out<<' '<<v[i];
  out<<" ]";
  return out;
}*/
//=============================================================================
//Common types
//typedef Vector<4> Quat;
//typedef Vector<3> Vect3;
//typedef Vector<2> Point;
//=============================================================================
// QUATERNIONS
//=============================================================================
class Quat : public Vector<4>
{
public:
    Quat()
        : Vector<4>()
    {
        reset();
    }
    Quat(const _mat_float &w, const _mat_float &s1, const _mat_float &s2, const _mat_float &s3)
    {
        (*this)[0] = w;
        (*this)[1] = s1;
        (*this)[2] = s2;
        (*this)[3] = s3;
    }

    Quat(const Vector<3> &euler) { fromEuler(euler); }

    Quat(const Vector<3> &axis, _mat_float angle)
    {
        _mat_float angle2 = angle / 2.0;
        _mat_float s = std::sin(angle2);
        (*this)[0] = std::cos(angle2);
        (*this)[1] = axis[0] * s;
        (*this)[2] = axis[1] * s;
        (*this)[3] = axis[2] * s;
    }

    Quat &reset()
    {
        (*this)[0] = 1;
        (*this)[1] = 0;
        (*this)[2] = 0;
        (*this)[3] = 0;
        return (*this);
    }

    bool isReset() { return (*this)[0] == 1 && (*this)[1] == 0 && (*this)[2] == 0 && (*this)[3] == 0; }

    Quat &qbuild(const Vector<3> &eps)
    {
        const _mat_float eps2 = 1.0 - eps * eps;
        const _mat_float eta = eps2 <= 0 ? 0 : std::sqrt(eps2);
        (*this)[0] = eta;
        (*this)[1] = eps[0];
        (*this)[2] = eps[1];
        (*this)[3] = eps[2];
        return (*this);
    }

    Quat &qmult(const Quat &q);      //rotate arg by this quaternion
    const Vector<3> toEuler() const; //Aerospace
    Quat &fromEuler(const Vector<3> &euler);
    Quat &conjugate(void); //rotate back
    inline Quat &normalize(void)
    {
        norm_self();
        return (*this);
    }

    //euler angles conversions
    const Vector<3> toEuler(int order) const;
    void fromEuler(Vector<3> euler, int order);

    void euler2HMatrix(Vector<3> euler, int order);
    const Vector<3> HMatrix2euler(int order) const;

    static _mat_float HMatrix[4][4];
#define EulOrd(i, p, r, f) (((((((i) << 1) + (p)) << 1) + (r)) << 1) + (f))
    enum { EulFrmS = 0, EulFrmR = 1, EulRepNo = 0, EulRepYes = 1, EulParEven = 0, EulParOdd = 1 };
    typedef enum {
        /* Static axes */
        EulOrdXYZs = EulOrd(0, EulParEven, EulRepNo, EulFrmS), //Aerospace
        EulOrdXYXs = EulOrd(0, EulParEven, EulRepYes, EulFrmS),
        EulOrdXZYs = EulOrd(0, EulParOdd, EulRepNo, EulFrmS),
        EulOrdXZXs = EulOrd(0, EulParOdd, EulRepYes, EulFrmS),
        EulOrdYZXs = EulOrd(1, EulParEven, EulRepNo, EulFrmS),
        EulOrdYZYs = EulOrd(1, EulParEven, EulRepYes, EulFrmS),
        EulOrdYXZs = EulOrd(1, EulParOdd, EulRepNo, EulFrmS),
        EulOrdYXYs = EulOrd(1, EulParOdd, EulRepYes, EulFrmS),
        EulOrdZXYs = EulOrd(2, EulParEven, EulRepNo, EulFrmS),
        EulOrdZXZs = EulOrd(2, EulParEven, EulRepYes, EulFrmS),
        EulOrdZYXs = EulOrd(2, EulParOdd, EulRepNo, EulFrmS),
        EulOrdZYZs = EulOrd(2, EulParOdd, EulRepYes, EulFrmS),
        /* Rotating axes */
        EulOrdZYXr = EulOrd(0, EulParEven, EulRepNo, EulFrmR),
        EulOrdXYXr = EulOrd(0, EulParEven, EulRepYes, EulFrmR),
        EulOrdYZXr = EulOrd(0, EulParOdd, EulRepNo, EulFrmR),
        EulOrdXZXr = EulOrd(0, EulParOdd, EulRepYes, EulFrmR),
        EulOrdXZYr = EulOrd(1, EulParEven, EulRepNo, EulFrmR),
        EulOrdYZYr = EulOrd(1, EulParEven, EulRepYes, EulFrmR),
        EulOrdZXYr = EulOrd(1, EulParOdd, EulRepNo, EulFrmR),
        EulOrdYXYr = EulOrd(1, EulParOdd, EulRepYes, EulFrmR),
        EulOrdYXZr = EulOrd(2, EulParEven, EulRepNo, EulFrmR),
        EulOrdZXZr = EulOrd(2, EulParEven, EulRepYes, EulFrmR),
        EulOrdXYZr = EulOrd(2, EulParOdd, EulRepNo, EulFrmR),
        EulOrdZYZr = EulOrd(2, EulParOdd, EulRepYes, EulFrmR)
    } _euler_order;
};
// construct a direction cosine matrix from euler angles in the standard
// rotation sequence [phi][theta][psi] from NED to body frame
// body = tBL(3,3)*NED
//extern const Matrix<3,3> eulerDC(const Vector<3> & euler);

// construct a direction cosine matrix from quaternions in the standard
// rotation sequence [phi][theta][psi] from NED to body frame
// body = tBL(3,3)*NED q(4,1)
//extern const Matrix<3,3> quatDC(const Quat & q);

//construct the euler omega-cross matrix wx(3,3)
// p, q, r (rad/sec)
//extern const Matrix<3,3> eulerWx(const Vector<3> & euler);

// construct the quaternion omega matrix W(4,4)
// p, q, r (rad/sec)
//extern const Matrix<4,4> quatW(const Vector<3> euler);

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
//extern const Quat dphi_dq(const Quat & q, const Matrix<3,3> & DCM);

// Compute the derivative of the Euler angle Theta with respect
// to the quaternion Q. The result is a row vector
// d(theta)/d(q0)
// d(theta)/d(q1)
// d(theta)/d(q2)
// d(theta)/d(q3)
//extern const Quat dtheta_dq(const Quat & q, const Matrix<3,3> & DCM);

// Compute the derivative of the Euler angle Psi with respect
// to the quaternion Q. The result is a row vector
// d(psi)/d(q0)
// d(psi)/d(q1)
// d(psi)/d(q2)
// d(psi)/d(q3)
//extern const Quat dpsi_dq(const Quat & q, const Matrix<3,3> & DCM);
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
// 100,1,[0 0 1]',[std::cos(decl) std::sin(decl) 0]',-acc/norm(acc),mag/norm(mag)
//extern const Quat qmethod(const _mat_float &a1,const _mat_float &a2,const Vect &r1,const Vect &r2,const Vect &b1,const Vect &b2);

// For the quaternion differential equation:
// q_dot = Tquat(q)*w
//extern const Matrix<4,3> Tquat(const Quat &q);

// Jacobian of Transposed rotation matrix:
// Returns the partial derivative of  R(q)'*v  with
// respect to eps, when q = [std::sqrt(1-eps'*eps); eps].
//extern const Matrix<3,3> Wmtrx(const Vect &eps,const Vect &v);

// For the quaternion differential equation:
// q_dot = Omega(w)*q;
//extern const Matrix<4,4> Omega(const Vect &w);

//=============================================================================
}; // namespace matrixmath
//=============================================================================
#endif
