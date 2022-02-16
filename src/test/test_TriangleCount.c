//----------------------------------------------------------------------------
// LAGraph/src/test/test_TriangleCount.c: test cases for triangle
// counting algorithms
// ----------------------------------------------------------------------------

// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// See additional acknowledgments in the LICENSE file,
// or contact permission@sei.cmu.edu for the full terms.

// Contributed by Scott McMillan, SEI, and Tim Davis, Texas A&M University

//-----------------------------------------------------------------------------

#include <stdio.h>
#include <acutest.h>

#include <LAGraph_test.h>
#include <graph_zachary_karate.h>

char msg[LAGRAPH_MSG_LEN];
LAGraph_Graph G = NULL;

#define LEN 512
char filename [LEN+1] ;

typedef struct
{
    uint64_t ntriangles ;           // # triangles in original matrix
    const char *name ;              // matrix filename
}
matrix_info ;

const matrix_info files [ ] =
{
    {     45, "karate.mtx" },
    {     11, "A.mtx" },
    {   2016, "jagmesh7.mtx" },
    {      6, "ldbc-cdlp-undirected-example.mtx" },
    {      4, "ldbc-undirected-example.mtx" },
    {      5, "ldbc-wcc-example.mtx" },
    {      0, "LFAT5.mtx" },
    { 342300, "bcsstk13.mtx" },
    {      0, "tree-example.mtx" },
    {      0, "" },
} ;

//****************************************************************************
void setup(void)
{
    LAGraph_Init(msg);
    int retval;

    GrB_Matrix A = NULL;

    GrB_Matrix_new(&A, GrB_UINT32, ZACHARY_NUM_NODES, ZACHARY_NUM_NODES);
    GrB_Matrix_build(A, ZACHARY_I, ZACHARY_J, ZACHARY_V, ZACHARY_NUM_EDGES,
                     GrB_LOR);

    retval = LAGraph_New(&G, &A, LAGRAPH_ADJACENCY_UNDIRECTED, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_Property_NDiag(G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK(G->ndiag == 0);
}

//****************************************************************************
void teardown(void)
{
    int retval = LAGraph_Delete(&G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    G = NULL;
    LAGraph_Finalize(msg);
}

//****************************************************************************
//****************************************************************************
void test_TriangleCount_Methods1(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    // with presort
    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_Burkhardt = 1,    // sum (sum ((A^2) .* A)) / 6
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Burkhardt, &presort, msg);

    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles);

    teardown();
}

//****************************************************************************
void test_TriangleCount_Methods2(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_Cohen = 2,        // sum (sum ((L * U) .* A)) / 2
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Cohen, &presort, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles);

    teardown();
}

//****************************************************************************
void test_TriangleCount_Methods3(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_Sandia = 3,       // sum (sum ((L * L) .* L))
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Sandia, &presort, msg);
    TEST_CHECK(retval == LAGRAPH_PROPERTY_MISSING);  // should fail (rowdegrees needs to be defined)
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_Property_RowDegree(G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Sandia, &presort, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles);

    teardown();
}

//****************************************************************************
void test_TriangleCount_Methods4(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_Sandia2 = 4,      // sum (sum ((U * U) .* U))
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Sandia2, &presort, msg);
    TEST_CHECK(retval == LAGRAPH_PROPERTY_MISSING);  // should fail (rowdegrees needs to be defined)
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_Property_RowDegree(G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_Sandia2, &presort, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles) ;

    teardown();
}

//****************************************************************************
void test_TriangleCount_Methods5(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_SandiaDot = 5,    // sum (sum ((L * U') .* L))
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_SandiaDot, &presort, msg);
    TEST_CHECK(retval == LAGRAPH_PROPERTY_MISSING);  // should fail (rowdegrees needs to be defined)
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_Property_RowDegree(G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_SandiaDot, &presort, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles) ;

    teardown();
}

//****************************************************************************
void test_TriangleCount_Methods6(void)
{
    setup();
    int retval;
    uint64_t ntriangles = 0UL;

    LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
    ntriangles = 0UL;
    // LAGraph_TriangleCount_SandiaDot2 = 6,   // sum (sum ((U * L') .* U))
    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_SandiaDot2 , &presort, msg);
    TEST_CHECK(retval == LAGRAPH_PROPERTY_MISSING);  // should fail (rowdegrees needs to be defined)
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_Property_RowDegree(G, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    retval = LAGraph_TriangleCount_Methods(&ntriangles, G,
        LAGraph_TriangleCount_SandiaDot2 , &presort, msg);
    TEST_CHECK(retval == 0);
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles) ;

    teardown();
}

//****************************************************************************
void test_TriangleCount(void)
{
    setup();

    uint64_t ntriangles = 0UL;
    int retval = LAGraph_TriangleCount(&ntriangles, G, msg);
    TEST_CHECK(retval == 0);  // should not fail (rowdegrees will be calculated)
    TEST_MSG("retval = %d (%s)", retval, msg);

    TEST_CHECK( ntriangles == 45 );
    TEST_MSG("numtri = %g", (double) ntriangles) ;

    OK (LG_check_tri (&ntriangles, G, msg)) ;
    TEST_CHECK( ntriangles == 45 );

    teardown();
}

//****************************************************************************
void test_TriangleCount_many (void)
{
    LAGraph_Init(msg);
    GrB_Matrix A = NULL ;
    printf ("\n") ;

    for (int k = 0 ; ; k++)
    {

        // load the adjacency matrix as A
        const char *aname = files [k].name ;
        uint64_t ntriangles = files [k].ntriangles ;
        if (strlen (aname) == 0) break;
        TEST_CASE (aname) ;
        snprintf (filename, LEN, LG_DATA_DIR "%s", aname) ;
        FILE *f = fopen (filename, "r") ;
        TEST_CHECK (f != NULL) ;
        OK (LAGraph_MMRead (&A, f, msg)) ;
        OK (fclose (f)) ;
        TEST_MSG ("Loading of adjacency matrix failed") ;

        // create the graph
        OK (LAGraph_New (&G, &A, LAGRAPH_ADJACENCY_UNDIRECTED, msg)) ;
        TEST_CHECK (A == NULL) ;    // A has been moved into G->A

        // delete any diagonal entries
        OK (LAGraph_DeleteDiag (G, msg)) ;
        TEST_CHECK (G->ndiag == 0) ;
        OK (LAGraph_DeleteDiag (G, msg)) ;
        TEST_CHECK (G->ndiag == 0) ;

        // get the # of triangles
        uint64_t nt0, nt1 ;
        OK (LAGraph_TriangleCount (&nt1, G, msg)) ;
        printf ("# triangles: %g Matrix: %s\n", (double) nt1, aname) ;
        TEST_CHECK (nt1 == ntriangles) ;
        OK (LG_check_tri (&nt0, G, msg)) ;
        TEST_CHECK (nt0 == nt1) ;

        // convert to directed but with symmetric pattern
        G->kind = LAGRAPH_ADJACENCY_DIRECTED ;
        G->structure_is_symmetric = LAGRAPH_TRUE ;
        OK (LAGraph_TriangleCount (&nt1, G, msg)) ;
        TEST_CHECK (nt1 == ntriangles) ;

        OK (LG_check_tri (&nt0, G, msg)) ;
        TEST_CHECK (nt0 == nt1) ;

        // try each method
        for (int method = 1 ; method <= 6 ; method++)
        {
            for (int presort = 0 ; presort <= 2 ; presort++)
            {
                LAGraph_TriangleCount_Presort s = presort ;
                OK (LAGraph_TriangleCount_Methods (&nt1, G, method, &s, msg)) ;
                TEST_CHECK (nt1 == ntriangles) ;
            }
        }

        // invalid method
        int result = LAGraph_TriangleCount_Methods (&nt1, G, 99, NULL, msg) ;
        TEST_CHECK (result == GrB_INVALID_VALUE) ;

        OK (LAGraph_Delete (&G, msg)) ;
    }

    LAGraph_Finalize(msg);
}

//****************************************************************************
void test_TriangleCount_autosort (void)
{
    OK (LAGraph_Init(msg)) ;

    // create a banded matrix with a some dense rows/columns
    GrB_Index n = 50000 ;
    GrB_Matrix A = NULL ;
    OK (GrB_Matrix_new (&A, GrB_BOOL, n, n)) ;

    for (int k = 0 ; k <= 10 ; k++)
    {
        for (int i = 0 ; i < n ; i++)
        {
            OK (GrB_Matrix_setElement_BOOL (A, true, i, k)) ;
            OK (GrB_Matrix_setElement_BOOL (A, true, k, i)) ;
        }
    }

    // create the graph
    OK (LAGraph_New (&G, &A, LAGRAPH_ADJACENCY_UNDIRECTED, msg)) ;
    TEST_CHECK (A == NULL) ;    // A has been moved into G->A

    OK (LAGraph_DeleteDiag (G, msg)) ;
    TEST_CHECK (G->ndiag == 0) ;

    OK (LAGraph_Property_RowDegree (G, msg)) ;

    // try each method
    GrB_Index nt1 = 0 ;
    for (int method = 1 ; method <= 6 ; method++)
    {
        LAGraph_TriangleCount_Presort presort = LAGraph_TriangleCount_AutoSort ;
        nt1 = 0 ;
        OK (LAGraph_TriangleCount_Methods (&nt1, G, method, &presort, msg)) ;
        TEST_CHECK (nt1 == 2749560) ;
    }

    nt1 = 0 ;
    OK (LAGraph_TriangleCount (&nt1, G, msg)) ;
    TEST_CHECK (nt1 == 2749560) ;

    OK (LAGraph_Finalize(msg)) ;
}


#if LG_SUITESPARSE
void test_TriangleCount_brutal (void)
{
    OK (LG_brutal_setup (msg)) ;

    GrB_Matrix A = NULL ;
    printf ("\n") ;

    for (int k = 0 ; ; k++)
    {

        // load the adjacency matrix as A
        const char *aname = files [k].name ;
        uint64_t ntriangles = files [k].ntriangles ;
        if (strlen (aname) == 0) break;
        printf ("\n================== Matrix: %s\n", aname) ;
        TEST_CASE (aname) ;
        snprintf (filename, LEN, LG_DATA_DIR "%s", aname) ;
        FILE *f = fopen (filename, "r") ;
        TEST_CHECK (f != NULL) ;
        OK (LAGraph_MMRead (&A, f, msg)) ;
        OK (fclose (f)) ;
        TEST_MSG ("Loading of adjacency matrix failed") ;

        // create the graph
        OK (LAGraph_New (&G, &A, LAGRAPH_ADJACENCY_UNDIRECTED, msg)) ;

        // delete any diagonal entries
        OK (LAGraph_DeleteDiag (G, msg)) ;

        // get the # of triangles
        uint64_t nt0, nt1 ;
        LG_BRUTAL_BURBLE (LAGraph_TriangleCount (&nt1, G, msg)) ;
        printf ("# triangles: %g Matrix: %s\n", (double) nt1, aname) ;
        TEST_CHECK (nt1 == ntriangles) ;

        LG_BRUTAL_BURBLE (LG_check_tri (&nt0, G, msg)) ;
        TEST_CHECK (nt0 == nt1) ;

        // convert to directed but with symmetric pattern
        G->kind = LAGRAPH_ADJACENCY_DIRECTED ;
        G->structure_is_symmetric = LAGRAPH_TRUE ;
        LG_BRUTAL (LAGraph_TriangleCount (&nt1, G, msg)) ;
        TEST_CHECK (nt1 == ntriangles) ;

        LG_BRUTAL_BURBLE (LG_check_tri (&nt0, G, msg)) ;
        TEST_CHECK (nt0 == nt1) ;

        // try each method
        for (int method = 1 ; method <= 6 ; method++)
        {
            for (int presort = 0 ; presort <= 2 ; presort++)
            {
                LAGraph_TriangleCount_Presort s = presort ;
                LG_BRUTAL_BURBLE (LAGraph_TriangleCount_Methods (&nt1, G,
                    method, &s, msg)) ;
                TEST_CHECK (nt1 == ntriangles) ;
            }
        }

        OK (LAGraph_Delete (&G, msg)) ;
    }

    OK (LG_brutal_teardown (msg)) ;
}
#endif


//****************************************************************************
//****************************************************************************
TEST_LIST = {
    {"TriangleCount_Methods1", test_TriangleCount_Methods1},
    {"TriangleCount_Methods2", test_TriangleCount_Methods2},
    {"TriangleCount_Methods3", test_TriangleCount_Methods3},
    {"TriangleCount_Methods4", test_TriangleCount_Methods4},
    {"TriangleCount_Methods5", test_TriangleCount_Methods5},
    {"TriangleCount_Methods6", test_TriangleCount_Methods6},
    {"TriangleCount"         , test_TriangleCount},
    {"TriangleCount_many"    , test_TriangleCount_many},
    {"TriangleCount_autosort", test_TriangleCount_autosort},
    {"TriangleCount_brutal"  , test_TriangleCount_brutal},
    {NULL, NULL}
};

