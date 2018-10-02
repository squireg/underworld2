/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#include <stdio.h>
#include <stdlib.h>

#include "pcu/pcu.h"
#include <StGermain/StGermain.h> 
#include "StgDomain/Geometry/Geometry.h"
#include "StgDomain/Shape/Shape.h"
#include "StgDomain/Mesh/Mesh.h" 
#include "StgDomain/Utils/Utils.h"
#include "StgDomain/Swarm/Swarm.h"

#include "DofLayoutSuite.h"

typedef struct {
   MPI_Comm comm;
   int      rank;
   int      nProcs;
} DofLayoutSuiteData;

void DofLayoutSuite_Setup( DofLayoutSuiteData* data ) {
   /* MPI Initializations */
   data->comm = MPI_COMM_WORLD;
   MPI_Comm_rank( data->comm, &data->rank );
   MPI_Comm_size( data->comm, &data->nProcs );
}

void DofLayoutSuite_Teardown( DofLayoutSuiteData* data ) {
}

void DofLayoutSuite_TestBasic( DofLayoutSuiteData* data ) {
   char    expected_file[PCU_PATH_MAX];
   int     procToWatch;
   Stream* stream = Journal_Register( Info_Type, (Name)"DofLayoutBasic" );   

   procToWatch = data->nProcs >=2 ? 1 : 0;

   if( data->rank == procToWatch  ) {
      DofLayout*         dof;
      DofLayout*         destDof;
      Variable_Register* variableRegister;
      StgVariable*          var[6];
      char*              varName[] = {"x", "y", "z", "vx", "vy", "vz"};
      Index              ii, dof_I, var_I;
      Index              arraySize = 27;
      double*            varArrays[6];
      int                counts[27];

      Stream_RedirectFile( stream, "testBasic.dat" );

      /* Create variable register */
      variableRegister = Variable_Register_New();

      /* Create variables */
      for (var_I = 0; var_I < 6; var_I++) {
         varArrays[var_I] = Memory_Alloc_Array_Unnamed( double, arraySize );
         var[var_I] = StgVariable_NewScalar( varName[var_I], NULL, StgVariable_DataType_Double, (Index*)&arraySize, NULL, (void**)&(varArrays[var_I]), variableRegister  );
         Stg_Component_Build( var[var_I], 0, False );
         Stg_Component_Initialise( var[var_I], 0, False );
      }

      for (ii = 0; ii < arraySize; ii++) {
         for (var_I = 0; var_I < 6; var_I++) {
            StgVariable_SetValueDouble( var[var_I], ii, 0.0 );
         }
      }

      /* Simple test */
      dof = DofLayout_New( "dofLayout", variableRegister, arraySize, NULL );
      for (ii = 0; ii < arraySize; ii++)
         for (var_I = 0; var_I < 6; var_I++)
            DofLayout_AddDof_ByVarName(dof, varName[var_I], ii);

      Stg_Component_Build(dof, 0, False);

      Journal_Printf( stream, "Simple test:\n" );
        for (ii = 0; ii < arraySize; ii++) {
         Journal_Printf( stream, "\t%u\n", dof->dofCounts[ii] );
         pcu_check_true( dof->dofCounts[ii] == 6 );
      }
         
      Stg_Class_Delete(dof);

      /* Advanced test */
      for (ii = 0; ii < 27; ii++) counts[ii] = 0;
      dof = DofLayout_New( "dofLayout1", variableRegister, arraySize, NULL );
      
      for (ii = 0; ii < 12; ii++) {
         for (var_I = 0; var_I < 2; var_I++) {
            DofLayout_AddDof_ByVarName(dof, varName[var_I], ii);
            counts[ii]++;
         }
      }
 
      for (ii = 9; ii < 20; ii++) {
         for (var_I = 2; var_I < 6; var_I++) {
            DofLayout_AddDof_ByVarName(dof, varName[var_I], ii);
            counts[ii]++;
         }
      }
 
      Stg_Component_Build(dof, 0, False);
 
      Journal_Printf( stream, "\nAdvanced test:\n" );
      for (ii = 0; ii < arraySize; ii++) {
         Journal_Printf( stream, "\t%u\n", dof->dofCounts[ii] );
         pcu_check_true( counts[ii] == dof->dofCounts[ii] );
      }
      Stg_Class_Delete(dof);

      /* Copy test */
       dof = DofLayout_New( "dofLayout2", variableRegister, arraySize, NULL );
      destDof = DofLayout_New( "dofLayout3", variableRegister, arraySize, NULL );
      for (ii = 0; ii < arraySize; ii++) {
         for (var_I = 0; var_I < 3; var_I++) {
            DofLayout_AddDof_ByVarName(dof, varName[var_I], ii);
         }
         for (var_I = 3; var_I < 6; var_I++) {
            DofLayout_AddDof_ByVarName(destDof, varName[var_I], ii);
         }
      }

      Stg_Component_Build(dof, NULL, False);
      Stg_Component_Build(destDof, NULL, False);

      for (ii = 0; ii < arraySize; ii++) {
         for (dof_I = 0; dof_I < 3; dof_I++) {
            DofLayout_SetValueDouble( dof, ii, dof_I, ii*10 );
            DofLayout_SetValueDouble( destDof, ii, dof_I, 0 );
         }
      }

      Journal_Printf( stream, "Copy Test: pre copy:\n" );
      for (ii = 0; ii < arraySize; ii++) {
         Journal_Printf( stream, "\tIndex %d - src %2g,%2g,%2g - dest %2g, %2g, %2g\n", ii,
            DofLayout_GetValueDouble( dof, ii, 0 ),
            DofLayout_GetValueDouble( dof, ii, 1 ),
            DofLayout_GetValueDouble( dof, ii, 2 ),
            DofLayout_GetValueDouble( destDof, ii, 0 ),
            DofLayout_GetValueDouble( destDof, ii, 1 ),
            DofLayout_GetValueDouble( destDof, ii, 2 ) );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 0 ) == ii * 10 );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 1 ) == ii * 10 );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 2 ) == ii * 10 );
         pcu_check_true( DofLayout_GetValueDouble( destDof, ii, 0 ) == 0 );
         pcu_check_true( DofLayout_GetValueDouble( destDof, ii, 1 ) == 0 );
         pcu_check_true( DofLayout_GetValueDouble( destDof, ii, 2 ) == 0 );
      }

      DofLayout_CopyValues( dof, destDof );

      Journal_Printf( stream, "Copy Test: post copy:\n" );
      for (ii = 0; ii < arraySize; ii++) {
         Journal_Printf( stream, "\tIndex %d - src %2g,%2g,%2g - dest %2g, %2g, %2g\n", ii,
            DofLayout_GetValueDouble( dof, ii, 0 ),
            DofLayout_GetValueDouble( dof, ii, 1 ),
            DofLayout_GetValueDouble( dof, ii, 2 ),
            DofLayout_GetValueDouble( destDof, ii, 0 ),
            DofLayout_GetValueDouble( destDof, ii, 1 ),
            DofLayout_GetValueDouble( destDof, ii, 2 ) );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 0 ) == DofLayout_GetValueDouble( destDof, ii, 0 ) );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 1 ) == DofLayout_GetValueDouble( destDof, ii, 1 ) );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 2 ) == DofLayout_GetValueDouble( destDof, ii, 2 ) );
      }

      Stg_Class_Delete(destDof);

      Journal_Printf( stream, "Zero Test: all values in src dof should be zero again\n" );
      DofLayout_SetAllToZero( dof );
      for (ii = 0; ii < arraySize; ii++) {
         Journal_Printf( stream,  "\tIndex %d - src %2g,%2g,%2g\n", ii,
            DofLayout_GetValueDouble( dof, ii, 0 ),
            DofLayout_GetValueDouble( dof, ii, 1 ),
            DofLayout_GetValueDouble( dof, ii, 2 ) );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 0 ) == 0 );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 1 ) == 0 );
         pcu_check_true( DofLayout_GetValueDouble( dof, ii, 2 ) == 0 );
      }

      Stg_Class_Delete(dof);

      /* Cleanup */

      Stg_Class_Delete(variableRegister);

      for (var_I = 0; var_I < 6; var_I++) {
         if (var[var_I]) Stg_Class_Delete(var[var_I]);
            Memory_Free( varArrays[var_I] );
      }
      pcu_filename_expected( "testDofLayoutBasicOutput.expected", expected_file );
      pcu_check_fileEq( "testBasic.dat", expected_file );
      remove( "testBasic.dat" );
   }
}

void DofLayoutSuite( pcu_suite_t* suite ) {
   pcu_suite_setData( suite, DofLayoutSuiteData );
   pcu_suite_setFixtures( suite, DofLayoutSuite_Setup, DofLayoutSuite_Teardown );
   pcu_suite_addTest( suite, DofLayoutSuite_TestBasic );
}


