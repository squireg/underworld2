/*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*
**                                                                                  **
** This file forms part of the Underworld geophysics modelling application.         **
**                                                                                  **
** For full license and copyright information, please refer to the LICENSE.md file  **
** located at the project root, or contact the authors.                             **
**                                                                                  **
**~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*/

#ifndef __lucDatabase_h__
#define __lucDatabase_h__

#include "ViewerTypes.h"
#include "sqlite3.h"

struct lucColourMap;
struct lucDrawingObject;

#include "types.h"

#define MAX_PATH 1024

/* For collating Geometry Data before writing to database */
typedef struct
{
   float* data;
   int size;
   int count;
   int allocated;
   int width;
   int height;
   float minimum;
   float maximum;
   float min[3];
   float max[3];
   char* labels;     /* Label strings */
} lucGeometryData;

extern const Type lucDatabase_Type;

/** Class contents - this is defined as a macro so that sub-classes of this class can use this macro at the start of the definition of their struct */
#define __lucDatabase                                       \
      /* Macro defining parent goes here - This means you can cast this class as its parent */ \
      __Stg_Component                                       \
      /* Other Info */ \
      DomainContext*    context;                            \
      /* Internal */ \
      NamedObject_Register* drawingObjects;                 \
      char*             labels[lucMaxType];                 \
      unsigned int      label_lengths[lucMaxType];          \
      lucGeometryData*  data[lucMaxType][lucMaxDataType];   \
      char              bin_path[MAX_PATH];                 \
      /* Database */ \
      sqlite3*          db;                     \
      sqlite3*          db2;                    \
      sqlite3*          memdb;                  \
      char              path[MAX_PATH];         \
      /* Params */ \
      char*             filename;               \
      char*             vfs;                    \
      Bool              compressed;             \
      Bool              singleFile;             \
      Bool              splitTransactions;      \
      int               deleteAfter;            \
      Bool              viewonly;               \
      int               rank;                   \
      int               nproc;                  \
      MPI_Comm          communicator;           \
      int               timeStep;

struct lucDatabase
{
   __lucDatabase
};


#ifndef ZERO
#define ZERO 0
#endif

#define LUCDATABASE_DEFARGS \
                STG_COMPONENT_DEFARGS

#define LUCDATABASE_PASSARGS \
                STG_COMPONENT_PASSARGS


lucDatabase* _lucDatabase_New(  LUCDATABASE_DEFARGS  );

/* A public constructor for use in Window when creating default database */
lucDatabase* lucDatabase_New(
   AbstractContext*  context,
   int               deleteAfter,
   Bool              splitTransactions,
   Bool              compressed,
   Bool              singleFile,
   char*             filename,
   char*             vfs);

void _lucDatabase_Delete( void* database ) ;

void* _lucDatabase_DefaultNew( Name name ) ;
void _lucDatabase_AssignFromXML( void* database, Stg_ComponentFactory* cf, void* data ) ;
void _lucDatabase_Build( void* database, void* data );
void _lucDatabase_Initialise( void* database, void* data );
void _lucDatabase_Execute( void* database, void* data );
void _lucDatabase_Destroy( void* database, void* data );

void lucDatabase_Dump(void* database);

void lucDatabase_OutputDrawingObject(lucDatabase* self, lucDrawingObject* object);
void lucDatabase_OutputColourMap(lucDatabase* self, lucColourMap* colourMap, lucDrawingObject* object, lucGeometryDataType type);

void lucDatabase_ClearGeometry(lucDatabase* self);
void lucDatabase_OutputGeometry(lucDatabase* self, int object_id);
int lucDatabase_GatherCounts(lucDatabase* self, int count, int* counts, int* offsets);
void lucDatabase_GatherGeometry(lucDatabase* self, lucGeometryType type, lucGeometryDataType data_type);
void lucDatabase_GatherLabels(lucDatabase* self, lucGeometryType type);

struct lucIsosurface;
void lucDatabase_AddGridVertices(lucDatabase* self, int n, int width, float* data);
void lucDatabase_AddGridVertex(lucDatabase* self, int width, int height, float* data);
void lucDatabase_AddVertices(lucDatabase* self, int n, lucGeometryType type, float* data);
void lucDatabase_AddVerticesWidth(lucDatabase* self, int n, lucGeometryType type, int width, float* data);
void lucDatabase_AddNormals(lucDatabase* self, int n, lucGeometryType type, float* data);
void lucDatabase_AddNormal(lucDatabase* self, lucGeometryType type, XYZ norm);
void lucDatabase_AddVectors(lucDatabase* self, int n, lucGeometryType type, float min, float max, float* data);
void lucDatabase_AddValues(lucDatabase* self, int n, lucGeometryType type, lucGeometryDataType data_type, lucColourMap* colourMap, float* data);
void lucDatabase_AddIndex(lucDatabase* self, lucGeometryType type, unsigned int index);
void lucDatabase_AddRGBA(lucDatabase* self, lucGeometryType type, float opacity, lucColour* colour);
void lucDatabase_AddTexCoord(lucDatabase* self, lucGeometryType type, float x, float y);
void lucDatabase_AddLabel(lucDatabase* self, lucGeometryType type, char* label);
void lucDatabase_AddVolumeSlice(lucDatabase* self, int width, int height, float* corners, lucColourMap* colourMap, float* data);

lucGeometryData* lucGeometryData_New(lucGeometryDataType data_type);
void lucGeometryData_Clear(lucGeometryData* self);
void lucGeometryData_Delete(lucGeometryData* self);
void lucGeometryData_Read(lucGeometryData* self, int items, float* data); //, int width, int height)
void lucGeometryData_Setup(lucGeometryData* self, float min, float max);

void lucDatabase_OpenDatabase(lucDatabase* self);
void lucDatabase_CreateDatabase(lucDatabase* self);
Bool lucDatabase_IssueSQL(sqlite3* db, const char* SQL);
Bool lucDatabase_BeginTransaction(lucDatabase* self);
void lucDatabase_Commit(lucDatabase* self);
void lucDatabase_AttachDatabase(lucDatabase* self);
void lucDatabase_DeleteGeometry(lucDatabase* self, int start_timestep, int end_timestep);

int lucDatabase_WriteGeometry(lucDatabase* self, int index, lucGeometryType type, lucGeometryDataType data_type, int object_id, lucGeometryData* block);

void lucDatabase_BackupDb(sqlite3 *fromDb, sqlite3* toDb);
void lucDatabase_BackupDbFile(lucDatabase* self, char* filename);
void lucDatabase_WriteState(lucDatabase* self, const char* name, const char* properties);

#endif

