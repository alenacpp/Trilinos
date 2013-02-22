// -*- c++ -*-

// @HEADER
// ***********************************************************************
//
//              PyTrilinos: Python Interface to Trilinos
//                 Copyright (2005) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
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
// Questions? Contact Bill Spotz (wfspotz@sandia.gov)
//
// ***********************************************************************
// @HEADER

%define %loca_bifurcation_docstring
"
PyTrilinos.LOCA.Bifurcation is the python interface to namespace Bifurcation
of the Trilinos continuation algorithm package LOCA:

    http://trilinos.sandia.gov/packages/nox

The purpose of LOCA.Bifurcation is to provide strategies for creating
bifurcation objects.  The python version of LOCA.Bifurcation supports
the following classes:

    * Factory  - 

Any other notes about the package as a whole. . . .
"
%enddef

%module(package   = "PyTrilinos.LOCA",
        docstring = %loca_bifurcation_docstring) Bifurcation

%{
// PyTrilinos includes
#include "PyTrilinos_Teuchos_Util.h"

// LOCA includes
#include "LOCA.H"

// Local includes
#define NO_IMPORT_ARRAY
#include "numpy_include.h"
%}

// Include LOCA documentation
%feature("autodoc", "1");
%include "LOCA_dox.i"

// Ignore/renames
%ignore operator=;
%rename(Print) *::print(ostream& stream, int indent = 0) const;

// Import base class declarations
%import "Teuchos.i"
%import "LOCA.MultiContinuation.i"
%import "LOCA.Parameter.i"

// LOCA::Bifurcation Factory class
%include "LOCA_Bifurcation_Factory.H"
