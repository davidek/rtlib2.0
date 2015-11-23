/***************************************************************************
 *   begin                : 5015-11-21
 *   copyright            : (C) 2015 Davide Kirchner
 *   email                : davide.kirchner@yahoo.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <algorithm>

#include <simul.hpp>

#include <apamrtkernel.hpp>
#include <cpu.hpp>
#include <mrtkernel.hpp>
#include <resmanager.hpp>
#include <scheduler.hpp>
#include <task.hpp>
#include <edfsched.hpp>


namespace RTSim {

    using namespace std;
    using namespace MetaSim;


    int APAMRTKernel::numCPUs() {
        return _m_currExe.size();
    }


    void APAMRTKernel::dispatch()
    {
        // TODO
        for (ITCPU i = _m_currExe.begin(); i != _m_currExe.end(); i++)
            dispatch(i->first);
    }

    void APAMRTKernel::dispatch(CPU *c)
    {
        DBGENTER(_KERNEL_DBG_LEV);

        _beginEvt[c]->drop();
        _beginEvt[c]->post(SIMUL.getTime());
    }

    void APAMRTKernel::onBeginDispatchMulti(BeginDispatchMultiEvt* e)
    {
        // TODO: ricontrollare
        // Basically implements the single-processor version of onBeginDispatch
        DBGENTER(_KERNEL_DBG_LEV);

        CPU * c = e->getCPU();
        AbsRTTask *newExe = _sched->getFirst(c);
        AbsRTTask *currExe = _m_currExe.at(c);

        if (newExe != NULL)
            DBGPRINT_2("From sched: ", taskname(newExe));

        if(currExe != newExe) {
            if (currExe != NULL) { 
                _m_oldExe[currExe] = c;
                _m_currExe[c] = NULL;
                _m_dispatched[currExe] = NULL;
                currExe->deschedule();
            }
            if (newExe != NULL) { 
                DBGPRINT_4("Scheduling task ", taskname(newExe), " on cpu ", c);
                _isContextSwitching[c] = true;
                _m_currExe[c] = newExe;
                _m_dispatched[newExe] = c;
                // TODO: account for _migrationDelay
                Tick overhead (_contextSwitchDelay);
                _endEvt[c]->setTask(newExe);
                _endEvt[c]->post(SIMUL.getTime() + overhead);
            }
        }
        else {
            // _sched->notify(newExe); // was here in single-processor, but may be useless
            if (newExe != NULL)
                DBGPRINT_2("Now Running: ", taskname(newExe));
        }
    }

    void APAMRTKernel::onEndDispatchMulti(EndDispatchMultiEvt* e)
    {
        // performs the "real" context switch
        DBGENTER(_KERNEL_DBG_LEV);

        AbsRTTask *t = e->getTask();
        CPU *c = e->getCPU();

        _m_currExe[c] = t;

        DBGPRINT_2("CPU: ", c);
        DBGPRINT_2("Task: ", taskname(t));
        
        // t could be null (because of an idling processor)
        if (t) t->schedule();

        _isContextSwitching[c] = false;
        _sched->notify(t);
    }
    
//CPU * APAMRTKernel::pickCPU()
//{
//    if(!numCPUs())
//        throw UndefinedCPUException(
//                "No CPU exisits (add CPUs before adding Tasks)");
//
//    return _m_sched.cbegin()->first;
//}

}
    

