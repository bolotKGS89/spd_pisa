#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define SENDER 0
#define RECEIVER 1
#define ROWS 10
#define COLS 10

#define REQUEST 2


#define K 3
#define BUFFER_SIZE 10

void initializeMatrix(int matrix[ROWS][COLS], int row, int col);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);   // initialize MPI environment
    int world_size; // number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_size != 2)
    {
        printf("This application is meant to be run with 2 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    int world_rank; // the rank of the process
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    MPI_Datatype matrixType;
    MPI_Type_vector(ROWS, COLS, COLS, MPI_INT, &matrixType);
    MPI_Type_commit(&matrixType);

    int matrix[ROWS][COLS];

    //  int send_buffer[BUFFER_SIZE], recv_buffer[BUFFER_SIZE];
    // int send_count = 0, recv_count = 0;
    // int tag = 0, count = 1;
    // MPI_Request send_requests[BUFFER_SIZE], recv_requests[BUFFER_SIZE];
    // MPI_Status statuses[BUFFER_SIZE];

    int num_sent = 0;
    int num_received = 0;
    MPI_Request send_requests[BUFFER_SIZE];
    MPI_Request recv_requests[BUFFER_SIZE];
    int buffer[BUFFER_SIZE];

    switch (world_rank)
    {
    case SENDER:
        /* code */
         for (int i = 0; i < BUFFER_SIZE; i++) {
            if (num_sent - num_received <= K) {
                MPI_Request request;
                printf("[Process %d] sent value %d from process %d\n", world_rank, i, RECEIVER);
                MPI_Isend(&i, sizeof(int), MPI_INT, RECEIVER, 0, MPI_COMM_WORLD, &request);
                send_requests[num_sent % BUFFER_SIZE] = request;
                num_sent++;
            }
            else {
                MPI_Status status;
                MPI_Wait(&send_requests[num_received % BUFFER_SIZE], &status);
                num_received++;
                // printf("received %d", num_received);
                i--;
            }
        }
        // wait for all messages to be sent
        while (num_received < num_sent) {
            MPI_Status status;
            MPI_Wait(&send_requests[num_received % BUFFER_SIZE], &status);
            num_received++;
        }
        break;
    
        case RECEIVER:
            while (num_received < BUFFER_SIZE) {
                if (num_sent - num_received < K) {
                    MPI_Request request;
                    printf("[Process %d] received value %d from process %d\n", world_rank, buffer[num_received % BUFFER_SIZE], SENDER);
                    MPI_Irecv(&buffer[num_received % BUFFER_SIZE], sizeof(int), MPI_INT, SENDER, 0, MPI_COMM_WORLD, &request);
                    recv_requests[num_received % BUFFER_SIZE] = request;
                    num_received++;
                }
                else {
                    MPI_Status status;
                    MPI_Wait(&recv_requests[num_received % BUFFER_SIZE], &status);
                    num_received++;
                }
            }
            // wait for all messages to be received
            while (num_received <= num_sent) {
                MPI_Status status;
                MPI_Wait(&recv_requests[num_received % BUFFER_SIZE], &status);
                num_received++;
            }
            // print the received messages
            // for (int i = 0; i < BUFFER_SIZE; i++) {
            //     printf("[Process %d] received value %d from process %d\n", world_rank, buffer[i], SENDER);
            // }
        break;
    }


    
    // int message = 42;
    // asynchrony degree 1
    // switch (world_rank)
    // {
    //     case SENDER:
    //     {
    //         /* code */
    //         MPI_Request send_req;
    //         MPI_Isend(&message, 1, MPI_INT, RECEIVER, REQUEST, MPI_COMM_WORLD, &send_req);
    //         int sent = 0;
    //         MPI_Test(&send_req, &sent, MPI_STATUS_IGNORE);
    //         while (!sent) {
    //             // do some work
    //             MPI_Test(&send_req, &sent, MPI_STATUS_IGNORE);
    //         }
    //         printf("message sent\n");
    //         break;
    //     }

    //     case RECEIVER:
    //     {
    //         MPI_Request recv_request;
    //         MPI_Irecv(&message, 1, MPI_INT, SENDER, REQUEST, MPI_COMM_WORLD, &recv_request);
    //         int received = 0;
    //         MPI_Test(&recv_request, &received, MPI_STATUS_IGNORE);
    //         while (!received) {
    //             // do some work
    //             MPI_Test(&recv_request, &received, MPI_STATUS_IGNORE);
    //             if(received) {
    //                 printf("Message received successfully by rank 1: %d\n", message);
    //             }
    //         }
            
    //         break;
    //     }
    // }

 

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


//    int rank, size, tag = 1, count = 1;
//     char send_buffer[BUFFER_SIZE];
//     char recv_buffer[BUFFER_SIZE];
//     MPI_Request send_request, recv_request;
//     MPI_Status status;

//     MPI_Init(&argc, &argv);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);

//     if (rank == 0) {
//         sprintf(send_buffer, "Hello");
//         MPI_Isend(send_buffer, BUFFER_SIZE, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &send_request);
//         MPI_Wait(&send_request, &status);
//         printf("Process %d sent message '%s' to process %d\n", rank, send_buffer, 1);

//         MPI_Irecv(recv_buffer, BUFFER_SIZE, MPI_CHAR, 1, tag, MPI_COMM_WORLD, &recv_request);
//         MPI_Wait(&recv_request, &status);
//         printf("Process %d received message '%s' from process %d\n", rank, recv_buffer, 1);
//     } else {
//         MPI_Irecv(recv_buffer, BUFFER_SIZE, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &recv_request);
//         MPI_Wait(&recv_request, &status);
//         printf("Process %d received message '%s' from process %d\n", rank, recv_buffer, 0);

//         sprintf(send_buffer, "Hi");
//         MPI_Isend(send_buffer, BUFFER_SIZE, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &send_request);
//         MPI_Wait(&send_request, &status);
//         printf("Process %d sent message '%s' to process %d\n", rank, send_buffer, 0);
//     }

//     MPI_Finalize();
