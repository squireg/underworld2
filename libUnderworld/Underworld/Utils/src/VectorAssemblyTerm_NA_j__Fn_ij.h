/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/


#ifndef __Underworld_Utils_VectorAssemblyTerm_NA_j__Fn_ij_h__
#define __Underworld_Utils_VectorAssemblyTerm_NA_j__Fn_ij_h__

#ifdef __cplusplus

extern "C++" {

#include <Underworld/Function/Function.hpp>
#include <Underworld/Function/FEMCoordinate.hpp>

struct VectorAssemblyTerm_NA_j__Fn_ij_cppdata
{
    Fn::Function* fn;
    Fn::Function::func func;
    std::shared_ptr<FEMCoordinate> input;
};

void _VectorAssemblyTerm_NA_j__Fn_ij_SetFn( void* _self, Fn::Function* fn );

}

extern "C" {
#endif

#include <StGermain/StGermain.h>
#include <StgDomain/StgDomain.h>
#include <StgFEM/StgFEM.h>
#include <PICellerator/PICellerator.h>
#include <Underworld/Underworld.h>
#include "types.h"

   /** Textual name of this class */
   extern const Type VectorAssemblyTerm_NA_j__Fn_ij_Type;

   /** VectorAssemblyTerm_NA_j__Fn_ij class contents */
   #define __VectorAssemblyTerm_NA_j__Fn_ij \
      /* General info */ \
      __ForceTerm \
      \
      /* Virtual info */ \
      void*       funeForce;   \
      double**    GNx; \
      int         maxNodesPerEl;

   struct VectorAssemblyTerm_NA_j__Fn_ij { __VectorAssemblyTerm_NA_j__Fn_ij };


   #ifndef ZERO
   #define ZERO 0
   #endif

   #define FORCEASSEMBLYTERM_NA__FN_DEFARGS \
                FORCETERM_DEFARGS

   #define FORCEASSEMBLYTERM_NA__FN_PASSARGS \
                FORCETERM_PASSARGS

   VectorAssemblyTerm_NA_j__Fn_ij* _VectorAssemblyTerm_NA_j__Fn_ij_New(  FORCEASSEMBLYTERM_NA__FN_DEFARGS  );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Delete( void* residual );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Print( void* residual, Stream* stream );

   void* _VectorAssemblyTerm_NA_j__Fn_ij_DefaultNew( Name name );

   void _VectorAssemblyTerm_NA_j__Fn_ij_AssignFromXML( void* residual, Stg_ComponentFactory* cf, void* data );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Build( void* residual, void* data );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Initialise( void* residual, void* data );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Execute( void* residual, void* data );

   void _VectorAssemblyTerm_NA_j__Fn_ij_Destroy( void* residual, void* data );

   void _VectorAssemblyTerm_NA_j__Fn_ij_AssembleElement( void* forceTerm, ForceVector* forceVector, Element_LocalIndex lElement_I, double* elForceVec );


#ifdef __cplusplus
}
#endif

#endif

