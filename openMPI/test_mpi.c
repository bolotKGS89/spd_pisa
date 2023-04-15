#include <mpi.h>
#include <stdio.h>

#define ROWS 120
#define COLS 80
#define SENDER 0
#define RECEIVER 1

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

        MPI_Datatype rowType;
        MPI_Type_contiguous(ROWS, MPI_INT, &rowType);

        MPI_Datatype groupType;
        MPI_Type_vector(ROWS, 3, COLS, MPI_INT, &groupType);

        MPI_Datatype downwardType;
        MPI_Type_vector(ROWS, 1, COLS+1, MPI_INT, &downwardType);

        MPI_Datatype upwardType;
        MPI_Type_vector(ROWS, 1, COLS-1, MPI_INT, &downwardType);

        MPI_Datatype halfMatrixType;
        MPI_Type_vector(ROWS / 2, COLS / 2, COLS, MPI_INT, &halfMatrixType);

        
        int row, column;
        int matrix[ROWS][COLS];

        int i = 0;

        int token = 0;
        switch(world_rank) {
            case SENDER:
                initializeMatrix(matrix, ROWS, COLS);
                while(i <= world_size) {
                    if(token == SENDER) {
                        MPI_Send(matrix, 1, matrixType, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                        MPI_Status status;
                        MPI_Recv(matrix, 1, matrixType, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                        printf("recv matrix last element of column: %d process rank %d\n", 
                                matrix[ROWS-1][0], world_rank);
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
                        MPI_Recv(matrix, 1, matrixType, (world_rank-1+world_size)%world_size, 1, MPI_COMM_WORLD, &status);
                        increaseByOne(matrix, ROWS, COLS);
                        MPI_Send(matrix, 1, matrixType, (world_rank+1)%world_size, 1, MPI_COMM_WORLD);
                        printf("send matrix last element of column: %d process rank %d\n", 
                                matrix[ROWS-1][0], world_rank);
                        i++;
                    }
                    token = (token+1) % world_size;
                }
                break;
        }


        // switch (world_rank)
        // {
        //    case SENDER:
        //         initializeMatrix(matrix, ROWS, COLS);
        //         while(i <= 3) {
        //              // process 1
        //              MPI_Send(matrix, 1, colType, RECEIVER, 1, MPI_COMM_WORLD);
        //              MPI_Status status;
        //              MPI_Recv(matrix, 1, colType, RECEIVER, 1, MPI_COMM_WORLD, &status);
        //              printf("matrix last element of column: %d first element of column %d process rank %d\n", matrix[ROWS-1][0], matrix[0][0], world_rank);
        //              i++;
        //         }        
        //    break;
        //    case RECEIVER:
        //         initializeMatrixByZero(matrix, ROWS, COLS);
        //         while(i <= 3) {
        //              MPI_Status status;
        //              MPI_Recv(matrix, 1, colType, SENDER, 1, MPI_COMM_WORLD, &status);
        //              increaseByOne(matrix, ROWS, COLS);
        //              MPI_Send(matrix, 1, colType, SENDER, 1, MPI_COMM_WORLD);
        //              printf("send matrix last element of column: %d first element of column %d process rank %d\n", matrix[ROWS-1][0], matrix[0][0], world_rank);         
        //              i++;     
        //         }        
        //    break;
        
        // }

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