#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>


const int PROC_ONE = 0;
const int PROC_TWO = 1;
const int ROWS = 10;
const int COLS = 5;

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);
void printMatrix(int matrix[ROWS][COLS], int row, int col);

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(world_size != 2)
    {
        printf("This application is meant to be run with 2 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    MPI_Datatype matrixType;
    MPI_Type_vector(ROWS, COLS, COLS, MPI_INT, &matrixType);
    MPI_Type_commit(&matrixType);

    MPI_Datatype downwardType;
    MPI_Type_vector(ROWS, 1, COLS+1, MPI_INT, &downwardType);
    MPI_Type_commit(&downwardType);

    MPI_Datatype upwardType;
    MPI_Type_vector(ROWS, 1, COLS-1, MPI_INT, &upwardType);
    MPI_Type_commit(&upwardType);


    int row, column;
    int matrix[ROWS][COLS];

    if(world_rank == 0) {
        initializeMatrix(matrix, ROWS, COLS);
        MPI_Send(matrix, 1, downwardType, PROC_TWO, 1, MPI_COMM_WORLD);
        printf("send matrix %d process rank %d\n", matrix[0][COLS-1], world_rank);
        MPI_Status status;
        MPI_Recv(matrix, 1, upwardType, PROC_TWO, 1, MPI_COMM_WORLD, &status);
    } else if(world_rank == 1) {
        MPI_Status status;
        MPI_Recv(matrix, 1, downwardType, PROC_ONE, 1, MPI_COMM_WORLD, &status);
        MPI_Send(matrix, 1, upwardType, PROC_ONE, 1, MPI_COMM_WORLD);
        printf("send matrix %d process rank %d\n", matrix[ROWS-1][0], world_rank);
        printf("send matrix %d process rank %d\n", matrix[0][COLS-1], world_rank);
    }

    MPI_Type_free(&matrixType);

    MPI_Finalize(); // finish MPI environment
}


void initializeMatrix(int matrix[ROWS][COLS], int row, int col) {
        srand(100);
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] = rand() % 100 + 1;
                }
        }
}

void printMatrix(int matrix[ROWS][COLS], int row, int col) {
        for(int i = 0; i < row; i++) {
            for(int j = 0; j < col; j++) {
                printf("%d\t", matrix[i][j]);
            }
            printf("\n");
        }
}