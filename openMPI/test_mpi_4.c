#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

const int PROC_ONE = 0;
const int PROC_TWO = 1;
const int ROWS = 10;
const int COLS = 5;

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);

int main(int argc, char** argv) {
    MPI_Init(NULL, NULL);      // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_size != 2)
    {
        printf("This application is meant to be run with 2 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    enum role_ranks { SENDER, RECEIVER };
    int world_rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Datatype matrixType;
    MPI_Type_vector(ROWS, COLS, COLS, MPI_INT, &matrixType);
    MPI_Type_commit(&matrixType);

    int matrix[ROWS][COLS];
    int i = 0, k = 4, n = 6;
    switch(world_rank)
    {
        case SENDER:
        {
            initializeMatrix(matrix, ROWS, COLS);

            int buffer_size = 0;
            while(i <= k) {
                buffer_size += (MPI_BSEND_OVERHEAD + sizeof(int));
                i++;
            }
            int *buffer = malloc(buffer_size);
            
            printf("buffer size: %d\n", buffer_size);
            printf("Size of an MPI_Bsend overhead: %d bytes.\n", MPI_BSEND_OVERHEAD);
            MPI_Buffer_attach(buffer, buffer_size);
            
            srand(time(NULL)); // Seed the random number generator with the current time
            i = 0;
            while(i <= k) {
                int msg = rand() % 100 + 1;
                printf("[Process %d] I send value %d to process %d.\n", world_rank, matrix[ROWS][COLS], RECEIVER);
                MPI_Bsend(&matrix[i][0], COLS, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD);
                i++;
            }

            MPI_Buffer_detach(&buffer, &buffer_size);
            free(buffer);

            break;
        }
        case RECEIVER:
        {
            int buffer;
            MPI_Request request;
            i = 0;
            while(i <= k) {
                MPI_Irecv(&matrix[i][0], COLS, MPI_INT, SENDER, 0, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, MPI_STATUS_IGNORE);
                printf("MPI process %d received value: %d.\n", world_rank, matrix[ROWS][COLS]);
                i++;
            } 
        }
    }
 
    MPI_Finalize(); // finish MPI environment

    return EXIT_SUCCESS;
}

void initializeMatrix(int matrix[ROWS][COLS], int row, int col) {
        srand(100);
        for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                        matrix[i][j] = rand() % 100 + 1;
                }
        }
}

void send_async1(void* data, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    MPI_Request req;
    MPI_Isend(data, count, datatype, dest, tag, comm, &req);
    MPI_Request_free(&req); // complete the send operation immediately
}

void recv_async1(void* data, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm) {
    MPI_Request req;
    MPI_Irecv(data, count, datatype, source, tag, comm, &req);
    MPI_Request_free(&req); // complete the receive operation immediately
}


void send_async_k(void* data, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm) {
    MPI_Request req;
    MPI_Isend(data, count, datatype, dest, tag, comm, &req);
}

void recv_async_k(void* data, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, int k) {
    MPI_Request req;
    MPI_Status status;
    char buffer[k];
    int flag = 0;
    while (!flag) {
        MPI_Iprobe(source, tag, comm, &flag, &status);
        if (flag) {
            MPI_Recv(data, count, datatype, source, tag, comm, &status);
            break;
        }
        MPI_Wait(&req, &status);
        MPI_Sendrecv(buffer, k, MPI_CHAR, source, tag, buffer, k, MPI_CHAR, source, tag, comm, &status);
    }
}
