#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <stdbool.h>
#include "mandelbrot.h"
#include "connect.h"

#define TAG_JOB 1
#define TAG_RESULT 2
#define TAG_END_STREAM 3
#define EMITTER 0
#define COLLECTOR 9
#define WORKERS_NUM 10


void emitter(int num_workers, DistributionPolicy distributionPolicy) {
    MPI_Status status;
    int next_worker = 1;
    TaskData task;
    double start_time = MPI_Wtime();
    double timeout_seconds = 1.5; // set timeout to 1.5 seconds

    while(true) {
        task = assignRandomData();
        // printf("[Process Emitter] I send value C_x %.2f  C_y %.2f iMax %d ER2 %.2f to process %d.\n", task.C_x, task.C_y, task.iMax, task._ER2, next_worker);

        if (MPI_Wtime() - start_time > timeout_seconds) {
            printf("Timeout expired, terminating loop.\n");
            break;
        }

        if (distributionPolicy == ROUND_ROBIN) {
            MPI_Rsend(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD);
            next_worker = (next_worker % num_workers) + 1;
        } else if (distributionPolicy == EXPLICIT_REQUEST) {
            MPI_Request request;
            MPI_Isend(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD, &request);
            int completed = 0;
            while (!completed) {
                MPI_Test(&request, &completed, &status);
            }

            next_worker = (next_worker % num_workers) + 1;
        } else if (distributionPolicy == BUFFER) {
            MPI_Request request;
            MPI_Isend(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD, &request);
        }

    }

    
    for (int i = 1; i < num_workers; i++) {
        printf("send end");
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_END_STREAM, MPI_COMM_WORLD);
    }
 
}

/* define your worker process */
void worker(int rank, DistributionPolicy distributionPolicy) {
    MPI_Status status;
    MPI_Request request;
    TaskData task;
    ResultData result;

    if (distributionPolicy == ROUND_ROBIN) {
        while (1) {
            MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end worker\n");
                MPI_Send(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD);
                break;
            } 
            compute(&task, &result);
        
                    
            MPI_Rsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
        } /* round-robin */
    } else if (distributionPolicy == EXPLICIT_REQUEST) {
        while (1) {
            MPI_Irecv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            if (status.MPI_TAG == TAG_END_STREAM) {
                MPI_Isend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD, &request);
                printf("end worker\n");
                break;
            }

            compute(&task, &result);

            MPI_Isend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
        } /* explicit task request */

    } else if (distributionPolicy == BUFFER) {
        MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, TAG_JOB, MPI_COMM_WORLD, &status);
        int i = 0, k = 4;
        int buffer_size = 0;
        while(i <= k) {
            buffer_size += (MPI_BSEND_OVERHEAD + sizeof(int));
            i++;
        }
        int *buffer = malloc(buffer_size);
        
        // printf("buffer size: %d\n", buffer_size);
        // printf("Size of an MPI_Bsend overhead: %d bytes.\n", MPI_BSEND_OVERHEAD);
        MPI_Buffer_attach(buffer, buffer_size);

        i = 0;
        while(i <= k) {
            compute(&task, &result);
            MPI_Bsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
            i++;
        } /* buffer */

    }

    printf("[Process Worker %d] I received and computed value C_x %.2f C_y %.2f iMax %d ER2 %.2f result %d from process %d.\n",
        rank, result.C_x, result.C_y, result.iMax, result._ER2, result.res, EMITTER);
}

/* define your collector process */
void collector(int num_workers, DistributionPolicy distributionPolicy) {
    MPI_Status status;
    TaskData task;
    ResultData result;
    MPI_Request request;
    int next_worker = 1;


    if (distributionPolicy == ROUND_ROBIN) { 
        while(1) {    
            MPI_Recv(&result, sizeof(ResultData), MPI_BYTE, next_worker, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end collector\n");
                break;
            }
            next_worker = (next_worker % (num_workers - 1)) + 1;
        } /* round-robin */
    } else if (distributionPolicy == EXPLICIT_REQUEST) {
        while(true) {
            MPI_Irecv(&result, sizeof(ResultData), MPI_BYTE, next_worker, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end collector\n");
                break;
            }
            next_worker = (next_worker % (num_workers - 1)) + 1;
        } /* explicit task request */
    } else if (distributionPolicy == BUFFER) { 
        for(int i = 1; i < num_workers; i++) {
            MPI_Irecv(&result, sizeof(ResultData), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);


        } /* buffer */
    }  

    printf("[Process Collecter %d] I received value %.2f %.2f %d %.2f %d from process %d.\n", COLLECTOR, 
    result.C_x, result.C_y, result.iMax, result._ER2, result.res, next_worker);  
}



int main(int argc, char** argv) {
    int num_workers, world_rank, num_jobs;  
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_workers);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(num_workers != WORKERS_NUM)
    {
        printf("This application is meant to be run with 10 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }


    switch (world_rank)
    {
        case EMITTER:
            emitter(8, BUFFER);
        break;
        case COLLECTOR:
            collector(COLLECTOR, BUFFER);
        break;
        default:
            worker(world_rank, BUFFER);
        break;
    }
    
    MPI_Finalize();
    
    return 0;
}