#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

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

    // switch(world_rank)
    // {
    //     case SENDER:
    //     {
    //         int buffer_sent = 12345;
    //         MPI_Request request;
    //         printf("MPI process %d sends value %d.\n", world_rank, buffer_sent);
    //         MPI_Isend(&buffer_sent, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
            
    //         // Do other things while the MPI_Isend completes
    //         // <...>
 
    //         // Let's wait for the MPI_Isend to complete before progressing further.
    //         MPI_Wait(&request, MPI_STATUS_IGNORE);
    //         break;
    //     }
    //     case RECEIVER:
    //     {
    //         int received;
    //         MPI_Recv(&received, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    //         printf("MPI process %d received value: %d.\n", world_rank, received);
    //         break;
    //     }
    // }

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
            char *buffer = malloc(buffer_size);
            
            printf("%d\n", buffer_size);
            printf("Size of an MPI_Bsend overhead: %d bytes.\n", MPI_BSEND_OVERHEAD);
            MPI_Buffer_attach(buffer, buffer_size);

            while(i <= k) {
                int msg = rand() % 100 + 1;
                printf("[Process %d] I send value %d to process %d.\n", world_rank, msg, RECEIVER);
                MPI_Bsend(&msg, 1, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD);
                i++;
            }

            // int msg = rand() % 100 + 1;
            // printf("[Process %d] I send value %d to process %d.\n", world_rank, msg, RECEIVER);
            // MPI_Bsend(&msg, 1, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD);

            MPI_Buffer_detach(&buffer, &buffer_size);
            free(buffer);


            // while(i <= n) {
            //     int msg = rand() % 100 + 1;
            //     if(k <= i) {
            //         int buffer_size = (MPI_BSEND_OVERHEAD + sizeof(int));
            //         char *buffer = malloc(buffer_size);
            //         printf("Size of an MPI_Bsend overhead: %d bytes.\n", MPI_BSEND_OVERHEAD);
            //         MPI_Buffer_attach(buffer, buffer_size);

            //         printf("[Process %d] I send value %d to process %d.\n", world_rank, msg, RECEIVER);
            //         MPI_Bsend(&msg, 1, MPI_INT, RECEIVER, 0, MPI_COMM_WORLD);
                    
            //         MPI_Buffer_detach(&buffer, &buffer_size);
            //         free(buffer);
            //     }
                // else {
                //     MPI_Request request;
                //     printf("MPI process %d sends value %d.\n", world_rank, msg);
                //     MPI_Isend(&msg, 1, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
                //     MPI_Status myStatus;
                //     MPI_Wait(&request, &myStatus);
                // }
                // i++;
            // }

            break;
        }
        case RECEIVER:
        {
            int buffer;
            MPI_Request request;
            while(i <= k) {
                printf("MPI process %d received value: %d.\n", world_rank, buffer);
                MPI_Irecv(&buffer, 1, MPI_INT, SENDER, 0, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, MPI_STATUS_IGNORE);
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
