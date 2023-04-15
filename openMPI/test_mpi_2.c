#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define SENDER 0
#define RECEIVER 1
#define ROWS 10
#define COLS 10

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);
void increaseByOne(int matrix[ROWS][COLS], int row, int col);
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

    MPI_Datatype colType;
    MPI_Type_vector(ROWS, 1, COLS, MPI_INT, &colType);
    MPI_Type_commit(&colType);

    MPI_Datatype row_type;
    MPI_Type_vector(1, ROWS, 1, MPI_INT, &row_type);
    MPI_Type_commit(&row_type);

    int n = 6; // matrix size
    int blocksize = 1; // number of elements in each block
    int stride = 3; // distance between start of each block
    MPI_Datatype column_type;

    MPI_Type_vector(n, blocksize, stride, MPI_INT, &column_type);
    MPI_Type_commit(&column_type);

    MPI_Datatype up_diag_type;    
    MPI_Type_vector(n, 1, n+1, MPI_DOUBLE, &up_diag_type);
    MPI_Type_commit(&up_diag_type);

    MPI_Datatype down_diag_type;   
    MPI_Type_vector(n, 1, n-1, MPI_DOUBLE, &down_diag_type);
    MPI_Type_commit(&down_diag_type);

    int i = 0;
    int matrix[ROWS][COLS];

    int token = 0;
    switch(world_rank) {
        case SENDER:
            initializeMatrix(matrix, ROWS, COLS);
            while(i <= world_size) {
                if(token == SENDER) {
                    MPI_Send(matrix, 1, column_type, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                    MPI_Status status;
                    MPI_Recv(matrix, 1, column_type, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                    printf("recv matrix last element of column: %d process rank %d\n", 
                            matrix[ROWS-1][0], world_rank);
                    i++;
                }
                token = (token+1) % world_size;
            }        
            break;
        case RECEIVER:
            // initializeMatrixByZero(matrix, ROWS, COLS);
            while(i <= world_size) {
                if(token == world_rank) {
                    MPI_Status status;
                    MPI_Recv(matrix, 1, column_type, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                    increaseByOne(matrix, ROWS, COLS);
                    MPI_Send(matrix, 1, column_type, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                    printf("send matrix last element of column: %d process rank %d\n", 
                            matrix[ROWS-1][0], world_rank);
                    i++;
                }
                token = (token+1) % world_size;
            }
            break;
    }

    MPI_Finalize(); // finish MPI environment
}

void initializeMatrixByZero(int matrix[ROWS][COLS], int row, int col) {
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] = 0;
                }
        }
}


void initializeMatrix(int matrix[ROWS][COLS], int row, int col) {
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] = i + (j * 10);
                }
        }
}

void increaseByOne(int matrix[ROWS][COLS], int row, int col) {
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] *= 2;
                }
        }
}