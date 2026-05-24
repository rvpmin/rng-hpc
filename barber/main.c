#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>
#include "myvar.h"
#include "Timming.h"

//Points structure

typedef struct {
    double phi;
    double theta;
} Point;

//Create random points on the unit sphere with normal distribution
void generate_points(Point *points, int N) {
    srand(42);
    for (int n = 0; n < N; n++) {
        double u        = (double)rand() / RAND_MAX;
        double phi_raw  = acos(1.0 - 2.0 * u);
        points[n].phi   = round(phi_raw  * 10000.0) / 10000.0;

        double v        = (double)rand() / RAND_MAX;
        double theta_raw = 2.0 * M_PI * v;
        points[n].theta = round(theta_raw * 10000.0) / 10000.0;
    }
}


//Calculate spherical distance
double spherical_distance(Point a, Point b) {
    double arg = sin(a.phi) * sin(b.phi) * cos(a.theta - b.theta)
               + cos(a.phi) * cos(b.phi);
    if (arg >  1.0) arg =  1.0;
    if (arg < -1.0) arg = -1.0;
    return acos(arg);
}

//Message
static int msg_size(int N) {
    return (int)(sizeof(MPI_myvar) + (N - 1) * sizeof(MPI_Point));
}


static MPI_myvar *alloc_msg(int N) {
    MPI_myvar *m = (MPI_myvar *)malloc(msg_size(N));
    memset(m, 0, msg_size(N));
    return m;
}


int main(int argn, char **argc) {

    int miproc, numproc;
    MPI_Status  status;
    MPI_Request request;

    double utime0, stime0, wtime0,
           utime2, stime2, wtime2;

    MPI_Init(&argn, &argc);
    MPI_Comm_rank(MPI_COMM_WORLD, &miproc);
    MPI_Comm_size(MPI_COMM_WORLD, &numproc);
    MPI_Barrier(MPI_COMM_WORLD);


    int N = 20;
    if (miproc == 0) {
        if (argn >= 2) N = atoi(argc[1]);
        printf("N = %d  procesos = %d\n", N, numproc);
        uswtime(&utime2, &stime2, &wtime2);   
    }
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);

    //Keep the same points throughout all processes
    Point *points = (Point *)malloc(N * sizeof(Point));
    generate_points(points, N);

    //Precalculate distances
    double *dist = (double *)calloc(N * N, sizeof(double));
    for (int i = 0; i < N; i++)
        for (int j = i + 1; j < N; j++) {
            double d = spherical_distance(points[i], points[j]);
            dist[i * N + j] = d;
            dist[j * N + i] = d;
        }

    long total_pairs = (long)N * (N - 1) / 2;

    //Slaves
    if (miproc != 0) {

        MPI_myvar *msg = alloc_msg(N);
        msg->N        = N;
        msg->i        = -1;   
        msg->j        = -1;
        msg->is_edge  = 0;

        //Copy points to message
        for (int k = 0; k < N; k++) {
            msg->points[k].phi   = points[k].phi;
            msg->points[k].theta = points[k].theta;
        }

        while (1) {
            //Send result to master
            MPI_Send(msg, msg_size(N), MPI_CHARACTER,
                     0, 0, MPI_COMM_WORLD);

            //Receive next pair to evaluate
            MPI_Recv(msg, msg_size(N), MPI_CHARACTER,
                     0, 0, MPI_COMM_WORLD, &status);

            int i = msg->i;
            int j = msg->j;

            //Stop signal: i = j = -2
            if (i == -2) break;

            double d_ij  = dist[i * N + j];
            bool is_edge = true;

            for (int k = 0; k < N && is_edge; k++) {
                if (k == i || k == j) continue;
                if (dist[i * N + k] < d_ij &&
                    dist[j * N + k] < d_ij)
                    is_edge = false;
            }

            msg->is_edge = is_edge ? 1 : 0;
        }

        free(msg);

    //Master
    } else {

        //Matrixes
        int *edges_global = (int *)calloc(N * N, sizeof(int));
        int  num_edges    = 0;

        MPI_myvar *msg = alloc_msg(N);
        int flag = -1;


        long next_pair = 0;

        
        #define PAIR_TO_IJ(p, N, pi, pj) do {                        \
            double _d = (2.0*(N)-1.0);                                \
            int _i = (int)((_d - sqrt(_d*_d - 8.0*(p))) / 2.0);      \
            while ((long)_i*(2*(N)-1-_i)/2 > (p)) _i--;              \
            while ((long)(_i+1)*(2*(N)-1-(_i+1))/2 <= (p)) _i++;     \
            long _base = (long)_i*(2*(N)-1-_i)/2;                     \
            *(pi) = _i;  *(pj) = (int)((p)-_base)+_i+1;              \
        } while(0)

        
        long msgs_to_recv = (numproc - 1) + total_pairs;
        long msgs_recvd   = 0;

        while (1) {

            
            if (flag != 0) {
                MPI_Irecv(msg, msg_size(N), MPI_CHARACTER,
                          MPI_ANY_SOURCE, MPI_ANY_TAG,
                          MPI_COMM_WORLD, &request);
                flag = 0;
            }

            MPI_Test(&request, &flag, &status);

            if (flag != 0) {
                int src = status.MPI_SOURCE;

        
                int ri = msg->i, rj = msg->j;
                if (ri >= 0 && rj >= 0 && msg->is_edge) {
                    edges_global[ri * N + rj] = 1;
                    edges_global[rj * N + ri] = 1;
                    num_edges++;
                }
                msgs_recvd++;


                if (next_pair < total_pairs) {
                    int ni, nj;
                    PAIR_TO_IJ(next_pair, N, &ni, &nj);
                    msg->i  = ni;
                    msg->j  = nj;
                    msg->is_edge = 0;
                    next_pair++;
                } else {

                    msg->i = -2;
                    msg->j = -2;
                }

                MPI_Send(msg, msg_size(N), MPI_CHARACTER,
                         src, 0, MPI_COMM_WORLD);

                flag = -1;
            }


            if (msgs_recvd == msgs_to_recv) break;

        } 

        printf("\nAristas totales : %d\n", num_edges);
        printf("Grado promedio  : %.4f\n",
               (double)(2 * num_edges) / N);
        printf("\nGrado por nodo:\n");
        for (int i = 0; i < N; i++) {
            int deg = 0;
            for (int j = 0; j < N; j++)
                deg += edges_global[i * N + j];
            printf("  nodo %3d → grado %d\n", i, deg);
        }

        //Time
        uswtime(&utime0, &stime0, &wtime0);
        printf("\nBenchmarks (sec):\n");
        printf("real %.6f\n", wtime0 - wtime2);
        printf("user %.6f\n", utime0 - utime2);
        printf("sys  %.6f\n", stime0 - stime2);
        printf("\nCPU/Wall %.3f %%\n",
               100.0 * (utime0 - utime2 + stime0 - stime2)
                     / (wtime0 - wtime2));

        //Free memory
        free(edges_global);

    } 

    free(dist);
    free(points);

    MPI_Finalize();
    return 0;
}
