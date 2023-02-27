#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

const int PROC_ONE = 0;
const int PROC_TWO = 1;
const int ROWS = 10;
const int COLS = 10;

void initializeMatrix(int matrix[ROWS][0], int col);
void increaseByOne(int matrix[ROWS][0], int col);

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


    MPI_Datatype colType;
    MPI_Type_vector(ROWS, 1, COLS, MPI_INT, &colType);
    MPI_Type_commit(&colType);

    int num_cols = 10; // number of columns in the matrix
    int row_len = num_cols * sizeof(double); // length of a row in bytes

    MPI_Datatype row_type;
    MPI_Type_vector(1, num_cols, 1, MPI_INT, &row_type);
    MPI_Type_commit(&row_type);

    int n = 6; // matrix size
    int blocksize = 1; // number of elements in each block
    int stride = 3; // distance between start of each block
    MPI_Datatype column_type;

    MPI_Type_vector(n, blocksize, stride, MPI_INT, &column_type);
    MPI_Type_commit(&column_type);

    int n = 6;
    MPI_Datatype up_diag_type;    
    MPI_Type_vector(n, 1, n+1, MPI_DOUBLE, &up_diag_type);
    MPI_Type_commit(&up_diag_type);

    int n = 6;
    MPI_Datatype down_diag_type;   
    MPI_Type_vector(n, 1, n-1, MPI_DOUBLE, &down_diag_type);
    MPI_Type_commit(&down_diag_type);

    int i = 0, n = 3;
    int matrix[ROWS][0];

    if(world_rank == 0) {
        initializeMatrix(matrix, COLS);
        while(i <= n) {
            // process 1
            // int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
            MPI_Send(matrix, 1, colType, PROC_TWO, 1, MPI_COMM_WORLD);
            // int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
            MPI_Status status;
            MPI_Recv(matrix, 1, colType, PROC_TWO, 1, MPI_COMM_WORLD, &status);
            printf("recv matrix last element: %d process rank %d\n", matrix[0][COLS-1], world_rank);
            i++;
        }        
    } else if(world_rank == 1) {
        while(i <= n) {
            // process 2
            MPI_Status status;
            MPI_Recv(matrix, 1, colType, PROC_ONE, 1, MPI_COMM_WORLD, &status);
            increaseByOne(matrix, COLS);
            MPI_Send(matrix, 1, colType, PROC_ONE, 1, MPI_COMM_WORLD);
            printf("send matrix last element: %d process rank %d\n", matrix[0][COLS-1], world_rank);           
            i++;     
        }        
    }

    


    MPI_Finalize(); // finish MPI environment
}

void initializeMatrix(int matrix[ROWS][0], int col) {
    for(int j = 0; j < col; j++) {
            matrix[j][0] = 0 + (j * 10);
    }
}

void increaseByOne(int matrix[ROWS][0], int col) {
    for(int j = 0; j < col; j++) {
            matrix[j][0] *= 2;
    }
}
        