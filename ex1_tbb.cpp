#include "tbb/tbb.h”

class TaskScheduler {
public:
    TaskScheduler() {
        tbb::task_scheduler_init init(NUM_THREADS);
    }
};

class ParallelFor : public tbb::parallel_for {
public:
    void operator()(const tbb::blocked_range<int>& range) const override {
        // Perform computation here
    }
};

int main() {
    TaskScheduler task_scheduler;
    ParallelFor parallel_for;
    parallel_for(tbb::blocked_range<int>(0, N));
}