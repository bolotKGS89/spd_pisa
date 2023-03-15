#ifndef FARM_H
#define FARM_H

class Farm
{
protected:
    void doEmit();
    void doWork();
    void doCollect();
};

class roundRobinFarm : Farm {
    inline void roundRobinFarm::doEmit() {

    }

    inline void roundRobinFarm::doWork() {
    }

    inline void roundRobinFarm::doCollect() {
    }
};



#endif