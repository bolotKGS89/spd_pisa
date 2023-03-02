#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>

#define TAG_JOB 1
#define TAG_RESULT 2
#define EMITTER 0
#define COLLECTOR 9

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
     /* generate and send tasks to workers */
    srand(time(NULL)); // Seed the random number generator with the current time
    while(1) {
         /* generate a new task */
         /* fill in the task data structure */
        task.x = rand() % 100 + 1;
        task.y = rand() % 100 + 1;

        // printf("[Process Emitter] I send value %d %d to process %d.\n", task.x, task.y, next_worker);
        MPI_Send(&task, sizeof(TaskData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD);

        // MPI_Recv(NULL, 0, MPI_BYTE, MPI_ANY_SOURCE, TAG_RESULT, MPI_COMM_WORLD, &status);
        // if(status.MPI_TAG == TAG_RESULT) {
        //     printf("done");
        //     break;
        // }

        next_worker = (next_worker % num_workers) + 1;
    }

}

/* define your worker process */
void worker(int rank) {
    MPI_Status status;
    TaskData task;
    ResultData result;

    /* receive and process tasks from emitter */
    while (1) {
        MPI_Recv(&task, sizeof(TaskData), MPI_BYTE, EMITTER, TAG_JOB, MPI_COMM_WORLD, &status);
        compute(&task, &result);
        // printf("[Process Worker %d] I received and computed value %.2f %.2f from process %d.\n", rank, result.x, result.y, EMITTER);
        MPI_Rsend(&result, sizeof(ResultData), MPI_BYTE, COLLECTOR, TAG_JOB, MPI_COMM_WORLD);
    }
}

/* define your collector process */
void collector(int num_workers) {
    MPI_Status status;
    TaskData task;
    ResultData result;
    MPI_Request request;
    int next_worker = 1;

    // for (int i = 1; i < num_workers; i++)
    // {
     while(1) {    
        MPI_Recv(&result, sizeof(ResultData), MPI_BYTE, next_worker, TAG_JOB, MPI_COMM_WORLD, &status);
        printf("[Process Collecter %d] I received value %.2f %.2f from process %d.\n", COLLECTOR, result.x, result.y, next_worker);
        next_worker = (next_worker % num_workers) + 1;
    }
    MPI_Rsend(NULL, 0, MPI_BYTE, EMITTER, TAG_RESULT, MPI_COMM_WORLD);

    
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


    if (world_rank == EMITTER) {
        emitter(COLLECTOR);
    } else if (world_rank == COLLECTOR) {
        collector(COLLECTOR);
    }
    else {
        worker(world_rank);
    }
        
    
    MPI_Finalize();
    
    return 0;
}