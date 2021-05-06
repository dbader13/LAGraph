//------------------------------------------------------------------------------
// LAGraph_BinRead: read a matrix from a binary file
//------------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
// Contributed by Tim Davis, Texas A&M University.

//------------------------------------------------------------------------------

#include "LG_internal.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    if (f != NULL)          \
    {                       \
        fclose (f) ;        \
        f = NULL ;          \
    }                       \
    GrB_free (A) ;          \
    LAGRAPH_FREE (Ap) ;     \
    LAGRAPH_FREE (Ah) ;     \
    LAGRAPH_FREE (Ab) ;     \
    LAGRAPH_FREE (Ai) ;     \
    LAGRAPH_FREE (Ax) ;     \
}

#define FREAD(p,s,n)                                                \
{                                                                   \
    size_t result = fread (p, s, n, f) ;                            \
    LG_CHECK (result != n, -1, "file I/O error") ;                  \
}

int LAGraph_BinRead         // returns 0 if successful, -1 if failure
(
    GrB_Matrix *A,          // matrix to read from the file
    char *filename,         // file to read it from
    char *msg
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Index *Ap = NULL, *Ai = NULL, *Ah = NULL ;
    int8_t *Ab = NULL ;
    void *Ax = NULL ;
    FILE *f = NULL ;

    LG_CHECK (A == NULL, -1, "&A is NULL") ;
    LG_CHECK (filename == NULL, -1, "filename is NULL") ;
    (*A) = NULL ;

    //--------------------------------------------------------------------------
    // open the file
    //--------------------------------------------------------------------------

    f = fopen (filename, "r") ;
    LG_CHECK (f == NULL, -1, "cannot open file") ;

    //--------------------------------------------------------------------------
    // basic matrix properties
    //--------------------------------------------------------------------------

    GxB_Format_Value fmt = -999 ;
    bool is_hyper, is_sparse, is_bitmap, is_full ;
    int32_t kind, typecode ;
    double hyper = -999 ;
    GrB_Type type ;
    GrB_Index nrows, ncols, nvals, nvec ;
    size_t typesize ;
    int64_t nonempty ;

    //--------------------------------------------------------------------------
    // read the header (and ignore it)
    //--------------------------------------------------------------------------

    // The header is informational only, for "head" command, so the file can
    // be visually inspected.

    char header [LAGRAPH_BIN_HEADER] ;
    FREAD (header, sizeof (char), LAGRAPH_BIN_HEADER) ;
    // printf ("%s\n", header) ;

    //--------------------------------------------------------------------------
    // read the scalar content
    //--------------------------------------------------------------------------

    FREAD (&fmt,      sizeof (GxB_Format_Value), 1) ;
    FREAD (&kind,     sizeof (int32_t), 1) ;
    FREAD (&hyper,    sizeof (double), 1) ;
    FREAD (&nrows,    sizeof (GrB_Index), 1) ;
    FREAD (&ncols,    sizeof (GrB_Index), 1) ;
    FREAD (&nonempty, sizeof (int64_t), 1) ;
    FREAD (&nvec,     sizeof (GrB_Index), 1) ;
    FREAD (&nvals,    sizeof (GrB_Index), 1) ;
    FREAD (&typecode, sizeof (int32_t), 1) ;
    FREAD (&typesize, sizeof (size_t), 1) ;

    is_hyper  = (kind == 1) ;
    is_sparse = (kind == 0 || kind == GxB_SPARSE) ;
    is_bitmap = (kind == GxB_BITMAP) ;
    is_full   = (kind == GxB_FULL) ;

    switch (typecode)
    {
        case 0:  type = GrB_BOOL        ; break ;
        case 1:  type = GrB_INT8        ; break ;
        case 2:  type = GrB_INT16       ; break ;
        case 3:  type = GrB_INT32       ; break ;
        case 4:  type = GrB_INT64       ; break ;
        case 5:  type = GrB_UINT8       ; break ;
        case 6:  type = GrB_UINT16      ; break ;
        case 7:  type = GrB_UINT32      ; break ;
        case 8:  type = GrB_UINT64      ; break ;
        case 9:  type = GrB_FP32        ; break ;
        case 10: type = GrB_FP64        ; break ;
        case 11: type = GxB_FC32        ; break ;
        case 12: type = GxB_FC64        ; break ;
        default: LG_CHECK (false, -1, "unknown type") ;
    }

    //--------------------------------------------------------------------------
    // allocate the array content
    //--------------------------------------------------------------------------

    GrB_Index Ap_len = 0, Ap_size = 0 ;
    GrB_Index Ah_len = 0, Ah_size = 0 ;
    GrB_Index Ab_len = 0, Ab_size = 0 ;
    GrB_Index Ai_len = 0, Ai_size = 0 ;
    GrB_Index Ax_len = 0, Ax_size = 0 ;

    // v4.0.1: hypersparse, sparse, bitmap, and full
    // v3.3.3 and v4.0.0: only hypersparse and sparse
    bool ok = true ;
    if (is_hyper)
    {
        Ap_len = nvec+1 ;
        Ah_len = nvec ;
        Ai_len = nvals ;
        Ax_len = nvals ;
        Ap = LAGraph_Malloc (Ap_len, sizeof (GrB_Index), &Ap_size) ;
        Ah = LAGraph_Malloc (Ah_len, sizeof (GrB_Index), &Ah_size) ;
        Ai = LAGraph_Malloc (Ai_len, sizeof (GrB_Index), &Ai_size) ;
        ok = (Ap != NULL && Ah != NULL && Ai != NULL) ;
    }
    else if (is_sparse)
    {
        Ap_len = nvec+1 ;
        Ai_len = nvals ;
        Ax_len = nvals ;
        Ap = LAGraph_Malloc (Ap_len, sizeof (GrB_Index), &Ap_size) ;
        Ai = LAGraph_Malloc (Ai_len, sizeof (GrB_Index), &Ai_size) ;
        ok = (Ap != NULL && Ai != NULL) ;
    }
    else if (is_bitmap)
    {
        Ab_len = nrows*ncols ;
        Ax_len = nrows*ncols ;
        Ab = LAGraph_Malloc (nrows*ncols, sizeof (int8_t), &Ab_size) ;
        ok = (Ab != NULL) ;
    }
    else if (is_full)
    {
        Ax_len = nrows*ncols ;
    }
    else
    {
        LG_CHECK (false, -1, "unknown matrix format") ;
    }
    Ax = LAGraph_Malloc (Ax_len, typesize, &Ax_size) ;
    LG_CHECK (!ok || Ax == NULL, -1, "out of memory") ;

    //--------------------------------------------------------------------------
    // read the array content
    //--------------------------------------------------------------------------

    if (is_hyper)
    {
        FREAD (Ap, sizeof (GrB_Index), Ap_len) ;
        FREAD (Ah, sizeof (GrB_Index), Ah_len) ;
        FREAD (Ai, sizeof (GrB_Index), Ai_len) ;
    }
    else if (is_sparse)
    {
        FREAD (Ap, sizeof (GrB_Index), Ap_len) ;
        FREAD (Ai, sizeof (GrB_Index), Ai_len) ;
    }
    else if (is_bitmap)
    {
        FREAD (Ab, sizeof (int8_t), Ab_len) ;
    }

    FREAD (Ax, typesize, Ax_len) ;
    fclose (f) ;
    f = NULL ;

    //--------------------------------------------------------------------------
    // import the matrix
    //--------------------------------------------------------------------------

    #if GxB_IMPLEMENTATION >= GxB_VERSION (5,0,1)
    // in SuiteSparse:GraphBLAS v5, sizes are in bytes, not entries
    GrB_Index Ap_siz = Ap_size ;
    GrB_Index Ah_siz = Ah_size ;
    GrB_Index Ab_siz = Ab_size ;
    GrB_Index Ai_siz = Ai_size ;
    GrB_Index Ax_siz = Ax_size ;
    #define IS_UNIFORM ,false

    #elif GxB_IMPLEMENTATION >= GxB_VERSION (5,0,0)
    // in SuiteSparse:GraphBLAS v5, sizes are in bytes, not entries
    GrB_Index Ap_siz = Ap_size ;
    GrB_Index Ah_siz = Ah_size ;
    GrB_Index Ab_siz = Ab_size ;
    GrB_Index Ai_siz = Ai_size ;
    GrB_Index Ax_siz = Ax_size ;
    #define IS_UNIFORM

    #else
    // in SuiteSparse:GraphBLAS v4, sizes are in # of entries, not bytes
    GrB_Index Ap_siz = Ap_len ;
    GrB_Index Ah_siz = Ah_len ;
    GrB_Index Ab_siz = Ab_len ;
    GrB_Index Ai_siz = Ai_len ;
    GrB_Index Ax_siz = Ax_len ;
    #define IS_UNIFORM
    #endif

    if (fmt == GxB_BY_COL && is_hyper)
    {
        // hypersparse CSC
        GrB_TRY (GxB_Matrix_import_HyperCSC (A, type, nrows, ncols,
            &Ap, &Ah, &Ai, &Ax, Ap_siz, Ah_siz, Ai_siz, Ax_siz IS_UNIFORM,
            nvec, false, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && is_hyper)
    {
        // hypersparse CSR
        GrB_TRY (GxB_Matrix_import_HyperCSR (A, type, nrows, ncols,
            &Ap, &Ah, &Ai, &Ax, Ap_siz, Ah_siz, Ai_siz, Ax_siz IS_UNIFORM,
            nvec, false, NULL)) ;
    }
    else if (fmt == GxB_BY_COL && is_sparse)
    {
        // standard CSC
        GrB_TRY (GxB_Matrix_import_CSC (A, type, nrows, ncols,
            &Ap, &Ai, &Ax, Ap_siz, Ai_siz, Ax_siz IS_UNIFORM, false, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && is_sparse)
    {
        // standard CSR
        GrB_TRY (GxB_Matrix_import_CSR (A, type, nrows, ncols,
            &Ap, &Ai, &Ax, Ap_siz, Ai_siz, Ax_siz IS_UNIFORM, false, NULL)) ;
    }
    else if (fmt == GxB_BY_COL && is_bitmap)
    {
        // bitmap by col
        GrB_TRY (GxB_Matrix_import_BitmapC (A, type, nrows, ncols,
            &Ab, &Ax, Ab_siz, Ax_siz IS_UNIFORM, nvals, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && is_bitmap)
    {
        // bitmap by row
        GrB_TRY (GxB_Matrix_import_BitmapR (A, type, nrows, ncols,
            &Ab, &Ax, Ab_siz, Ax_siz IS_UNIFORM, nvals, NULL)) ;
    }
    else if (fmt == GxB_BY_COL && is_full)
    {
        // full by col
        GrB_TRY (GxB_Matrix_import_FullC (A, type, nrows, ncols,
            &Ax, Ax_siz IS_UNIFORM, NULL)) ;
    }
    else if (fmt == GxB_BY_ROW && is_full)
    {
        // full by row
        GrB_TRY (GxB_Matrix_import_FullR (A, type, nrows, ncols,
            &Ax, Ax_siz IS_UNIFORM, NULL)) ;
    }
    else
    {
        LG_CHECK (false, -1, "unknown format") ;
    }

    GrB_TRY (GxB_set (*A, GxB_HYPER_SWITCH, hyper)) ;

    return (0) ;
}

