/*
 * Copyright(C) 2012 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 * http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 *(at your option) any later version.
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
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <dmsg.h>
#include "MatrixMath.h"
//=============================================================================
namespace matrixmath {
//=============================================================================
const Matrix<3,3> eulerDC(const Vector<3> & euler) {
  const _mat_float & phi = euler[0];
  const _mat_float & theta = euler[1];
  const _mat_float & psi = euler[2];
  const _mat_float cpsi = cos(psi);
  const _mat_float cphi = cos(phi);
  const _mat_float ctheta = cos(theta);
  const _mat_float spsi = sin(psi);
  const _mat_float sphi = sin(phi);
  const _mat_float stheta = sin(theta);
  return Matrix<3,3>(
           Vector<3>(cpsi*ctheta, spsi*ctheta, -stheta),
           Vector<3>(-spsi*cphi + cpsi*stheta*sphi, cpsi*cphi + spsi*stheta*sphi, ctheta*sphi),
           Vector<3>(spsi*sphi + cpsi*stheta*cphi, -cpsi*sphi + spsi*stheta*cphi, ctheta*cphi)
         );
}
/*const Matrix<3,3> quatDC(const Quat & q) {
  const _mat_float & q0 = q[0];
  const _mat_float & q1 = q[1];
  const _mat_float & q2 = q[2];
  const _mat_float & q3 = q[3];
  return Matrix<3,3>(
           Vector<3>(1.0-2*(q2*q2 + q3*q3),2*(q1*q2 + q0*q3),2*(q1*q3 - q0*q2)),
           Vector<3>(2*(q1*q2 - q0*q3),1.0-2*(q1*q1 + q3*q3),2*(q2*q3 + q0*q1)),
           Vector<3>(2*(q1*q3 + q0*q2),2*(q2*q3 - q0*q1),1.0-2*(q1*q1 + q2*q2))
         );
}*/
/*const Matrix<3,3> eulerWx(const Vector<3> & euler) {
  const _mat_float & p = euler[0];
  const _mat_float & q = euler[1];
  const _mat_float & r = euler[2];
  return Matrix<3,3>(
           Vector<3>(0, -r, q),
           Vector<3>(r, 0, -p),
           Vector<3>(-q, p, 0)
         );
}*/
const Matrix<4,4> quatW(const Vector<3> euler) {
  const _mat_float p = euler[0] / 2.0;
  const _mat_float q = euler[1] / 2.0;
  const _mat_float r = euler[2] / 2.0;
  return Matrix<4,4>(
           Vector<4>(0, -p, -q, -r),
           Vector<4>(p, 0, r, -q),
           Vector<4>(q, -r, 0, p),
           Vector<4>(r, q, -p, 0)
         );
}
const Vector<3> quat2euler(const Quat & q) {
  const _mat_float & q0 = q[0];
  const _mat_float & q1 = q[1];
  const _mat_float & q2 = q[2];
  const _mat_float & q3 = q[3];
  _mat_float theta = -asin(2*(q1*q3 - q0*q2));
  _mat_float phi = atan2(2.0*(q2*q3 + q0*q1), 1.0-2.0*(q1*q1 + q2*q2));
  _mat_float psi = atan2(2.0*(q1*q2 + q0*q3), 1.0-2.0*(q2*q2 + q3*q3));
  return Vector<3>(phi, theta, psi);
}
const Quat euler2quat(const Vector<3> & euler) {
  const _mat_float phi = euler[0] / 2.0;
  const _mat_float theta = euler[1] / 2.0;
  const _mat_float psi = euler[2] / 2.0;
  const _mat_float shphi0 = sin(phi);
  const _mat_float chphi0 = cos(phi);
  const _mat_float shtheta0 = sin(theta);
  const _mat_float chtheta0 = cos(theta);
  const _mat_float shpsi0 = sin(psi);
  const _mat_float chpsi0 = cos(psi);
  return Quat(
           chphi0 * chtheta0 * chpsi0 + shphi0 * shtheta0 * shpsi0,
           -chphi0 * shtheta0 * shpsi0 + shphi0 * chtheta0 * chpsi0,
           chphi0 * shtheta0 * chpsi0 + shphi0 * chtheta0 * shpsi0,
           chphi0 * chtheta0 * shpsi0 - shphi0 * shtheta0 * chpsi0
         );
}
const Vector<4> dphi_dq(const Vector<4> & quat, const Matrix<3,3> & DCM) {
  const _mat_float q0 = quat[0];
  const _mat_float q1 = quat[1];
  const _mat_float q2 = quat[2];
  const _mat_float q3 = quat[3];
  const _mat_float dcm22 = DCM[2][2];
  const _mat_float dcm12 = DCM[1][2];
  const _mat_float err = 2 /(dcm22*dcm22 + dcm12*dcm12);
  return Vector<4>(
           (q1 * dcm22) * err,
           (q0 * dcm22 + 2 * q1 * dcm12) * err,
           (q3 * dcm22 + 2 * q2 * dcm12) * err,
           (q2 * dcm22) * err
         );
}
const Vector<4> dtheta_dq(const Vector<4> & quat, const Matrix<3,3> & DCM) {
  const _mat_float q0 = quat[0];
  const _mat_float q1 = quat[1];
  const _mat_float q2 = quat[2];
  const _mat_float q3 = quat[3];
  const _mat_float dcm02 = DCM[0][2];
  const _mat_float err = -2 / sqrt(1 - dcm02*dcm02);
  return Vector<4>(-q2 * err, q3 * err, -q0 * err, q1 * err);
}
const Vector<4> dpsi_dq(const Vector<4> & quat, const Matrix<3,3> & DCM) {
  const _mat_float q0 = quat[0];
  const _mat_float q1 = quat[1];
  const _mat_float q2 = quat[2];
  const _mat_float q3 = quat[3];
  const _mat_float dcm00 = DCM[0][0];
  const _mat_float dcm01 = DCM[0][1];
  const _mat_float err = 2 /(dcm00*dcm00 + dcm01*dcm01);
  return Vector<4>(err *(q3 * dcm00), err *(q2 * dcm00), err *(q1 * dcm00 + 2 * q2 * dcm01), err *(q0 * dcm00 + 2 * q3 * dcm01));
}
//=============================================================================
// MATH
//=============================================================================
/*const Quat qmethod(const _mat_float &a1,const _mat_float &a2,const Vect &r1,const Vect &r2,const Vect &b1,const Vect &b2)
{
  Vect Z=a1*cross(r1,b1)+a2*cross(r2,b2);
  Matrix<3,3> B=mult_T(a1*r1,b1)+mult_T(a2*r2,b2);
  dmsg(B);
  Matrix<3,3> S=B+B.transpose();
  dmsg(S);
  _mat_float sigma=trace(B);
  //Compute the K matrixmath
  Matrix<4,4> K(
    Vector<4>(-sigma,Z[0],Z[1],Z[2]),
    Vector<4>(Z[0],-S[0][0]+sigma,-S[0][1],-S[0][2]),
    Vector<4>(Z[1],-S[1][0],-S[1][1]+sigma,-S[1][2]),
    Vector<4>(Z[2],-S[2][0],-S[2][1],-S[2][2]+sigma)
  );
  dmsg(Z);
  dmsg(K);
  //Find the eigenvector for the smallest eigenvalue of K
}*/
//=============================================================================
/*const Matrix<4,3> Tquat(const Quat &q)
{
  const _mat_float eta=q[0];
  const _mat_float eps1=q[1];
  const _mat_float eps2=q[2];
  const _mat_float eps3=q[3];
  return Matrix<4,3>(
    Vect(-eps1,-eps2,-eps3),
    Vect( eta, -eps3, eps2),
    Vect( eps3, eta, -eps1),
    Vect(-eps2, eps1, eta)
  );
}*/
//=============================================================================
/*const Matrix<3,3> Wmtrx(const Vect &eps,const Vect &v)
{
  const _mat_float eps2=eps*eps;
  const _mat_float eta=eps2>=1?0:sqrt(1-eps2);
  Matrix<3,3> W,tmp,tmp2;

  if(eta!=0){
    tmp.eulerWx(v*(-2.0/eta));
    tmp2.mult(eps,eps);
    W.mult(tmp,tmp2);
  }

  tmp.eulerWx(v*(2.0*eta));
  W+=tmp;

  W.eye_add(2.0*(v*eps));
  tmp.mult(eps,v);
  tmp*=2;
  W+=tmp;
  tmp.mult(v,eps);
  tmp*=-4;
  W+=tmp;
  return W;
}*/
//=============================================================================
const Quat qmult(const Quat &q1,const Quat &q2)
{
  const _mat_float eta1=q1[0];
  const Vect eps1=Vect(q1[1],q1[2],q1[3]);
  const _mat_float eta2=q2[0];
  const Vect eps2=Vect(q2[1],q2[2],q2[3]);
  const _mat_float eta=eta1*eta2 - eps1*eps2;
  const Vect eps=eta1*eps2+eta2*eps1+cross(eps1,eps2);
  return Quat(eta,eps[0],eps[1],eps[2]);
}
//=============================================================================
const Matrix<4,4> Omega(const Vect &w)
{
  const _mat_float x=w[0];
  const _mat_float y=w[1];
  const _mat_float z=w[2];
  Matrix<4,4> A;
  A[0]=Quat(0,-x,-y,-z);
  A[1]=Quat(x,0,z,-y);
  A[2]=Quat(y,-z,0,x);
  A[3]=Quat(z,y,-x,0);
  return A;
}
//=============================================================================
//=============================================================================
}//namespace
//=============================================================================
