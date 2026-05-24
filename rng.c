#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    double phi;
    double theta;
} Point;

typedef struct {
    bool **edges;
    int *nodes_degree;
    double *distances_m;
    Point *points;
    int num_edges;
    int N;
} RNGResult;

void generate_points(Point *points, int N){
    for(int n = 0; n < N; n++){
        double u = (double)rand() / RAND_MAX;
        double phi_raw = acos(1 - 2 * u);
        points[n].phi = round(phi_raw * 10000) / 10000;

        double v = (double)rand() / RAND_MAX;
        double theta_raw = 2 * M_PI * v;
        points[n].theta = round(theta_raw * 10000) / 10000;
    }
}


double spherical_distance(Point pointA, Point pointB){
    double argument = sin(pointA.phi) * sin(pointB.phi) * cos(pointA.theta - pointB.theta) 
                    + cos(pointA.phi) * cos(pointB.phi);
    
    if (argument > 1.0) {
        argument = 1.0;
    } else if (argument < -1.0) {
        argument = -1.0;
    }

    return acos(argument);
}


RNGResult rng(Point *points, int N){
    double *distances_m = (double*)calloc(N * N, sizeof(double));

    bool *edges_flat = (bool*)calloc(N * N, sizeof(bool));
    bool **edges = (bool**)malloc(N * sizeof(bool*));
    for(int i = 0; i < N; i++) {
        edges[i] = edges_flat + (i * N);
    }

    int *nodes_degree = (int*)calloc(N, sizeof(int));

    int num_edges = 0;

    for (int i = 0; i < N; i++){
        for (int j = i + 1; j < N; j++){

            double distance_ij = spherical_distance(points[i], points[j]);
            distances_m[i * N + j] = distance_ij;
            distances_m[j * N + i] = distance_ij;

            bool is_edge = true;

            for (int k = 0; k < N; k++){
                if (k == i || k == j){
                    continue;
                }

                double distance_ik = spherical_distance(points[i], points[k]);
                double distance_jk = spherical_distance(points[j], points[k]);
            
                if (distance_ik < distance_ij && distance_jk < distance_ij){
                    is_edge = false;
                    break;
                }
                
            }


            if (is_edge){
                edges[i][j] = true;
                edges[j][i] = true;
                nodes_degree[i]++;
                nodes_degree[j]++;
                num_edges++;
            }
            
        }
    }
    RNGResult result;
    result.edges = edges;
    result.distances_m = distances_m;
    result.nodes_degree = nodes_degree;
    result.num_edges = num_edges;
    result.points = points;
    result.N = N;
    return result;
}


int main(int argc, char *argv[]){

    if(argc < 2){
        printf("Uso: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    Point *points = (Point*)malloc(N * sizeof(Point));
    
    generate_points(points, N);

    RNGResult result = rng(points, N);

    // Imprimir resultados
    printf("Número de aristas: %d\n", result.num_edges);

    printf("\nGrado de cada nodo:\n");
    for(int i = 0; i < N; i++){
        printf("  Nodo %d: %d\n", i, result.nodes_degree[i]);
    }

    printf("\nMatriz de adyacencia:\n");
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            printf("%d ", result.edges[i][j] ? 1 : 0);
        }
        printf("\n");
    }

    printf("\nDistancias (media matriz):\n");
    for(int i = 0; i < N; i++){
        for(int j = i+1; j < N; j++){
            printf("  d(%d,%d) = %.4f\n", i, j, result.distances_m[i * N + j]);
        }
    }

    printf("\nPuntos (phi, theta):\n");
    for(int i = 0; i < result.N; i++){
        printf("  Punto %d: phi=%.4f, theta=%.4f\n", i, result.points[i].phi, result.points[i].theta);
    }

    // Liberar memoria
    free(result.edges[0]);  // edges_flat
    free(result.edges);
    free(result.nodes_degree);
    free(result.distances_m);
    free(points);

    return 0;
}