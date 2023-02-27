#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

// Function declarations
void emitter(int size, int *stream);
void collector(int size, int *stream);
void worker(int task_size, int *task_buffer, int *result_buffer);

int main(int argc, char **argv) {
    int rank, size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Initialize data structures
    int *stream = NULL;
    int stream_size = 0;

    // Emitter process
    if (rank == 0) {
        emitter(size, stream);
    }
    // Worker processes
    else if (rank > 0 && rank < size-1) {
        // Receive tasks and execute them until end of stream is reached
        int task_size;
        MPI_Status status;
        while (1) {
            // Receive task size
            MPI_Recv(&task_size, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == 0) { // End of stream tag
                break;
            }
            // Receive task data
            int *task_buffer = (int*) malloc(task_size * sizeof(int));
            MPI_Recv(task_buffer, task_size, MPI_INT, 0, status.MPI_TAG, MPI_COMM_WORLD, &status);
            // Execute task
            int *result_buffer = (int*) malloc(task_size * sizeof(int));
            worker(task_size, task_buffer, result_buffer);
            // Send result back to collector
            MPI_Send(result_buffer, task_size, MPI_INT, size-1, status.MPI_TAG, MPI_COMM_WORLD);
            free(task_buffer);
            free(result_buffer);
        }
    }
    // Collector process
    else {
        collector(size, stream);
    }

    MPI_Finalize();
    return 0;
}

// Emits a stream of tasks to the worker processes
void emitter(int size, int *stream) {
    // Generate stream of tasks
    int stream_size = 100;
    stream = (int*) malloc(stream_size * sizeof(int));
    for (int i = 0; i < stream_size; i++) {
        stream[i] = i;
    }

    // Send tasks to worker processes
    int task_size = stream_size / (size-2);
    int remainder = stream_size % (size-2);
    int offset = 0;
    for (int i = 1; i < size-1; i++) {
        int this_task_size = task_size + (i <= remainder ? 1 : 0);
        MPI_Send(&this_task_size, 1, MPI_INT, i, i, MPI_COMM_WORLD);
        MPI_Send(&stream[offset], this_task_size, MPI_INT, i, i, MPI_COMM_WORLD);
        offset += this_task_size;
    }

    // Send end-of-stream tag to worker processes
    int end_tag = 0;
    for (int i = 1; i < size-1; i++) {
        MPI_Send(&end_tag, 1, MPI_INT, i, end_tag, MPI_COMM_WORLD);
    }

    free(stream);
}

// Collects results from the worker processes and prints them out
void collector(int size, int *stream) {
    // Receive results from worker processes and add them to the stream
    MPI_Status status;
    while (1) {
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

}