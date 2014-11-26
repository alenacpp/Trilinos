// @HEADER
// ***********************************************************************
//
//                           Sacado Package
//                 Copyright (2006) Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
// USA
// Questions? Contact David M. Gay (dmgay@sandia.gov) or Eric T. Phipps
// (etphipp@sandia.gov).
//
// ***********************************************************************
//
// The forward-mode AD classes in Sacado are a derivative work of the
// expression template classes in the Fad package by Nicolas Di Cesare.
// The following banner is included in the original Fad source code:
//
// ************ DO NOT REMOVE THIS BANNER ****************
//
//  Nicolas Di Cesare <Nicolas.Dicesare@ann.jussieu.fr>
//  http://www.ann.jussieu.fr/~dicesare
//
//            CEMRACS 98 : C++ courses,
//         templates : new C++ techniques
//            for scientific computing
//
//********************************************************
//
//  A short implementation ( not all operators and
//  functions are overloaded ) of 1st order Automatic
//  Differentiation in forward mode (FAD) using
//  EXPRESSION TEMPLATES.
//
//********************************************************
// @HEADER

#include "Sacado_ConfigDefs.h"

namespace Sacado {
namespace LFad {

#define FAD LogicalSparseImp<ValT,LogT>

template <typename ValT, typename LogT>
template <typename S>
inline LogicalSparseImp<ValT,LogT>::
LogicalSparseImp(const Expr<S>& x, SACADO_ENABLE_EXPR_CTOR_DEF) :
  Storage(value_type(0))
{
  int sz = x.size();

  if (sz != this->size())
    this->resize(sz);

  if (sz) {
    if (x.hasFastAccess())
      for(int i=0; i<sz; ++i)
        this->fastAccessDx(i) = x.fastAccessDx(i);
    else
      for(int i=0; i<sz; ++i)
        this->fastAccessDx(i) = x.dx(i);
  }

  this->val() = x.val();
}


template <typename ValT, typename LogT>
inline void
LogicalSparseImp<ValT,LogT>::
diff(const int ith, const int n)
{
  if (this->size() != n)
    this->resize(n);

  this->zero();
  this->fastAccessDx(ith) = logical_type(1);

}

template <typename ValT, typename LogT>
inline LogicalSparseImp<ValT,LogT>&
LogicalSparseImp<ValT,LogT>::
operator=(const LogicalSparseImp<ValT,LogT>& x)
{
  // Copy value & dx_
  Storage::operator=(x);

  return *this;
}

template <typename ValT, typename LogT>
template <typename S>
inline SACADO_FAD_ENABLE_EXPR_FUNC
LogicalSparseImp<ValT,LogT>::
operator=(const Expr<S>& x)
{
  int sz = x.size();

  if (sz != this->size())
    this->resize(sz);

  if (sz) {
    if (x.hasFastAccess())
      for(int i=0; i<sz; ++i)
        this->fastAccessDx(i) = x.fastAccessDx(i);
    else
      for(int i=0; i<sz; ++i)
        this->fastAccessDx(i) = x.dx(i);
  }

  this->val() = x.val();

  return *this;
}

template <typename ValT, typename LogT>
inline LogicalSparseImp<ValT,LogT>&
LogicalSparseImp<ValT,LogT>::
operator += (const LogicalSparseImp<ValT,LogT>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
    }
    else {
      this->resize(xsz);
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.fastAccessDx(i);
    }
  }

  this->val() += x.val();

  return *this;
}

template <typename ValT, typename LogT>
inline LogicalSparseImp<ValT,LogT>&
LogicalSparseImp<ValT,LogT>::
operator -= (const LogicalSparseImp<ValT,LogT>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
    }
    else {
      this->resize(xsz);
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = x.fastAccessDx(i);
    }
  }

  this->val() -= x.val();


  return *this;
}

template <typename ValT, typename LogT>
inline LogicalSparseImp<ValT,LogT>&
LogicalSparseImp<ValT,LogT>::
operator *= (const LogicalSparseImp<ValT,LogT>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
    }
    else {
      this->resize(xsz);
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = x.fastAccessDx(i);
    }
  }

  this->val() *= x.val();

  return *this;
}

template <typename ValT, typename LogT>
inline LogicalSparseImp<ValT,LogT>&
LogicalSparseImp<ValT,LogT>::
operator /= (const LogicalSparseImp<ValT,LogT>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
    }
    else {
      this->resize(xsz);
      for (int i=0; i<xsz; ++i)
        this->fastAccessDx(i) = x.fastAccessDx(i);
    }
  }

  this->val() /= x.val();

  return *this;
}

template <typename ValT, typename LogT>
template <typename S>
inline SACADO_FAD_ENABLE_EXPR_FUNC
LogicalSparseImp<ValT,LogT>::
operator += (const Expr<S>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.dx(i);
    }
    else {
      this->resize(xsz);
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.dx(i);
    }
  }

  this->val() += x.val();

  return *this;
}

template <typename ValT, typename LogT>
template <typename S>
inline SACADO_FAD_ENABLE_EXPR_FUNC
LogicalSparseImp<ValT,LogT>::
operator -= (const Expr<S>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.dx(i);
    }
    else {
      this->resize(xsz);
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.dx(i);
    }
  }

  this->val() -= x.val();


  return *this;
}

template <typename ValT, typename LogT>
template <typename S>
inline SACADO_FAD_ENABLE_EXPR_FUNC
LogicalSparseImp<ValT,LogT>::
operator *= (const Expr<S>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.dx(i);
    }
    else {
      this->resize(xsz);
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.dx(i);
    }
  }

  this->val() *= x.val();

  return *this;
}

template <typename ValT, typename LogT>
template <typename S>
inline SACADO_FAD_ENABLE_EXPR_FUNC
LogicalSparseImp<ValT,LogT>::
operator /= (const Expr<S>& x)
{
  int xsz = x.size(), sz = this->size();

#ifdef SACADO_DEBUG
  if ((xsz != sz) && (xsz != 0) && (sz != 0))
    throw "LFad Error:  Attempt to assign with incompatible sizes";
#endif

  if (xsz) {
    if (sz) {
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = this->fastAccessDx(i) || x.dx(i);
    }
    else {
      this->resize(xsz);
      if (x.hasFastAccess())
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.fastAccessDx(i);
      else
        for (int i=0; i<xsz; ++i)
          this->fastAccessDx(i) = x.dx(i);
    }
  }

  this->val() /= x.val();

  return *this;
}

#undef FAD

} // namespace LFad
} // namespace Sacado
