#ifndef IFPACK_ADDITIVESCHWARZ_H
#define IFPACK_ADDITIVESCHWARZ_H

#include "Ifpack_Preconditioner.h"
#include "Teuchos_ParameterList.hpp"
#include "Ifpack_ConfigDefs.h"
#include "Ifpack_Preconditioner.h"
#include "Ifpack_Reordering.h"
#include "Ifpack_RCMReordering.h"
#include "Ifpack_LocalFilter.h"
#include "Ifpack_ReorderFilter.h"
#include "Ifpack_Utils.h"
#include "Epetra_CombineMode.h"
#include "Epetra_MultiVector.h"
#include "Epetra_Map.h"
#include "Epetra_Comm.h"
#include "Epetra_LinearProblem.h"
#include "Epetra_Import.h"
#include "Epetra_Export.h"
#include "Epetra_RowMatrix.h"
#include "Epetra_CrsMatrix.h"
#include "Teuchos_ParameterList.hpp"

//! Ifpack_AdditiveSchwarz: a class to define Additive Schwarz preconditioners.

/*!
  Class  Ifpack_AdditiveSchwarz enables the construction of Additive
  Schwarz (one-level overlapping domain decomposition) preconditioners, 
  for a given Epetra_RowMatrix.
  Ifpack_AdditiveSchwarz is derived from Ifpack_Preconditioner,
  itself derived from Epetra_Operator. An application of
  the Additive Schwarz preconditioner can be obtained 
  by calling method ApplyInverse().

  One-level overlapping domain decomposition preconditioners use 
  local solvers, of Dirichlet type. This means that the inverse of
  the local matrix (with minimal or wider overlap) is applied to
  the residual to be preconditioned.

  The preconditioner can be written as:
  \f[
  P_{AS}^{-1} = \sum_{i=1}^M R_i^T A_i^{-1} R_i ,
  \f]
  where \f$M\f$ is the number of subdomains (that is, the number of 
  processors in
  the computation), \f$R_i\f$ is an operator that restricts the global
  vector to the vector lying on subdomain \f$i\f$, and
  \f[
  A_i = R_i A R_i^T.
  \f]

  The construction of Schwarz preconditioners is mainly composed by
  two steps:
  - definition of the restriction and prolongation operator
  \f$R_i\f$ and \f$R_i^T\f$. If minimal overlap is chosen, their
  implementation is trivial, \f$R_i\f$ will return all the local
  components. For wider overlaps, instead, Epetra_Import and
  Epetra_Export will be used to import/export data. The user
  must provide both the matrix to be preconditioned (which is suppose
  to have minimal-overlap) and the matrix with wider overlap.
  - definition of a technique to apply the inverse of \f$A_i\f$.
  To solve on each subdomain, the user can adopt any class, derived
  from Ifpack_Preconditioner. This can be easily accomplished, as
  Ifpack_AdditiveSchwarz is templated with the solver for each subdomain.

*/

template<typename T>
class Ifpack_AdditiveSchwarz : public Ifpack_Preconditioner {
      
public:

  //@{ \name Constructors/Destructors
  //! Ifpack_AdditiveSchwarz constructor with given Epetra_RowMatrix.
  /*! Creates an Ifpack_AdditiveSchwarz preconditioner with overlap.
   * To use minimal-overlap, OverlappingMatrix is omitted
   * (as defaulted to 0).
   *
   * \param
   * Matrix - (In) Pointer to matrix to be preconditioned
   *
   * \param
   * OverlappingMatrix - (In) Pointer to the matrix extended with the
   *                     desired level of overlap.
   */
  Ifpack_AdditiveSchwarz(Epetra_RowMatrix* Matrix,
			 int OverlapLevel = 0);
  
  //@{ \name Destructor.
  //! Destructor
  virtual ~Ifpack_AdditiveSchwarz();
  //@}

  //@{ \name Atribute set methods.

    //! If set true, transpose of this operator will be applied (not implemented).
    /*! This flag allows the transpose of the given operator to be used 
     * implicitly.  
      
    \param 
	   UseTranspose - (In) If true, multiply by the transpose of operator, 
	   otherwise just use operator.

    \return Integer error code, set to 0 if successful.  Set to -1 if this implementation does not support transpose.
  */
    virtual int SetUseTranspose(bool UseTranspose);
  //@}
  
  //@{ \name Mathematical functions.

  //! Applies the matrix to an Epetra_MultiVector.
  /*! 
    \param
    X - (In) A Epetra_MultiVector of dimension NumVectors 
       to multiply with matrix.
    \param
    Y -(Out) A Epetra_MultiVector of dimension NumVectors 
       containing the result.

    \return Integer error code, set to 0 if successful.
    */
    virtual int Apply(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const;

    //! Applies the preconditioner to X, returns the result in Y.
  /*! 
    \param
    X - (In) A Epetra_MultiVector of dimension NumVectors to be preconditioned.
    \param
    Y -(Out) A Epetra_MultiVector of dimension NumVectors containing result.

    \return Integer error code, set to 0 if successful.

    \warning In order to work with AztecOO, any implementation of this method 
    must support the case where X and Y are the same object.
    */
    virtual int ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const;

    //! Returns the infinity norm of the global matrix (not implemented)
    virtual double NormInf() const;
  //@}
  
  //@{ \name Atribute access functions

    //! Returns a character string describing the operator
    virtual char * Label() const;

    //! Returns the current UseTranspose setting.
    virtual bool UseTranspose() const;

    //! Returns true if the \e this object can provide an approximate Inf-norm, false otherwise.
    virtual bool HasNormInf() const;

    //! Returns a pointer to the Epetra_Comm communicator associated with this operator.
    virtual const Epetra_Comm & Comm() const;

    //! Returns the Epetra_Map object associated with the domain of this operator.
    virtual const Epetra_Map & OperatorDomainMap() const;

    //! Returns the Epetra_Map object associated with the range of this operator.
    virtual const Epetra_Map & OperatorRangeMap() const;
  //@}

  //! Returns \c true if the preconditioner has been successfully initialized.
  virtual bool IsInitialized() const
  {
    return(IsInitialized_);
  }

  //! Returns \c true if the preconditioner has been successfully computed.
  virtual bool IsComputed() const
  {
    return(IsComputed_);
  }

  //! Sets the parameters.
  /*! Sets the parameter for the additive Schwarz preconditioner,
   * as well as for all the preconditioners that may need to
   * be defined on each subblock.
   * Parameters accepted by List are:
   * - \c "schwarz: combine mode" : It must be an Epetra_CombineMode.
   *     Default: \c Insert.
   *     It Can be assume of the following values:
   *   - Add: Components on the receiving processor will be added together;
   *   - Zero: Off-processor components will be ignored;
   *   - Insert: Off-processor components will be inserted into locations on 
   *     receiving processor replacing existing values.
   *   - Average: Off-processor components will be averaged with existing;
   *   - AbsMax: Magnitudes of Off-processor components will be 
   *     maxed with magnitudes of existing components on the receiving 
   *     processor.
   * - \c "schwarz: compute condest" : if \c true, \c Compute() will
   *    estimate the condition number of the preconditioner. 
   *    Default: \c false.
   */
  virtual int SetParameters(Teuchos::ParameterList& List);

  //! Sets an integer parameters. Not implemented.
  virtual int SetParameter(const string Name, const int Value)
  {
    return(-1);
  }

  //! Sets a double parameters. Not implemented.
  virtual int SetParameter(const string Name, const double Value)
  {
    return(-1);
  }

  //! Initialized the preconditioner.
  virtual int Initialize();

  //! Computes the preconditioner.
  virtual int Compute();

  //! Returns the estimated condition number (if computed).
  virtual double Condest(const Ifpack_CondestType CT = Ifpack_Cheap,
			 Epetra_RowMatrix* Matrix = 0);

  //! Returns a refernence to the internally stored matrix.
  virtual const Epetra_RowMatrix& Matrix() const
  {
    return(*Matrix_);
  }

  //! Returns a refernence to the internally stored matrix.
  virtual Epetra_RowMatrix& Matrix()
  {
    return(*Matrix_);
  }

  //! Returns \c true is an overlapping matrix is present.
  virtual bool IsOverlapping() const
  {
    return(IsOverlapping_);
  }

  //! Prints major information about this preconditioner.
  virtual std::ostream& Print(std::ostream&) const;
  
protected:

  //! Sets up LocalizedMatrix_, Importer and Exporter_.
  int Setup();
  
  //! Destroys all allocated data
  void Destroy();

  //! Pointers to the matrix to be preconditioned.
  Epetra_RowMatrix* Matrix_;
  //! Pointers to the overlapping matrix.
  Epetra_CrsMatrix* OverlappingMatrix_;
  //! Localized version of Matrix_ or OverlappingMatrix_.
  Ifpack_LocalFilter* LocalizedMatrix_;
  //! Contains the label of \c this object.
  string Label_;
  //! If true, the preconditioner has been successfully initialized.
  bool IsInitialized_;
  //! If true, the preconditioner has been successfully computed.
  bool IsComputed_;
  //! Pointer to the local solver.
  T* Inverse_;
  //! If true, overlapping is used
  bool IsOverlapping_;
  int OverlapLevel_;
  Epetra_Import* Importer_;
  Epetra_Export* Exporter_;
  //! Stores a copy of the list given in SetParameters()
  Teuchos::ParameterList List_;
  //! Combine mode for off-process elements (only if overlap is used)
  Epetra_CombineMode CombineMode_;
  //! Contains the estimated condition number.
  double Condest_;
  //! Maximum number of iterations for condest
  int CondestMaxIters_;
  //! Tolerance for condest
  double CondestTol_;
  
  bool UseReordering_;
  Ifpack_Reordering* Reordering_;
  Ifpack_ReorderFilter* ReorderedLocalizedMatrix_;

};

//==============================================================================
template<typename T>
Ifpack_AdditiveSchwarz<T>::
Ifpack_AdditiveSchwarz(Epetra_RowMatrix* Matrix,
		       int OverlapLevel) :
  Matrix_(Matrix),
  OverlappingMatrix_(0),
  LocalizedMatrix_(0),
  IsInitialized_(false),
  IsComputed_(false),
  Inverse_(0),
  OverlapLevel_(OverlapLevel),
  IsOverlapping_(false),
  Importer_(0),
  Exporter_(0),
  CombineMode_(Add),
  Condest_(-1.0),
  CondestMaxIters_(1550),
  CondestTol_(1e-12),
  UseReordering_(false),
  ReorderedLocalizedMatrix_(0),
  Reordering_(0)
{
  if (Matrix_->Comm().NumProc() == 1)
    OverlapLevel_ = 0;

  if ((OverlapLevel_ != 0) && (Matrix_->Comm().NumProc() > 1))
    IsOverlapping_ = true;
  // Sets parameters to default values
  Teuchos::ParameterList List;
  SetParameters(List);
}

//==============================================================================
template<typename T>
Ifpack_AdditiveSchwarz<T>::~Ifpack_AdditiveSchwarz()
{
  Destroy();
}

//==============================================================================
template<typename T>
void Ifpack_AdditiveSchwarz<T>::Destroy() 
{
  if (OverlappingMatrix_)
    delete OverlappingMatrix_;
  OverlappingMatrix_ = 0;

  if (Inverse_)
    delete Inverse_;
  Inverse_ = 0;

  if (LocalizedMatrix_)
    delete LocalizedMatrix_;
  LocalizedMatrix_ = 0;

  if (Importer_)
    delete Importer_;
  Importer_ = 0;

  if (Exporter_)
    delete Exporter_;
  Exporter_ = 0;

  if (ReorderedLocalizedMatrix_)
    delete ReorderedLocalizedMatrix_;
  ReorderedLocalizedMatrix_ = 0;

  if (Reordering_)
    delete Reordering_;
  Reordering_ = 0;
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::Setup()
{

  if (OverlappingMatrix_)
    LocalizedMatrix_ = new Ifpack_LocalFilter(OverlappingMatrix_);
  else
    LocalizedMatrix_ = new Ifpack_LocalFilter(Matrix_);

  if (LocalizedMatrix_ == 0)
    IFPACK_CHK_ERR(-1);

  // may need to reorder local matrix
  if (UseReordering_) {
    // create reordeing and compute it
    Reordering_ = new Ifpack_RCMReordering();
    IFPACK_CHK_ERR(Reordering_->SetParameters(List_));
    IFPACK_CHK_ERR(Reordering_->Compute(*LocalizedMatrix_));
    // now create reordered localized matrix
    ReorderedLocalizedMatrix_ = 
      new Ifpack_ReorderFilter(LocalizedMatrix_,Reordering_);
    assert(ReorderedLocalizedMatrix_ != 0);
    // create the inverse based on reordered matrix
    Inverse_ = new T(ReorderedLocalizedMatrix_);

  }
  else
    // create the inverse based on original (non-reordered) matrix
    Inverse_ = new T(LocalizedMatrix_);

  if (Inverse_ == 0)
    IFPACK_CHK_ERR(-1);

  if (IsOverlapping()) {
    Importer_ = new Epetra_Import(OverlappingMatrix_->RowMatrixRowMap(), 
				  Matrix_->RowMatrixRowMap());
    Exporter_ = new Epetra_Export(Matrix_->RowMatrixRowMap(),
				  OverlappingMatrix_->RowMatrixRowMap());
    if ((Importer_ == 0) || (Exporter_ == 0))
      IFPACK_CHK_ERR(-1);
  }

  return(0);
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::SetParameters(Teuchos::ParameterList& List)
{
 
  CombineMode_ = List.get("schwarz: combine mode", CombineMode_);
  UseReordering_ = List.get("schwarz: use RCM reordering", UseReordering_);
  CondestMaxIters_ = List.get("condest: max iters", CondestMaxIters_);
  CondestTol_ = List.get("condest: tolerance", CondestTol_);

  // This copy may be needed by Amesos or other preconditioners.
  List_ = List;

  return(0);
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::Initialize()
{

  IsInitialized_ = false;
  IsComputed_ = false; // values required
  Condest_ = -1.0; // zero-out condest

  Destroy();

  // compute the overlapping matrix if necessary
  if (IsOverlapping_) {
    OverlappingMatrix_ = Ifpack_CreateOverlappingCrsMatrix(Matrix_,
							   OverlapLevel_);
    if (OverlappingMatrix_ == 0)
      IFPACK_CHK_ERR(-1);
  }

  IFPACK_CHK_ERR(Setup());

  if (Inverse_ == 0)
    IFPACK_CHK_ERR(-1);

  if (LocalizedMatrix_ == 0)
    IFPACK_CHK_ERR(-1);

  IFPACK_CHK_ERR(Inverse_->SetParameters(List_));
  IFPACK_CHK_ERR(Inverse_->Initialize());

  // Label is for Aztec-like solvers
  Label_ = "Ifpack_AdditiveSchwarz, ov = " + Ifpack_toString(OverlapLevel_)
    + ", local solver = \n\t\t***** `" + string(Inverse_->Label()) + "'";

  IsInitialized_ = true;

  return(0);
}
//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::Compute()
{

  if (IsInitialized() == false)
    IFPACK_CHK_ERR(Initialize());

  IsComputed_ = false;
  Condest_ = -1.0;
  
  IFPACK_CHK_ERR(Inverse_->Compute());

  IsComputed_ = true;

  string R = "";
  if (UseReordering_)
    R = "RCM reord, ";

  // reset lavel with condest()
  Label_ = "Ifpack_AdditiveSchwarz, ov = " + Ifpack_toString(OverlapLevel_)
    + ", local solver = \n\t\t***** `" + string(Inverse_->Label()) + "'"
    + "\n\t\t***** " + R + "Condition number estimate = "
    + Ifpack_toString(Condest());

  return(0);
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::SetUseTranspose(bool UseTranspose)
{
  IFPACK_CHK_ERR(-99); // not implemented
  return(-99);
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::
Apply(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const
{
  IFPACK_CHK_ERR(Matrix_->Apply(X,Y));
  return(0);
}

//==============================================================================
template<typename T>
double Ifpack_AdditiveSchwarz<T>::NormInf() const
{
  return(-1.0);
}

//==============================================================================
template<typename T>
char * Ifpack_AdditiveSchwarz<T>::Label() const
{
  return((char*)Label_.c_str());
}

//==============================================================================
template<typename T>
bool Ifpack_AdditiveSchwarz<T>::UseTranspose() const
{
  return(false);
}

//==============================================================================
template<typename T>
bool Ifpack_AdditiveSchwarz<T>::HasNormInf() const
{
  return(false);
}

//==============================================================================
template<typename T>
const Epetra_Comm & Ifpack_AdditiveSchwarz<T>::Comm() const
{
  return(Matrix_->Comm());
}

//==============================================================================
template<typename T>
const Epetra_Map & Ifpack_AdditiveSchwarz<T>::OperatorDomainMap() const
{
  return(Matrix_->OperatorDomainMap());
}

//==============================================================================
template<typename T>
const Epetra_Map & Ifpack_AdditiveSchwarz<T>::OperatorRangeMap() const
{
  return(Matrix_->OperatorRangeMap());
}

//==============================================================================
template<typename T>
int Ifpack_AdditiveSchwarz<T>::
ApplyInverse(const Epetra_MultiVector& X, Epetra_MultiVector& Y) const
{

  if (X.NumVectors() != Y.NumVectors())
    IFPACK_CHK_ERR(-1); // wrong input
  // check for maps??

  if (IsOverlapping() == false) {
    if (!UseReordering_)
      Inverse_->ApplyInverse(X,Y);
    else {
      Epetra_MultiVector Xtilde(X);
      Epetra_MultiVector Ytilde(Y);
      Reordering_->P(X,Xtilde);
      Inverse_->ApplyInverse(Xtilde,Ytilde);
      Reordering_->Pinv(Ytilde,Y);
    }
  }
  else {
    // a bit more with overlap
    Epetra_MultiVector OverlappingX(OverlappingMatrix_->RowMatrixRowMap(),
				     X.NumVectors());
    Epetra_MultiVector OverlappingY(OverlappingMatrix_->RowMatrixRowMap(),
				     Y.NumVectors());

    IFPACK_CHK_ERR(OverlappingX.Export(X,*Exporter_,Insert));
    Epetra_MultiVector LocalX(View,LocalizedMatrix_->RowMatrixRowMap(),
			      OverlappingX.Pointers(),X.NumVectors());
    Epetra_MultiVector LocalY(View,LocalizedMatrix_->RowMatrixRowMap(),
			      OverlappingY.Pointers(),Y.NumVectors());

    IFPACK_CHK_ERR(Inverse_->ApplyInverse(LocalX,LocalY));

    IFPACK_CHK_ERR(Y.Export(OverlappingY,*Importer_,CombineMode_));
  }

  return(0);
 
}

//==============================================================================
template<typename T>
std::ostream& Ifpack_AdditiveSchwarz<T>::
Print(std::ostream& os) const
{
  if( Matrix().Comm().MyPID())
    return(os);

  os << "*** " << Label() << endl;
  if (CombineMode_ == Insert)
    os << "*** Combine mode              = Insert" << endl;
  else if (CombineMode_ == Add)
    os << "*** Combine mode              = Add" << endl;
  else if (CombineMode_ == Zero)
    os << "*** Combine mode              = Zero" << endl;
  else if (CombineMode_ == Average)
    os << "*** Combine mode              = Average" << endl;
  else if (CombineMode_ == AbsMax)
    os << "*** Combine mode              = AbsMax" << endl;
  os << "*** Overlap Level               = " << OverlapLevel_ << endl;
  os << "*** Condition number estimate = " << Condest_ << endl;

  os << "*** Inverse label             = `" 
     << Inverse_->Label() << "'" << endl;

  return(os);
}

#include "Ifpack_Condest.h"
//==============================================================================
template<typename T>
double Ifpack_AdditiveSchwarz<T>::
Condest(const Ifpack_CondestType CT, 
	Epetra_RowMatrix* Matrix)
{
  if (!IsComputed()) // cannot compute right now
    return(-1.0);

  if (Condest_ == -1.0)
    Condest_ = Ifpack_Condest(*this, CT, CondestMaxIters_, CondestTol_,
			      Matrix);

  return(Condest_);
}
#endif // IFPACK_ADDITIVESCHWARZ_H
