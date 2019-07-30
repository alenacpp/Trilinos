/*@HEADER
// ***********************************************************************
//
//       Ifpack2: Templated Object-Oriented Algebraic Preconditioner Package
//                 Copyright (2009) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
//@HEADER
*/

#ifndef IFPACK2_BANDEDCONTAINER_DEF_HPP
#define IFPACK2_BANDEDCONTAINER_DEF_HPP

#include "Teuchos_LAPACK.hpp"
#include "Tpetra_CrsMatrix.hpp"
#include <iostream>
#include <sstream>

#ifdef HAVE_MPI
#  include <mpi.h>
#  include "Teuchos_DefaultMpiComm.hpp"
#else
#  include "Teuchos_DefaultSerialComm.hpp"
#endif // HAVE_MPI

namespace Ifpack2 {

template<class MatrixType, class LocalScalarType>
BandedContainer<MatrixType, LocalScalarType>::
BandedContainer (const Teuchos::RCP<const row_matrix_type>& matrix,
                 const Teuchos::Array<Teuchos::Array<local_ordinal_type> >& partitions,
                 const Teuchos::RCP<const import_type>&,
                 bool pointIndexed) :
  ContainerImpl<MatrixType, LocalScalarType>(matrix, partitions, pointIndexed),
  ipiv_(this->blockRows_.size() * this->scalarsPerRow_),
  kl_(this->numBlocks_, -1),
  ku_(this->numBlocks_, -1),
  scalarOffsets_(this->numBlocks_)
{
  TEUCHOS_TEST_FOR_EXCEPTION(
    ! matrix->hasColMap (), std::invalid_argument, "Ifpack2::BandedContainer: "
    "The constructor's input matrix must have a column Map.");
}

template<class MatrixType, class LocalScalarType>
BandedContainer<MatrixType, LocalScalarType>::
BandedContainer (const Teuchos::RCP<const row_matrix_type>& matrix,
                 Teuchos::ArrayView<const local_ordinal_type> blockRows,
                 bool pointIndexed) :
  ContainerImpl<MatrixType, LocalScalarType>(matrix, blockRows, pointIndexed),
  ipiv_(this->blockSizes_[0] * this->scalarsPerRow_),
  kl_(1, -1),
  ku_(1, -1),
  scalarOffsets_(1, 0)
{
  TEUCHOS_TEST_FOR_EXCEPTION(!matrix->hasColMap(), std::invalid_argument, "Ifpack2::BandedContainer: "
    "The constructor's input matrix must have a column Map.");
}

template<class MatrixType, class LocalScalarType>
void BandedContainer<MatrixType, LocalScalarType>::
setParameters (const Teuchos::ParameterList& List)
{
  using Teuchos::Array;
  using Teuchos::ArrayView;
  if(List.isParameter("relaxation: banded container superdiagonals"))
  {
    int ku = List.get<int>("relaxation: banded container superdiagonals");
    for(local_ordinal_type b = 0; b < this->numBlocks_; b++)
      ku_[b] = ku;
  }
  if(List.isParameter("relaxation: banded container subdiagonals"))
  {
    int kl = List.get<int>("relaxation: banded container subdiagonals");
    for(local_ordinal_type b = 0; b < this->numBlocks_; b++)
      kl_[b] = kl;
  }
}

template<class MatrixType, class LocalScalarType>
void BandedContainer<MatrixType, LocalScalarType>::
computeBandwidth()
{
  using Teuchos::Array;
  using Teuchos::ArrayView;
  //now, for any block where kl_ or ku_ has not already been set, compute the actual bandwidth
  const auto INVALID = Teuchos::OrdinalTraits<local_ordinal_type>::invalid();
  size_t colToOffsetSize = this->inputMatrix_->getNodeNumCols();
  if(this->pointIndexed_)
    colToOffsetSize *= this->bcrsBlockSize_;
  Array<local_ordinal_type> colToBlockOffset(colToOffsetSize, INVALID);
  //Same logic as extract() to find entries in blocks efficiently
  //(but here, just to find bandwidth, no scalars used)
  for(int i = 0; i < this->numBlocks_; i++)
  {
    //maxSub, maxSuper are the maximum lower and upper bandwidth
    local_ordinal_type maxSub = 0;
    local_ordinal_type maxSuper = 0;
    if(this->scalarsPerRow_ > 1)
    {
      //Get the interval where block i is defined in blockRows_
      local_ordinal_type blockStart = this->blockOffsets_[i];
      local_ordinal_type blockEnd = (i == this->numBlocks_ - 1) ? this->blockRows_.size() : this->blockOffsets_[i + 1];
      ArrayView<const local_ordinal_type> blockRows = this->getBlockRows(i);
      //Set the lookup table entries for the columns appearing in block i.
      //If OverlapLevel_ > 0, then this may overwrite values for previous blocks, but
      //this is OK. The values updated here are only needed to process block i's entries.
      for(size_t j = 0; j < (size_t) blockRows.size(); j++)
      {
        local_ordinal_type localCol = this->translateRowToCol(blockRows[j]);
        colToBlockOffset[localCol] = blockStart + j;
      }
      for(local_ordinal_type blockRow = 0; blockRow < (local_ordinal_type) blockRows.size(); blockRow++)
      {
        //get a raw view of the whole block row
        const local_ordinal_type* indices;
        scalar_type* values;
        local_ordinal_type numEntries;
        local_ordinal_type inputRow = this->blockRows_[blockStart + blockRow];
        this->inputBlockMatrix_->getLocalRowView(inputRow, indices, values, numEntries);
        for(local_ordinal_type k = 0; k < numEntries; k++)
        {
          local_ordinal_type colOffset = colToBlockOffset[indices[k]];
          if(blockStart <= colOffset && colOffset < blockEnd)
          {
            //This entry does appear in the diagonal block.
            //(br, bc) identifies the scalar's position in the BlockCrs block.
            //Convert this to (r, c) which is its position in the container block.
            local_ordinal_type blockCol = colOffset - blockStart;
            for(local_ordinal_type bc = 0; bc < this->bcrsBlockSize_; bc++)
            {
              for(local_ordinal_type br = 0; br < this->bcrsBlockSize_; br++)
              {
                local_ordinal_type r = this->bcrsBlockSize_ * blockRow + br;
                local_ordinal_type c = this->bcrsBlockSize_ * blockCol + bc;
                if(r - c > maxSub)
                  maxSub = r - c;
                if(c - r > maxSuper)
                  maxSuper = c - r;
              }
            }
          }
        }
      }
    }
    else
    {
      //Get the interval where block i is defined in blockRows_
      local_ordinal_type blockStart = this->blockOffsets_[i];
      local_ordinal_type blockEnd = (i == this->numBlocks_ - 1) ? this->blockRows_.size() : this->blockOffsets_[i + 1];
      ArrayView<const local_ordinal_type> blockRows = this->getBlockRows(i);
      //Set the lookup table entries for the columns appearing in block i.
      //If OverlapLevel_ > 0, then this may overwrite values for previous blocks, but
      //this is OK. The values updated here are only needed to process block i's entries.
      for(size_t j = 0; j < (size_t) blockRows.size(); j++)
      {
        //translateRowToCol will return the corresponding split column
        local_ordinal_type localCol = this->translateRowToCol(blockRows[j]);
        colToBlockOffset[localCol] = blockStart + j;
      }
      for(local_ordinal_type blockRow = 0; blockRow < (local_ordinal_type) blockRows.size(); blockRow++)
      {
        //get a view of the general row
        local_ordinal_type inputSplitRow = this->blockRows_[blockStart + blockRow];
        auto rowView = this->getInputRowView(inputSplitRow);
        for(size_t k = 0; k < rowView.size(); k++)
        {
          local_ordinal_type colOffset = colToBlockOffset[rowView.ind(k)];
          if(blockStart <= colOffset && colOffset < blockEnd)
          {
            local_ordinal_type blockCol = colOffset - blockStart;
            maxSub = std::max(maxSub, blockRow - blockCol);
            maxSuper = std::max(maxSuper, blockCol - blockRow);
          }
        }
      }
    }
    kl_[i] = maxSub;
    ku_[i] = maxSuper;
  }
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
initialize ()
{
  using Teuchos::null;
  using Teuchos::rcp;
  //note: in BlockRelaxation, Container_->setParameters() immediately
  //precedes Container_->initialize(). So no matter what, either all
  //the block bandwidths were set (in setParameters()) or none of them were.
  //If none were they must be computed individually.
  if(kl_[0] == -1)
    computeBandwidth();
  global_ordinal_type totalScalars = 0;
  for(local_ordinal_type b = 0; b < this->numBlocks_; b++)
  {
    local_ordinal_type stride = 2 * kl_[b] + ku_[b] + 1;
    scalarOffsets_[b] = totalScalars;
    totalScalars += stride * this->blockSizes_[b] * this->scalarsPerRow_;
  }
  scalars_.resize(totalScalars);
  for(int b = 0; b < this->numBlocks_; b++)
  {
    //NOTE: the stride and upper bandwidth used to construct the SerialBandDenseMatrix looks
    //too large, but the extra kl_ in upper band space is needed by the LAPACK factorization routine.
    local_ordinal_type nrows = this->blockSizes_[b] * this->scalarsPerRow_;
    diagBlocks_.emplace_back(Teuchos::View, scalars_.data() + scalarOffsets_[b], 2 * kl_[b] + ku_[b] + 1, nrows, nrows, kl_[b], kl_[b] + ku_[b]);
    diagBlocks_[b].putScalar(Teuchos::ScalarTraits<local_scalar_type>::zero());
  }
  std::fill (ipiv_.begin (), ipiv_.end (), 0);
  // We assume that if you called this method, you intend to recompute
  // everything.
  this->IsComputed_ = false;
  this->IsInitialized_ = true;
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
compute ()
{
  TEUCHOS_TEST_FOR_EXCEPTION(
    ipiv_.size () != this->blockRows_.size() * this->scalarsPerRow_, std::logic_error,
    "Ifpack2::BandedContainer::compute: ipiv_ array has the wrong size.  "
    "Please report this bug to the Ifpack2 developers.");

  this->IsComputed_ = false;
  if (!this->isInitialized ()) {
    this->initialize ();
  }

  // Extract the submatrices from input matrix.
  this->extract (diagBlocks_);
  factor (); // factor the submatrix

  this->IsComputed_ = true;
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
clearBlocks ()
{
  diagBlocks_.clear();
  scalars_.clear();
  ContainerImpl<MatrixType, LocalScalarType>::clearBlocks();
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
factor ()
{
  Teuchos::LAPACK<int, local_scalar_type> lapack;
  int INFO = 0;

  for(int i = 0; i < this->numBlocks_; i++)
  {
    // Plausibility checks for matrix
    TEUCHOS_TEST_FOR_EXCEPTION(diagBlocks_[i].values()==0, std::invalid_argument,
                       "BandedContainer<T>::factor: Diagonal block is an empty SerialBandDenseMatrix<T>!");
    TEUCHOS_TEST_FOR_EXCEPTION(diagBlocks_[i].upperBandwidth() < diagBlocks_[i].lowerBandwidth(), std::invalid_argument,
                       "BandedContainer<T>::factor: Diagonal block needs kl additional superdiagonals for factorization! However, the number of superdiagonals is smaller than the number of subdiagonals!");
    int* blockIpiv = &ipiv_[this->blockOffsets_[i] * this->scalarsPerRow_];
    lapack.GBTRF (diagBlocks_[i].numRows(),
        diagBlocks_[i].numCols(),
        diagBlocks_[i].lowerBandwidth(),
        diagBlocks_[i].upperBandwidth() - diagBlocks_[i].lowerBandwidth(), /* enter the real number of superdiagonals (see Teuchos_SerialBandDenseSolver)*/
        diagBlocks_[i].values(),
        diagBlocks_[i].stride(),
        blockIpiv,
        &INFO);

    // INFO < 0 is a bug.
    TEUCHOS_TEST_FOR_EXCEPTION(
      INFO < 0, std::logic_error, "Ifpack2::BandedContainer::factor: "
      "LAPACK's _GBTRF (LU factorization with partial pivoting) was called "
      "incorrectly.  INFO = " << INFO << " < 0.  "
      "Please report this bug to the Ifpack2 developers.");
    // INFO > 0 means the matrix is singular.  This is probably an issue
    // either with the choice of rows the rows we extracted, or with the
    // input matrix itself.
    TEUCHOS_TEST_FOR_EXCEPTION(
      INFO > 0, std::runtime_error, "Ifpack2::BandedContainer::factor: "
      "LAPACK's _GBTRF (LU factorization with partial pivoting) reports that the "
      "computed U factor is exactly singular.  U(" << INFO << "," << INFO << ") "
      "(one-based index i) is exactly zero.  This probably means that the input "
      "matrix has a singular diagonal block.");
  }
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
solveBlock(HostSubview& X,
           HostSubview& Y,
           int blockIndex,
           Teuchos::ETransp mode,
           const local_scalar_type alpha,
           const local_scalar_type beta) const
{
  using Teuchos::ArrayRCP;
  using Teuchos::Ptr;
  using Teuchos::ptr;
  using Teuchos::RCP;
  using Teuchos::rcp;
  using Teuchos::rcpFromRef;

  #ifdef HAVE_IFPACK2_DEBUG
  TEUCHOS_TEST_FOR_EXCEPTION(
    X.extent (0) != Y.extent (0),
    std::logic_error, "Ifpack2::BandedContainer::solveBlock: X and Y have "
    "incompatible dimensions (" << X.extent (0) << " resp. "
    << Y.extent (0) << ").  Please report this bug to "
    "the Ifpack2 developers.");
  TEUCHOS_TEST_FOR_EXCEPTION(
    X.extent (0) != static_cast<size_t> (mode == Teuchos::NO_TRANS ? diagBlocks_[blockIndex].numCols() : diagBlocks_[blockIndex].numRows()),
    std::logic_error, "Ifpack2::BandedContainer::solveBlock: The input "
    "multivector X has incompatible dimensions from those of the "
    "inverse operator (" << X.extent (0) << " vs. "
    << (mode == Teuchos::NO_TRANS ? diagBlocks_[blockIndex].numCols() : diagBlocks_[blockIndex].numRows())
    << ").  Please report this bug to the Ifpack2 developers.");
  TEUCHOS_TEST_FOR_EXCEPTION(
    Y.extent (0) != static_cast<size_t> (mode == Teuchos::NO_TRANS ? diagBlocks_[blockIndex].numRows() : diagBlocks_[blockIndex].numCols()),
    std::logic_error, "Ifpack2::BandedContainer::solveBlock: The output "
    "multivector Y has incompatible dimensions from those of the "
    "inverse operator (" << Y.extent (0) << " vs. "
    << (mode == Teuchos::NO_TRANS ? diagBlocks_[blockIndex].numRows() : diagBlocks_[blockIndex].numCols())
    << ").  Please report this bug to the Ifpack2 developers.");
  #endif

  size_t numRows = (int) X.extent (0);
  size_t numVecs = (int) X.extent (1);

  auto zero = Teuchos::ScalarTraits<scalar_type>::zero ();
  if (alpha == zero) { // don't need to solve the linear system
    if (beta == zero) {
      // Use BLAS AXPY semantics for beta == 0: overwrite, clobbering
      // any Inf or NaN values in Y (rather than multiplying them by
      // zero, resulting in NaN values).
      for(size_t j = 0; j < numVecs; j++)
        for(size_t i = 0; i < numRows; i++)
          Y(i, j) = zero;
    }
    else { // beta != 0
      for(size_t j = 0; j < numVecs; j++)
        for(size_t i = 0; i < numRows; i++)
          Y(i, j) = beta * Y(i, j);
    }
  }
  else { // alpha != 0; must solve the linear system
    Teuchos::LAPACK<int, local_scalar_type> lapack;
    // If beta is nonzero or Y is not constant stride, we have to use
    // a temporary output multivector.  It gets a copy of X, since
    // GBTRS overwrites its (multi)vector input with its output.
    local_impl_scalar_type* yTemp = new local_impl_scalar_type[numVecs * numRows];
    for(size_t j = 0; j < numVecs; j++)
    {
      for(size_t i = 0; i < numRows; i++)
        yTemp[j * numRows + i] = X(i, j);
    }

    int INFO = 0;
    const char trans = (mode == Teuchos::CONJ_TRANS ? 'C' : (mode == Teuchos::TRANS ? 'T' : 'N'));

    const int* blockIpiv = &ipiv_[this->blockOffsets_[blockIndex] * this->scalarsPerRow_];

    lapack.GBTRS(trans,
        diagBlocks_[blockIndex].numCols(),
        diagBlocks_[blockIndex].lowerBandwidth(),
        /* enter the real number of superdiagonals (see Teuchos_SerialBandDenseSolver)*/
        diagBlocks_[blockIndex].upperBandwidth() - diagBlocks_[blockIndex].lowerBandwidth(),
        numVecs,
        diagBlocks_[blockIndex].values(),
        diagBlocks_[blockIndex].stride(),
        blockIpiv,
        yTemp,
        numRows,
        &INFO);

    if (beta != STS::zero ()) {
      for(size_t j = 0; j < numVecs; j++)
      {
        for(size_t i = 0; i < numRows; i++)
        {
          Y(i, j) *= beta;
          Y(i, j) += alpha * yTemp[j * numRows + i];
        }
      }
    }
    else {
      for(size_t j = 0; j < numVecs; j++)
      {
        for(size_t i = 0; i < numRows; i++)
          Y(i, j) = yTemp[j * numRows + i];
      }
    }

    delete[] yTemp;

    TEUCHOS_TEST_FOR_EXCEPTION(
      INFO != 0, std::runtime_error, "Ifpack2::BandedContainer::solveBlock: "
      "LAPACK's _GBTRS (solve using LU factorization with partial pivoting) "
      "failed with INFO = " << INFO << " != 0.");
  }
}

template<class MatrixType, class LocalScalarType>
std::ostream&
BandedContainer<MatrixType, LocalScalarType>::
print (std::ostream& os) const
{
  Teuchos::FancyOStream fos (Teuchos::rcpFromRef (os));
  fos.setOutputToRootOnly (0);
  describe (fos);
  return os;
}

template<class MatrixType, class LocalScalarType>
std::string
BandedContainer<MatrixType, LocalScalarType>::
description () const
{
  std::ostringstream oss;
  oss << Teuchos::Describable::description();
  if (this->isInitialized()) {
    if (this->isComputed()) {
      oss << "{status = initialized, computed";
    }
    else {
      oss << "{status = initialized, not computed";
    }
  }
  else {
    oss << "{status = not initialized, not computed";
  }
  oss << "}";
  return oss.str();
}

template<class MatrixType, class LocalScalarType>
void
BandedContainer<MatrixType, LocalScalarType>::
describe (Teuchos::FancyOStream& os,
          const Teuchos::EVerbosityLevel verbLevel) const
{
  if(verbLevel==Teuchos::VERB_NONE) return;
  os << "================================================================================" << std::endl;
  os << "Ifpack2::BandedContainer" << std::endl;
  for(int i = 0; i < this->numBlocks_; i++)
  {
    os << "Block " << i << ": Number of rows           = " << this->blockSizes_[i] << std::endl;
    os << "Block " << i << ": Number of subdiagonals   = " << diagBlocks_[i].lowerBandwidth() << std::endl;
    os << "Block " << i << ": Number of superdiagonals = " << diagBlocks_[i].upperBandwidth() << std::endl;
  }
  os << "isInitialized()          = " << this->IsInitialized_ << std::endl;
  os << "isComputed()             = " << this->IsComputed_ << std::endl;
  os << "================================================================================" << std::endl;
  os << std::endl;
}

template<class MatrixType, class LocalScalarType>
std::string BandedContainer<MatrixType, LocalScalarType>::getName()
{
  return "Banded";
}

} // namespace Ifpack2

#define IFPACK2_BANDEDCONTAINER_INSTANT(S,LO,GO,N) \
  template class Ifpack2::BandedContainer< Tpetra::RowMatrix<S, LO, GO, N>, S >;

#endif // IFPACK2_BANDEDCONTAINER_HPP
