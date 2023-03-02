#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define TAG_JOB 1
#define TAG_RESULT 2
#define EMITTER 0
#define COLLECTOR 9
#define TAG_END_STREAM 3

typedef struct {
    int x;
    int y;
} TaskData;

typedef struct {
    double x;
    double y;
} ResultData;

/* define your computing function here */
void compute(TaskData *task, ResultData *result) {
    /* implement your computation logic here */
    result->y = (double)task->x;
    result->x = (double)task->y;
}

/* define your emitter process */
void emitter(int num_workers) {
    MPI_Status status;
    int next_worker = 1;
    TaskData task;
    double start_time = MPI_Wtime();
    double timeout_seconds = 5.0; // set timeout to 5 seconds

     /* generate and send tasks to workers */
    srand(time(NULL)); // Seed the random number generator with the current time

    // while(1) {
    //     /* generate a new task */
    //     /* fill in the task data structure */
    //     task.x = rand() % 100 + 1;
    //     task.y = rand() % 100 + 1;

    //     printf("[Process Emitter] I send value %d %d to process %d.\n", task.x, task.y, next_worker);

    //     MPI_Request request;
    //     MPI_Isend(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD, &request);

    //     // check if timeout has expired
    //     if (MPI_Wtime() - start_time > timeout_seconds) {
    //         printf("Timeout expired, terminating loop.\n");
    //         break;
    //     }

    //     // wait for the send operation to complete before proceeding with the next task
    //     int completed = 0;
    //     while (!completed) {
    //         MPI_Test(&request, &completed, &status);
    //     }

    //     next_worker = (next_worker % num_workers) + 1;
    // } /* explicit task request */

    // while(1) {
    //      /* generate a new task */
    //      /* fill in the task data structure */
    //     task.x = rand() % 100 + 1;
    //     task.y = rand() % 100 + 1;

    //     printf("[Process Emitter] I send value %d %d to process %d.\n", task.x, task.y, next_worker);
    //     MPI_Send(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD);

    //     // check if timeout has expired
    //     if (MPI_Wtime() - start_time > timeout_seconds) {
    //         printf("Timeout expired, terminating loop.\n");
    //         break;
    //     }

    //     next_worker = (next_worker % num_workers) + 1;
    // } /* round-robin */

     while(next_worker < num_workers) {
         /* generate a new task */
         /* fill in the task data structure */
        task.x = rand() % 100 + 1;
        task.y = rand() % 100 + 1;

        printf("[Process Emitter] I send value %d %d to process %d.\n", task.x, task.y, next_worker);
        MPI_Send(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD);

        // next_worker = (next_worker % num_workers) + 1;
        next_worker++;
    }


    for (int i = 1; i < num_workers; i++) {
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_END_STREAM, MPI_COMM_WORLD);
    }

}

/* define your worker process */
void worker(int rank) {
    MPI_Status status;
    MPI_Request request;
    TaskData task;
    ResultData result;

    /* receive and process tasks from emitter */
    MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

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
        printf("[Process Worker %d] I received and computed value %.2f %.2f from process %d with degree %d.\n", rank, result.x, result.y, EMITTER, i);
        MPI_Bsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
        i++;
    }

    //  while (1) {
    //     MPI_Irecv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    //     MPI_Wait(&request, &status);

    //     if (status.MPI_TAG == TAG_END_STREAM) {
    //         MPI_Isend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD, &request);
    //         printf("end worker\n");
    //         break;
    //     }

    //     compute(&task, &result);
    //     printf("[Process Worker %d] I received and computed value %.2f %.2f from process %d.\n", rank, result.x, result.y, EMITTER);

    //     MPI_Isend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD, &request);
    //     MPI_Wait(&request, &status);
    // } /* explicit task request */
    // while (1) {
    //     MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //     if (status.MPI_TAG == TAG_END_STREAM) {
    //         MPI_Rsend(NULL, 0, MPI_BYTE, COLLECTOR, TAG_END_STREAM, MPI_COMM_WORLD);
    //         printf("end workder\n");
    //         break;
    //     }
    //     compute(&task, &result);
    //     printf("[Process Worker %d] I received and computed value %.2f %.2f from process %d.\n", rank, result.x, result.y, EMITTER);
    //     MPI_Rsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
    // } /* round-robin */
}

/* define your collector process */
void collector(int num_workers) {
    MPI_Status status;
    TaskData task;
    ResultData result;
    MPI_Request request;
    int next_worker = 1;

    for(int i = 1; i < num_workers; i++) {
        MPI_Irecv(&result, sizeof(ResultData), MPI_BYTE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        MPI_Wait(&request, &status);

        printf("[Process Collecter %d] I received value %.2f %.2f from process %d.\n", COLLECTOR, result.x, result.y, i);
    }



    //  while(1) {
    //     MPI_Irecv(&result, sizeof(ResultData), MPI_BYTE, next_worker, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
    //     MPI_Wait(&request, &status);

    //     if (status.MPI_TAG == TAG_END_STREAM) {
    //         printf("end collector\n");
    //         break;
    //     }

    //     printf("[Process Collecter %d] I received value %.2f %.2f from process %d.\n", COLLECTOR, result.x, result.y, next_worker);

    //     next_worker = (next_worker % (num_workers - 1)) + 1;
    // } /* explicit task request */

    // while(1) {    
    //     MPI_Recv(&result, sizeof(ResultData), MPI_BYTE, next_worker, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    //     if (status.MPI_TAG == TAG_END_STREAM) {
    //         printf("end collector\n");
    //         break;
    //     }
    //     printf("[Process Collecter %d] I received value %.2f %.2f from process %d.\n", COLLECTOR, result.x, result.y, next_worker);
    //     next_worker = (next_worker % (num_workers - 1)) + 1;
    // } /* round-robin */
    
}

int main(int argc, char** argv) {
    int num_workers, world_rank, num_jobs;  
    
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &num_workers);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    if(num_workers != 10)
    {
        printf("This application is meant to be run with 10 processes.\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    switch (world_rank)
    {
        case EMITTER:
            emitter(COLLECTOR);
        break;
        case COLLECTOR:
            collector(COLLECTOR);
        break;
        default:
            worker(world_rank);
        break;
    }
    
    MPI_Finalize();
    
    return 0;
}