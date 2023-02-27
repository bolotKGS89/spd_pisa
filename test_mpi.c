#include <mpi.h>
#include <stdio.h>

const int PROC_ONE = 0;
const int PROC_TWO = 1;
const int ROWS = 120;
const int COLS = 80;

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);
void increaseByOne(int matrix[ROWS][COLS], int row, int col);

int main(int argc, char** argv) {

        MPI_Init(NULL, NULL);      // initialize MPI environment
        int world_size; // number of processes
        MPI_Comm_size(MPI_COMM_WORLD, &world_size);

        int world_rank; // the rank of the process
        MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

        if(world_size != 2) {
                MPI_Finalize(); 
                return 0; 
        }

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

        int i = 0, n = 3;
        double buf[1] = {0.0};


       
        if(world_rank == 0) {
                initializeMatrix(matrix, ROWS, COLS);
                while(i <= n) {
                        // process 1
                        // int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm)
                        MPI_Send(matrix, 1, colType, PROC_TWO, 1, MPI_COMM_WORLD);
                        // int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status)
                        MPI_Status status;
                        MPI_Recv(matrix, 1, colType, PROC_TWO, 1, MPI_COMM_WORLD, &status);
                        printf("recv matrix last element of column: %d first element of column %d process rank %d\n", matrix[ROWS-1][0], matrix[0][0], world_rank);
                
                i++;
                }        
        }
             

        if(world_rank == 1) {
                while(i <= n) {
                        // process 2
                        MPI_Status status;
                        MPI_Recv(matrix, 1, colType, PROC_ONE, 1, MPI_COMM_WORLD, &status);
                        increaseByOne(matrix, ROWS, COLS);
                        MPI_Send(matrix, 1, colType, PROC_ONE, 1, MPI_COMM_WORLD);
                        printf("send matrix last element of column: %d first element of column %d process rank %d\n", matrix[ROWS-1][0], matrix[0][0], world_rank);         
                        i++;     
                }        
        }
  
       

        // printf("Hello world from processor %s, rank %d out of %d processors\n",
        //         processor_name, world_rank, world_size);

        MPI_Finalize(); // finish MPI environment
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