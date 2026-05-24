#ifndef MYVAR
#define MYVAR
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

typedef struct {
    double phi;
    double theta;
} MPI_Point;

typedef struct {
    int    i;         //Node i of the pair to evaluate
    int    j;          //Node j of the pair to evaluate
    int    N;          //Total size of the point cloud
    int    is_edge;    //Result: 1=edge, 0=no edge
    MPI_Point points[1]; //Actual size: N — sent as a block of bytes
} MPI_myvar;

#ifdef __cplusplus
}
#endif
#endif