/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 *  http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef MANDALATYPES_H
#define MANDALATYPES_H
//=============================================================================
#include<iostream>
#include<stdint.h>
#include<cmath>
//=============================================================================
#define ASSERT(case)
//#define NO_FPU
#ifdef NO_FPU
#undef NO_FPU
#endif
template<const int n,class T=double>class Vector{
protected:
  T v[n];
public:
  typedef unsigned int	index_t;
  Vector(){this->fill();}
  // Should only be valid for Vector<3>
  Vector(const T &s0,const T &s1,const T &s2){(*this)[0]=s0;(*this)[1]=s1;(*this)[2]=s2;}
  //Should only be valid for Vector<4>
  Vector(const T &s0,const T &s1,const T &s2,const T &s3){(*this)[0]=s0;(*this)[1]=s1;(*this)[2]=s2;(*this)[3]=s3;}
  //Convert a vector into a pointer to its data structure.  Yes, thisviolates the basic assumptions about data hiding, but we "feelthe need for speed".
  T *array(){return this->v;}
  const T*array()const{return this->v;}
  // Check for nan or inf
  bool isnan()const{for(index_t i=0;i<n;i++)if(isnan(this->v[i]))return true;return false;}
  void fill(const T &value=T()){for(index_t i=0;i<n;i++)this->v[i]=value;}
  size_t size()const{return n;}
  //Cast to a T to return the square of the magnitude.Call sqrt(v1)if you want the actual magnitude.
  const T mag2()const{return(*this)*(*this);}
  const T mag()const{return std::sqrt(this->mag2());}
  //Add two vectors, returning the sum of the two
  const Vector operator+(const Vector &that)const{return Vector(*this)+=that;}
  //Increment a vector
  Vector &operator+=(const Vector &that){for(index_t i=0;i<n;i++)
#ifdef NO_FPU
    increment((*this)[i], that[i]);
#else
    (*this)[i] +=that[i];
#endif
    return(*this);
  }
  //Subtract two vectors, returning the difference of the two
  const Vector operator-(const Vector &that)const{return Vector(*this)-=that;}
  //Decrement a vector
  Vector &operator-=(const Vector &that){for(index_t i=0;i<n;i++)
#ifdef NO_FPU
    decrement((*this)[i], that[i]);
#else
    (*this)[i] -=that[i];
#endif
    return(*this);
  }

  //Compute the dot product of two vectors
  const T operator *(const Vector &that)const{T dot=T();for(index_t i=0;i<n;i++){
#ifdef NO_FPU
      const T &	this_i((*this)[i]);
      const T &	that_i(that[i]);
      if(is_zero(this_i)
        ||  is_zero(that_i)
        )
        continue;
      dot +=this_i * that_i;
#else
      dot +=(*this)[i] * that[i];
#endif
    }
    return dot;
  }
  //Return the value of the index position.
  T &operator[](unsigned int index){ASSERT(index<n);return this->v[index];}
  const T &operator[](unsigned int index)const{ASSERT(index<n);return this->v[index];}
  //Multiply a vector by a scalar, returning the product
  const Vector operator*(const T &scale)const{return Vector(*this)*=scale;}
  const Vector operator/(const T &scale)const{return Vector(*this)/=scale;}
  //Scale a vector by a scalar, updating the vector.
  Vector &operator*=(const T &	scale){for(index_t i=0;i<n;i++){
#ifdef NO_FPU
      T &		t_i((*this)[i]);
      if(is_zero(t_i))
        continue;
      t_i *=scale;
#else
      (*this)[i] *=scale;
#endif
    }
    return (*this);
  }
  //set vector to v,v,v
  Vector & operator=(const double v){for(index_t i=0;i<n;i++)(*this)[i]=v;return(*this);}
  Vector &operator/=(const T &	scale){for(index_t i=0;i<n;i++){
#ifdef NO_FPU
      T &t_i((*this)[i]);
      if(is_zero(t_i))
        continue;
      t_i /=scale;
#else
      (*this)[i] /=scale;
#endif
    }
    return(*this);
  }
  //Normalize a vector
  const Vector norm()const{return(*this)/ this->mag();}
  Vector &norm_self(){return(*this)/=this->mag();}

  //compare vectors
  bool operator==(const Vector &v)const{for(index_t i=0;i<n;i++)if((*this)[i]!=v[i])return false;return true;}
  bool operator!=(const Vector &v)const{for(index_t i=0;i<n;i++)if((*this)[i]==v[i])return false;return true;}
};
//Multiplication is commutative.
template<const int n,class T1,class T2>Vector<n,T1>&operator *(const T2 &s,const Vector<n,T1>&v){return v * s;}
template<class T,int n>std::ostream &operator<<(std::ostream &out,const Vector<n,T>&v)
{
  typedef typename Vector<n,T>::index_t index_t;
  out<<'[';
  for(index_t i=0;i<v.size();i++)
    out<<' '<<v[i];
  out<<" ]";
  return out;
}
//We only have cross products for 3D vectors.It is not a member function since C++ doesn't allow
template<class T>const Vector<3,T>cross(const Vector<3,T>&a,const Vector<3,T>&b){return Vector<3,T>(a[1]*b[2] - a[2]*b[1],a[2]*b[0] - a[0]*b[2],a[0]*b[1] - a[1]*b[0]);}
//Define a few common types
typedef Vector<4>Quat;
//Splice extracts a portion of a vector
template<const int size,const int n,class T>const Vector<size,T>slice(const Vector<n,T>&v,const int start)
{
  typedef typename Vector<n,T>::index_t index_t;
  Vector<size,T>out;
  for(index_t i=0;i<size;i++)out[i]=v[i+start];
  return out;
}
//insert inserts a vector into a larger vector and returnsthe larger vector.
template<const int n,class T1,const int size,class T2>Vector<n,T1>&insert(Vector<n,T1>&v,int start,const Vector<size,T2>&in)
{
  typedef typename Vector<n,T1>::index_t index_t;
  for(index_t i=0;i<size;i++)v[i + start]=in[i];
  return v;
}
//=============================================================================
typedef Vector<3>Vect;
#endif
//=============================================================================
//=============================================================================
