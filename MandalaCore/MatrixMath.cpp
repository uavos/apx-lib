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
//#include <iostream>
#include "MatrixMath.h"
//#include <dmsg.h>
#include <float.h>
//=============================================================================
namespace matrixmath {
//=============================================================================
const Matrix<3, 3> eulerDC(const Vector<3> &euler)
{
    const _mat_float &phi = euler[0];
    const _mat_float &theta = euler[1];
    const _mat_float &psi = euler[2];
    const _mat_float cpsi = std::cos(psi);
    const _mat_float cphi = std::cos(phi);
    const _mat_float ctheta = std::cos(theta);
    const _mat_float spsi = std::sin(psi);
    const _mat_float sphi = std::sin(phi);
    const _mat_float stheta = std::sin(theta);
    return Matrix<3, 3>(Vector<3>(cpsi * ctheta, spsi * ctheta, -stheta), Vector<3>(-spsi * cphi + cpsi * stheta * sphi, cpsi * cphi + spsi * stheta * sphi, ctheta * sphi), Vector<3>(spsi * sphi + cpsi * stheta * cphi, -cpsi * sphi + spsi * stheta * cphi, ctheta * cphi));
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
const Matrix<4, 4> quatW(const Vector<3> euler)
{
    const _mat_float p = euler[0] / 2.0;
    const _mat_float q = euler[1] / 2.0;
    const _mat_float r = euler[2] / 2.0;
    return Matrix<4, 4>(Vector<4>(0, -p, -q, -r), Vector<4>(p, 0, r, -q), Vector<4>(q, -r, 0, p), Vector<4>(r, q, -p, 0));
}
const Vector<4> dphi_dq(const Vector<4> &quat, const Matrix<3, 3> &DCM)
{
    const _mat_float q0 = quat[0];
    const _mat_float q1 = quat[1];
    const _mat_float q2 = quat[2];
    const _mat_float q3 = quat[3];
    const _mat_float dcm22 = DCM[2][2];
    const _mat_float dcm12 = DCM[1][2];
    const _mat_float err = 2 / (dcm22 * dcm22 + dcm12 * dcm12);
    return Vector<4>((q1 * dcm22) * err, (q0 * dcm22 + 2 * q1 * dcm12) * err, (q3 * dcm22 + 2 * q2 * dcm12) * err, (q2 * dcm22) * err);
}
const Vector<4> dtheta_dq(const Vector<4> &quat, const Matrix<3, 3> &DCM)
{
    const _mat_float q0 = quat[0];
    const _mat_float q1 = quat[1];
    const _mat_float q2 = quat[2];
    const _mat_float q3 = quat[3];
    const _mat_float dcm02 = DCM[0][2];
    const _mat_float err = -2 / std::sqrt(1 - dcm02 * dcm02);
    return Vector<4>(-q2 * err, q3 * err, -q0 * err, q1 * err);
}
const Vector<4> dpsi_dq(const Vector<4> &quat, const Matrix<3, 3> &DCM)
{
    const _mat_float q0 = quat[0];
    const _mat_float q1 = quat[1];
    const _mat_float q2 = quat[2];
    const _mat_float q3 = quat[3];
    const _mat_float dcm00 = DCM[0][0];
    const _mat_float dcm01 = DCM[0][1];
    const _mat_float err = 2 / (dcm00 * dcm00 + dcm01 * dcm01);
    return Vector<4>(err * (q3 * dcm00), err * (q2 * dcm00), err * (q1 * dcm00 + 2 * q2 * dcm01), err * (q0 * dcm00 + 2 * q3 * dcm01));
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
  const _mat_float eta=eps2>=1?0:std::sqrt(1-eps2);
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
const Matrix<4, 4> Omega(const Vector<3> &w)
{
    const _mat_float x = w[0];
    const _mat_float y = w[1];
    const _mat_float z = w[2];
    Matrix<4, 4> A;
    A[0] = Quat(0, -x, -y, -z);
    A[1] = Quat(x, 0, z, -y);
    A[2] = Quat(y, -z, 0, x);
    A[3] = Quat(z, y, -x, 0);
    return A;
}
//=============================================================================
//=============================================================================
const Vector<3> Quat::toEuler() const
{
    const _mat_float &q0 = (*this)[0];
    const _mat_float &q1 = (*this)[1];
    const _mat_float &q2 = (*this)[2];
    const _mat_float &q3 = (*this)[3];
    _mat_float phi = std::atan2(2.0 * (q2 * q3 + q0 * q1), 1.0 - 2.0 * (q1 * q1 + q2 * q2));
    _mat_float theta = -std::asin(2 * (q1 * q3 - q0 * q2));
    _mat_float psi = std::atan2(2.0 * (q1 * q2 + q0 * q3), 1.0 - 2.0 * (q2 * q2 + q3 * q3));
    return Vector<3>(phi, theta, psi);
    /*const _mat_float & qw = (*this)[0];
  const _mat_float & qx = (*this)[1];
  const _mat_float & qy = (*this)[2];
  const _mat_float & qz = (*this)[3];
  Vector<3> euler;
  euler[0]=std::atan2(2.0*(qx*qw+qy*qz),1.0-2.0*(qx*qx-qz*qz));
  euler[1]=std::asin(2.0*(qx*qy+qz*qw));
  euler[2]=std::atan2(2.0*(qy*qw-qx*qz),1.0-2.0*(qy*qy-qz*qz));
  return euler;*/
}
//=============================================================================
Quat &Quat::qmult(const Quat &q)
{
    const _mat_float eta1 = (*this)[0];
    const Vector<3> eps1((*this)[1], (*this)[2], (*this)[3]);
    const _mat_float eta2 = q[0];
    const Vector<3> eps2(q[1], q[2], q[3]);
    const _mat_float eta = eta1 * eta2 - eps1 * eps2;
    const Vector<3> eps(eta1 * eps2 + eta2 * eps1 + cross(eps1, eps2));
    (*this)[0] = eta;
    (*this)[1] = eps[0];
    (*this)[2] = eps[1];
    (*this)[3] = eps[2];
    /*const _mat_float &w1=(*this)[0];
  const _mat_float &x1=(*this)[1];
  const _mat_float &y1=(*this)[2];
  const _mat_float &z1=(*this)[3];
  const _mat_float &w2=q[0];
  const _mat_float &x2=q[1];
  const _mat_float &y2=q[2];
  const _mat_float &z2=q[3];
  (*this)[0]=w1*w2-x1*x2-y1*y2-z1*z2;
  (*this)[1]=w1*x2+x1*w2+y1*z2-z1*y2;
  (*this)[2]=w1*y2-x1*z2+y1*w2+z1*x2;
  (*this)[3]=w1*z2+x1*y2-y1*x2+z1*w2;*/
    return (*this);
}
//=============================================================================
Quat &Quat::fromEuler(const Vector<3> &euler)
{
    const _mat_float phi = euler[0] / 2.0;
    const _mat_float theta = euler[1] / 2.0;
    const _mat_float psi = euler[2] / 2.0;
    const _mat_float shphi0 = std::sin(phi);
    const _mat_float chphi0 = std::cos(phi);
    const _mat_float shtheta0 = std::sin(theta);
    const _mat_float chtheta0 = std::cos(theta);
    const _mat_float shpsi0 = std::sin(psi);
    const _mat_float chpsi0 = std::cos(psi);
    (*this)[0] = chphi0 * chtheta0 * chpsi0 + shphi0 * shtheta0 * shpsi0;
    (*this)[1] = -chphi0 * shtheta0 * shpsi0 + shphi0 * chtheta0 * chpsi0;
    (*this)[2] = chphi0 * shtheta0 * chpsi0 + shphi0 * chtheta0 * shpsi0;
    (*this)[3] = chphi0 * chtheta0 * shpsi0 - shphi0 * shtheta0 * chpsi0;
    return (*this);
}
//=============================================================================
Quat &Quat::conjugate(void)
{
    //(*this)[0]=-(*this)[0];
    (*this)[1] = -(*this)[1];
    (*this)[2] = -(*this)[2];
    (*this)[3] = -(*this)[3];
    return (*this);
}
//=============================================================================
#define EulFrm(ord) ((unsigned) (ord) &1)
#define EulRep(ord) (((unsigned) (ord) >> 1) & 1)
#define EulPar(ord) (((unsigned) (ord) >> 2) & 1)
#define EulSafe "\000\001\002\000"
#define EulNext "\001\002\000\001"
#define EulAxI(ord) ((int) (EulSafe[(((unsigned) (ord) >> 3) & 3)]))
#define EulAxJ(ord) ((int) (EulNext[EulAxI(ord) + (EulPar(ord) == EulParOdd)]))
#define EulAxK(ord) ((int) (EulNext[EulAxI(ord) + (EulPar(ord) != EulParOdd)]))
#define EulAxH(ord) ((EulRep(ord) == EulRepNo) ? EulAxK(ord) : EulAxI(ord))
#define EulGetOrd(ord, i, j, k, h, n, s, f) \
    { \
        unsigned o = ord; \
        f = o & 1; \
        o >>= 1; \
        s = o & 1; \
        o >>= 1; \
        n = o & 1; \
        o >>= 1; \
        i = EulSafe[o & 3]; \
        j = EulNext[i + n]; \
        k = EulNext[i + 1 - n]; \
        h = s ? k : i; \
    }
_mat_float Quat::HMatrix[4][4];
const Vector<3> Quat::toEuler(int order) const
{
    {
        _mat_float Nq = (*this)[1] * (*this)[1] + (*this)[2] * (*this)[2] + (*this)[3] * (*this)[3] + (*this)[0] * (*this)[0];
        _mat_float s = (Nq > 0.0) ? (2.0 / Nq) : 0.0;
        _mat_float xs = (*this)[1] * s, ys = (*this)[2] * s, zs = (*this)[3] * s;
        _mat_float wx = (*this)[0] * xs, wy = (*this)[0] * ys, wz = (*this)[0] * zs;
        _mat_float xx = (*this)[1] * xs, xy = (*this)[1] * ys, xz = (*this)[1] * zs;
        _mat_float yy = (*this)[2] * ys, yz = (*this)[2] * zs, zz = (*this)[3] * zs;
        HMatrix[0][0] = 1.0 - (yy + zz);
        HMatrix[0][1] = xy - wz;
        HMatrix[0][2] = xz + wy;
        HMatrix[1][0] = xy + wz;
        HMatrix[1][1] = 1.0 - (xx + zz);
        HMatrix[1][2] = yz - wx;
        HMatrix[2][0] = xz - wy;
        HMatrix[2][1] = yz + wx;
        HMatrix[2][2] = 1.0 - (xx + yy);
        HMatrix[3][0] = HMatrix[3][1] = HMatrix[3][2] = HMatrix[0][3] = HMatrix[1][3] = HMatrix[2][3] = 0.0;
        HMatrix[3][3] = 1.0;
    }
    return HMatrix2euler(order);
}
//=============================================================================
void Quat::fromEuler(Vector<3> euler, int order)
{
    _mat_float *a = &((*this)[1]);
    _mat_float ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i, j, k, h, n, s, f;
    EulGetOrd(order, i, j, k, h, n, s, f);
    (void) h;
    if (f == EulFrmR) {
        float t = euler[0];
        euler[0] = euler[2];
        euler[2] = t;
    }
    if (n == EulParOdd)
        euler[1] = -euler[1];
    ti = euler[0] * 0.5;
    tj = euler[1] * 0.5;
    th = euler[2] * 0.5;
    ci = std::cos(ti);
    cj = std::cos(tj);
    ch = std::cos(th);
    si = std::sin(ti);
    sj = std::sin(tj);
    sh = std::sin(th);
    cc = ci * ch;
    cs = ci * sh;
    sc = si * ch;
    ss = si * sh;
    if (s == EulRepYes) {
        a[i] = cj * (cs + sc); /* Could speed up with */
        a[j] = sj * (cc + ss); /* trig identities. */
        a[k] = sj * (cs - sc);
        (*this)[0] = cj * (cc - ss);
    } else {
        a[i] = cj * sc - sj * cs;
        a[j] = cj * ss + sj * cc;
        a[k] = cj * cs - sj * sc;
        (*this)[0] = cj * cc + sj * ss;
    }
    if (n == EulParOdd)
        a[j] = -a[j];
}
//=============================================================================
void Quat::euler2HMatrix(Vector<3> euler, int order)
{
    _mat_float ti, tj, th, ci, cj, ch, si, sj, sh, cc, cs, sc, ss;
    int i, j, k, h, n, s, f;
    EulGetOrd(order, i, j, k, h, n, s, f);
    (void) h;
    if (f == EulFrmR) {
        float t = euler[0];
        euler[0] = euler[2];
        euler[2] = t;
    }
    if (n == EulParOdd) {
        euler[0] = -euler[0];
        euler[1] = -euler[1];
        euler[2] = -euler[2];
    }
    ti = euler[0];
    tj = euler[1];
    th = euler[2];
    ci = std::cos(ti);
    cj = std::cos(tj);
    ch = std::cos(th);
    si = std::sin(ti);
    sj = std::sin(tj);
    sh = std::sin(th);
    cc = ci * ch;
    cs = ci * sh;
    sc = si * ch;
    ss = si * sh;
    if (s == EulRepYes) {
        HMatrix[i][i] = cj;
        HMatrix[i][j] = sj * si;
        HMatrix[i][k] = sj * ci;
        HMatrix[j][i] = sj * sh;
        HMatrix[j][j] = -cj * ss + cc;
        HMatrix[j][k] = -cj * cs - sc;
        HMatrix[k][i] = -sj * ch;
        HMatrix[k][j] = cj * sc + cs;
        HMatrix[k][k] = cj * cc - ss;
    } else {
        HMatrix[i][i] = cj * ch;
        HMatrix[i][j] = sj * sc - cs;
        HMatrix[i][k] = sj * cc + ss;
        HMatrix[j][i] = cj * sh;
        HMatrix[j][j] = sj * ss + cc;
        HMatrix[j][k] = sj * cs - sc;
        HMatrix[k][i] = -sj;
        HMatrix[k][j] = cj * si;
        HMatrix[k][k] = cj * ci;
    }
    HMatrix[3][0] = HMatrix[3][1] = HMatrix[3][2] = HMatrix[0][3] = HMatrix[1][3] = HMatrix[2][3] = 0.0;
    HMatrix[3][3] = 1.0;
}
//=============================================================================
const Vector<3> Quat::HMatrix2euler(int order) const
{
    Vector<3> euler;
    int i, j, k, h, n, s, f;
    EulGetOrd(order, i, j, k, h, n, s, f);
    (void) h;
    if (s == EulRepYes) {
        _mat_float sy = std::sqrt(HMatrix[i][j] * HMatrix[i][j] + HMatrix[i][k] * HMatrix[i][k]);
        if (sy > 16 * FLT_EPSILON) {
            euler[0] = std::atan2(HMatrix[i][j], HMatrix[i][k]);
            euler[1] = std::atan2(sy, HMatrix[i][i]);
            euler[2] = std::atan2(HMatrix[j][i], -HMatrix[k][i]);
        } else {
            euler[0] = std::atan2(-HMatrix[j][k], HMatrix[j][j]);
            euler[1] = std::atan2(sy, HMatrix[i][i]);
            euler[2] = 0;
        }
    } else {
        _mat_float cy = std::sqrt(HMatrix[i][i] * HMatrix[i][i] + HMatrix[j][i] * HMatrix[j][i]);
        if (cy > 16 * FLT_EPSILON) {
            euler[0] = std::atan2(HMatrix[k][j], HMatrix[k][k]);
            euler[1] = std::atan2(-HMatrix[k][i], cy);
            euler[2] = std::atan2(HMatrix[j][i], HMatrix[i][i]);
        } else {
            euler[0] = std::atan2(-HMatrix[j][k], HMatrix[j][j]);
            euler[1] = std::atan2(-HMatrix[k][i], cy);
            euler[2] = 0;
        }
    }
    if (n == EulParOdd) {
        euler[0] = -euler[0];
        euler[1] = -euler[1];
        euler[2] = -euler[2];
    }
    if (f == EulFrmR) {
        float t = euler[0];
        euler[0] = euler[2];
        euler[2] = t;
    }
    return euler;
}
//=============================================================================
//=============================================================================
//=============================================================================
} // namespace matrixmath
//=============================================================================
