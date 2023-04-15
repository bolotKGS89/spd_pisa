#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define SENDER 0
#define RECEIVER 1
#define ROWS 10
#define COLS 10

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);
void printMatrix(int matrix[ROWS][COLS], int row, int col);
void initializeMatrixByZero(int matrix[ROWS][COLS], int row, int col);

int main(int argc, char** argv) {
     MPI_Init(&argc, &argv);      // initialize MPI environment
    
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Datatype matrixType;
    MPI_Type_vector(ROWS, COLS, COLS, MPI_INT, &matrixType);
    MPI_Type_commit(&matrixType);

    MPI_Datatype downwardType;
    MPI_Type_vector(ROWS, 1, COLS+1, MPI_INT, &downwardType);
    MPI_Type_commit(&downwardType);

    MPI_Datatype upwardType;
    MPI_Type_vector(ROWS, 1, COLS-1, MPI_INT, &upwardType);
    MPI_Type_commit(&upwardType);


    int row, column, i = 0;
    int matrix[ROWS][COLS];

    int token = 0;
    switch(world_rank) {
        case SENDER:
            initializeMatrix(matrix, ROWS, COLS);
            while(i <= world_size) {
                if(token == SENDER) {
                    MPI_Send(matrix, 1, downwardType, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                    printf("send downwardType matrix %d process rank %d\n", matrix[0][COLS+1], world_rank);
                    MPI_Status status;
                    MPI_Recv(matrix, 1, upwardType, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                    i++;
                }
                token = (token+1) % world_size;
            }        
            break;
        case RECEIVER:
            initializeMatrixByZero(matrix, ROWS, COLS);
            while(i <= world_size) {
                if(token == world_rank) {
                    MPI_Status status;
                    MPI_Recv(matrix, 1, downwardType, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                    printf("recv downwardType matrix %d process rank %d\n", matrix[0][COLS+1], world_rank);
                    MPI_Send(matrix, 1, upwardType, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                    printf("send upwardType matrix %d process rank %d\n", matrix[ROWS-1][0], world_rank);
                    i++;
                }
                token = (token+1) % world_size;
            }
            break;
    }

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

void initializeMatrixByZero(int matrix[ROWS][COLS], int row, int col) {
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] = 0;
                }
        }
}