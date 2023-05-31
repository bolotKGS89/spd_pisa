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
#define TAG_REQUEST 4
#define EMITTER 0
#define COLLECTOR 9
#define WORKERS_NUM 10
#define BUFFER_SIZE 2048

/* define your emitter process */
void emitter(int num_workers, DistributionPolicy distributionPolicy, MPI_Request request) {
    MPI_Status status;
    int num_sent = 0;
    TaskData task;
    // printf("[Process Emitter] I send value C_x %.2f  C_y %.2f iMax %d ER2 %.2f to process %d.\n", task.C_x, task.C_y, task.iMax, task._ER2, next_worker);


    if (distributionPolicy == ROUND_ROBIN) {
        int next_worker = 1;
        double start_time = MPI_Wtime();
        double timeout_seconds = 1.5; // set timeout to 1.5 seconds
        while(true) {
            task = assignRandomData();
            if (MPI_Wtime() - start_time > timeout_seconds) {
                printf("Timeout expired, terminating loop.\n");
                break;
            }

            MPI_Ssend(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD);
            next_worker = (next_worker % num_workers) + 1;
        } 
    } else if (distributionPolicy == EXPLICIT_REQUEST) {
        MPI_Status status;
        int num_sent_req = 0;
        int source = 0; 
        while(true) {
            MPI_Irecv(NULL, 0, MPI_C_BOOL, MPI_ANY_SOURCE, TAG_REQUEST, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            if(status.MPI_TAG == TAG_REQUEST) {
                task = assignRandomData();
                source = status.MPI_SOURCE;
                int sendRes = MPI_Isend(&task, sizeof(TaskData), MPI_BYTE, source, TAG_JOB, MPI_COMM_WORLD, &request);
                if (sendRes == MPI_SUCCESS) {
                    // printf("[Process Emitter %d] sent to %d with status %d a value %d\n", EMITTER, status.MPI_SOURCE, status.MPI_TAG, isEmitterFree);
                    MPI_Wait(&request, &status);
                    num_sent_req++;

                    if(WORKERS_NUM - 2 == num_sent_req) {
                        break;
                    } 
                }
            }
        }           
    } else if (distributionPolicy == BUFFER) {
        int num_sent = 0;
        TaskData data;

        char* buffer = (char*)malloc(BUFFER_SIZE);
        MPI_Buffer_attach(buffer, BUFFER_SIZE);
        int buffer_size = BUFFER_SIZE;

        while (num_sent < num_workers) {
            data = assignRandomData();
           
            // Send the task data to the worker processes
            MPI_Bsend(&data, sizeof(TaskData), MPI_BYTE, num_sent, TAG_JOB, MPI_COMM_WORLD);
            num_sent++;
        }
        // num_sent = 0;

        // while (num_sent < num_workers) {
        //      printf("[Process Emitter %d] sent to %d with status %d\n", EMITTER, num_sent, TAG_END_STREAM);
        //     // Send null to the worker processes
        //     MPI_Bsend(&data, sizeof(TaskData), MPI_BYTE, num_sent, TAG_END_STREAM, MPI_COMM_WORLD);
        //     num_sent++;
        // }

        // Detach the buffer from the communicator
        MPI_Buffer_detach(&buffer, &buffer_size);
    }


    for (int i = 1; i < WORKERS_NUM - 2; i++) {
        printf("done");
        MPI_Isend(NULL, 0, MPI_BYTE, i, TAG_END_STREAM, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);
    }
 
}

/* define your worker process */
void worker(int rank, DistributionPolicy distributionPolicy, MPI_Request request) {
    MPI_Status status;
    TaskData task;
    ResultData result;

    if (distributionPolicy == ROUND_ROBIN) {
        while (1) {
            MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end worker\n");
                MPI_Ssend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD);
                break;
            } 
            compute(&task, &result);
        
                    
            MPI_Ssend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
        } /* round-robin */
    } else if (distributionPolicy == EXPLICIT_REQUEST) {
        int num_sent = 0;
        while (true) {
            int sendRes = MPI_Isend(NULL, 0, MPI_C_BOOL, EMITTER, TAG_REQUEST, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            if(sendRes == MPI_SUCCESS) {
                
                MPI_Irecv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
                MPI_Wait(&request, &status);

                if(status.MPI_TAG == TAG_JOB) {
                    compute(&task, &result);
                    // printf("[Process Worker %d] received from %d with status %d a value %d\n", rank, EMITTER, status.MPI_TAG, result.iMax);

                    MPI_Isend(&task, sizeof(TaskData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);               
                } else if(status.MPI_TAG == TAG_END_STREAM) {
                    // printf("[Process Worker %d] received from %d with status %d a value %d\n", rank, EMITTER, status.MPI_TAG);
                    MPI_Isend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD, &request);
                    MPI_Wait(&request, &status);
                    break;
                }
            }
        } /* explicit task request */

    } else if (distributionPolicy == BUFFER) {
        char buffer[BUFFER_SIZE];
        MPI_Buffer_attach(buffer, BUFFER_SIZE);
        int buffer_size = BUFFER_SIZE;

        while (1) {
            // Receive the task data from the emitter or collector process
            MPI_Irecv(&task, buffer_size, MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            int tag = status.MPI_TAG;

            if (tag == TAG_JOB) {
                // Process the task
                compute(&task, &result);
                
                // Send the result back to the collector process
                MPI_Bsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_RESULT, MPI_COMM_WORLD);
            }
            else if (tag == TAG_END_STREAM) {
                // printf("[Process Worker %d] received from %d with status %d a value %d\n", rank, EMITTER, status.MPI_TAG, result.iMax);
                MPI_Bsend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD);
            }
        }


        MPI_Buffer_detach(&buffer, &buffer_size);
    
    }

    

    // printf("[Process Worker %d] I received and computed value C_x %.2f C_y %.2f iMax %d ER2 %.2f result %d from process %d.\n",
    //     rank, result.C_x, result.C_y, result.iMax, result._ER2, result.res, EMITTER);
}

/* define your collector process */
void collector(int num_workers, DistributionPolicy distributionPolicy, MPI_Request request) {
    MPI_Status status;
    TaskData task;
    ResultData result;

    if (distributionPolicy == ROUND_ROBIN) { 
        int next_worker = 1;
          while(1) {    
            MPI_Recv(&result, sizeof(ResultData), MPI_BYTE, next_worker, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end collector\n");
                
                break;
            }
            next_worker = (next_worker % (num_workers - 1)) + 1;
        } /* round-robin */
    } else if (distributionPolicy == EXPLICIT_REQUEST) {
        while (true) {
            MPI_Irecv(&result, sizeof(ResultData), MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);
            
            if (status.MPI_TAG == TAG_END_STREAM) {
                printf("end collector\n");
                printf("[Process Collector %d] received from %d with status %d a value %d\n", COLLECTOR, status.MPI_SOURCE, status.MPI_TAG, result.iMax);
                break;
            }
        }
    } else if (distributionPolicy == BUFFER) { 
        int num_received = 0;
        // Attach a buffer to the communicator
        char* buffer = (char*)malloc(BUFFER_SIZE);
        MPI_Buffer_attach(buffer, BUFFER_SIZE);
        int buffer_size = BUFFER_SIZE;

        while (true) {
            // Receive the result from a worker process
           
            MPI_Irecv(&result, buffer_size, MPI_BYTE, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            // Check end-of-stream condition
            int source = status.MPI_SOURCE;
            int tag = status.MPI_TAG;

            if (tag == TAG_END_STREAM) {
                // printf("end collector\n");
                printf("[Process Collector %d] received from %d with status %d a value %d\n", COLLECTOR, status.MPI_SOURCE, status.MPI_TAG, result.iMax);
                break;
            }
        }

        // Detach the buffer from the communicator
        MPI_Buffer_detach(&buffer, &buffer_size);
    }     

    // printf("[Process Collecter %d] I received value %.2f %.2f %d %.2f %d from process %d.\n", COLLECTOR, 
    // result.C_x, result.C_y, result.iMax, result._ER2, result.res, next_worker);  
}


int main(int argc, char** argv) {
    int num_workers, world_rank, num_jobs;  
    
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &num_workers);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

   MPI_Request request;

    switch (world_rank)
    {
        case EMITTER:
            emitter(8, EXPLICIT_REQUEST, request);
        break;
        case COLLECTOR:
            collector(COLLECTOR, EXPLICIT_REQUEST, request);
        break;
        default:
            worker(world_rank, EXPLICIT_REQUEST, request);
        break;
    }

     MPI_Finalize();

    return 0;
}