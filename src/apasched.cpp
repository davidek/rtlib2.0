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
#include <sstream>

#include <apasched.hpp>
#include <kernel.hpp>
#include <edfsched.hpp>
#include <rmsched.hpp>
#include <SchedulerFactory.hpp>

#define MIN_PRIO std::numeric_limits<MetaSim::Tick::impl_t>::max()
#define MAX_PRIO 0

static void myassert(bool exp, const std::string &msg="Assertion Failed") {
    if (! exp)
        throw RTSim::APASchedExc(msg);
}

namespace RTSim {
    //  ************ Initialization: addTask, removeTask, addCPU  *************

    void APAScheduler::enqueueModel(
            TaskModel *model, const std::string &other_params) {

        Scheduler::enqueueModel(model);
        AbsRTTask *task = model->getTask();
        APAModel *apamodel = apafind(task);

        bool assigned = false;
        for (SCHEDIT i = _inner_sched.begin(); i != _inner_sched.end(); i++) {
            if (apamodel->allowedOn(i->first)) {
                i->second->addTask(task, other_params);
                if (!assigned) {
                    _task_cpu[task] = i->first;
                    assigned = true;
                }
            }
        }
        if (!assigned)
            throw APASchedExc("Task is not allowed to run on any available CPU");
    }

    void APAScheduler::addTask(
            AbsRTTask* task, Affinity affinity, const std::string &other_params) {
        enqueueModel(new APAModel(task, affinity), other_params);
    }

    void APAScheduler::addTask(AbsRTTask* task, const std::string &params) {
        uint64_t mask = 0x0;
        std::string other_params;
        std::istringstream paramsstream(params);
        paramsstream >> std::hex >> mask;
        if (mask == 0x0)
            throw APASchedExc("Can't add a task without affinity.");
        std::getline(paramsstream, other_params);  // remaining string
        enqueueModel(new APAModel(task, mask), other_params);
    }

    void APAScheduler::removeTask(AbsRTTask* task) {
        // TODO: remove from all subschedulers where it was added,
        // then cleanup from _tasks
        throw APASchedExc("NOT implemented.");
    }

    void APAScheduler::addCPU(CPU *c) {
        if (_inner_sched.count(c))
            throw APASchedExc("CPU already present.");
        if (_cpuids.count(c->getIndex()))
            throw APASchedExc("Duplicate CPU index.");
        _inner_sched[c] = _inner_factory->createScheduler();
        _latest_prio[c] = -1;
        //_pending_push[c] = false;
        _cpuids.insert(c->getIndex());
    }


    //  ************ Runtime API: insert, extract, getFisrt  *************

    void APAScheduler::insert(AbsRTTask *task) throw(RTSchedExc, BaseExc) {
        DBGENTER(_APASCHED_DBG_LVL);

        CPU *c = _task_cpu.at(task);
        _inner_sched.at(c)->insert(task);
        // could avoid push if the arriving task is not the first or second?
        push(c);
    }

    void APAScheduler::extract(AbsRTTask *task) throw(RTSchedExc, BaseExc) {
        DBGENTER(_APASCHED_DBG_LVL);

        CPU *c = _task_cpu.at(task);
        _inner_sched.at(c)->extract(task);
        pull(c);
    }

    AbsRTTask *APAScheduler::getFirst(CPU *c) {
        DBGENTER(_APASCHED_DBG_LVL);

        Scheduler *isched = _inner_sched.at(c);
        AbsRTTask *first = isched->getFirst();
        AbsRTTask *curr = dynamic_cast<MRTKernel *>(_kernel)->getTask(c);
        Tick lp = _latest_prio.at(c);

        // recall priority comparison is up-side-down: pull if first
        // has lower priority than latest_prio
        if ((!curr) || (!first)
               || (first && lp >=0 && lp < isched->getPriority(first))) {
            pull(c);
            first = isched->getFirst();  // may be changed after pull()
        }

        if (first && curr && first != curr) {  // preemption
            //_pending_push[c] = true;
            push(c);
        }

        return first;
    }

    void APAScheduler::notify(AbsRTTask * task) {
        DBGENTER(_APASCHED_DBG_LVL);

        if (task) {
            CPU *c = _task_cpu.at(task);
            Scheduler *isched = _inner_sched.at(c);
            _latest_prio[c] = isched->getPriority(task);
            //if (_pending_push.at(c)) {
            //    push(c);
            //}
        }
    }

    void APAScheduler::push(CPU *c) {
        DBGENTER(_APASCHED_DBG_LVL);

        AbsRTTask *task = _inner_sched.at(c)->getTaskN(1); // second task
        if (task == NULL) {
            DBGPRINT_3("Less than 2 tasks in queue for cpu ", c->getIndex(),
                    ": nothing to push");
            return;
        }

        Tick pushedPrio = _inner_sched.at(c)->getPriority(task);
        APAModel *apam = apafind(task);
        DBGPRINT_6("Trying to push ", taskname(task), " with prio ",
                pushedPrio, " from cpu ", c->getIndex());
        // note: in varialbe names, "high/low/max/min priority" has the natural
        // meaning, even though values are reversed
        Tick lowestSoFar = pushedPrio;  // Not interested in
        CPU *lowestWhere = NULL;
        for (SCHEDIT i = _inner_sched.begin(); i != _inner_sched.end(); i++) {
            if (i->first != c  &&  apam->allowedOn(i->first)) {
                DBGPRINT_2("Trying cpu ", i->first->getIndex());
                AbsRTTask *firstHere = i->second->getFirst();
                Tick maxPrioHere;
                if (firstHere) {
                    maxPrioHere = i->second->getPriority(firstHere);
                    DBGPRINT_4(".best is ", taskname(firstHere), " with prio ", maxPrioHere);
                }
                else{
                    maxPrioHere = MIN_PRIO;
                    DBGPRINT(".this processor is free!");
                }
                // if maxPrioHere is lower priority than lowestSoFar
                if (maxPrioHere > lowestSoFar) {
                    lowestSoFar = maxPrioHere;
                    lowestWhere = i->first;
                    DBGPRINT(".could push here");
                }
            }
        }
        // if found a cpu running a task with lower priority than pushedPrio
        if (lowestSoFar > pushedPrio) {
            myassert(lowestWhere != NULL);
            DBGPRINT_2("Pushing to cpu ", lowestWhere->getIndex());
            _inner_sched.at(c)->extract(task);
            _inner_sched.at(lowestWhere)->insert(task);
        }
        else {
            DBGPRINT("Couldn't find a good cpu to push");
        }
    }

    void APAScheduler::pull(CPU *c) {
        DBGENTER(_APASCHED_DBG_LVL);

        /*
        Tick pulledPrio;
        AbsRTTask *task = _inner_sched.at(c)->getFirst();
        if (task)
            pulledPrio = _inner_sched.at(c)->getPriority(task);
        else
            pulledPrio = MIN_PRIO;
        DBGPRINT_4("Trying to pull to cpu ", c->getIndex(),
                " where current highest prio is ", pulledPrio);
        // note: in varialbe names, "high/low/max/min priority" has the natural
        // meaning, even though values are reversed
        Tick highestSoFar = pulledPrio;  // don't care of prio lower than mine
        AbsRTTask *highestTask = NULL;
        CPU *highestWhere = NULL;
        for (SCHEDIT i = _inner_sched.begin(); i != _inner_sched.end(); i++) {
            if (i->first == c)
                continue;

            DBGPRINT_2("Trying cpu ", i->first->getIndex());

            // scan local queue by priority until a compatible one is found
            AbsRTTask *t;
            for (int qindex = 0; ; qindex++) {
                t = i->second->getTaskN(qindex);
                if (t == NULL || apafind(t)->allowedOn(c))
                    break;
            }
            // now `t` is the first compatible task in this queue (or NULL)
            Tick maxPrioHere;
            if (t)
                maxPrioHere = i->second->getPriority(t);
            else
                maxPrioHere = MIN_PRIO;
            // if maxPrioHere is higher priority than highestSoFar
            if (maxPrioHere < highestSoFar) {
                highestSoFar = maxPrioHere;
                highestTask = t;
                highestWhere = i->first;
                DBGPRINT(".could pull from here");
            }
        }
        // if found a task with higher priority than pulledPrio
        if (highestSoFar < pulledPrio) {
            myassert(highestTask != NULL);
            myassert(highestWhere == _task_cpu.at(highestTask));
            DBGPRINT_4("Pulling task ", taskname(highestTask),
                    " from cpu ", highestWhere->getIndex());
            _inner_sched.at(highestWhere)->extract(highestTask);
            _inner_sched.at(c)->insert(highestTask);
        }
        else {
            DBGPRINT("Couldn't find a good cpu to pull from");
        }
        */
    }

    //  ************ Other protected utilities ******************

    APAModel * APAScheduler::apafind(AbsRTTask *task) {
        // return dynamic_cast<APAModel *>(find(task)), but check for errors
        TaskModel *tm = find(task);
        myassert(tm, "Task not found.");
        APAModel *ret = dynamic_cast<APAModel *>(tm);
        myassert(ret, "Invalid (non-APA) Model");
        return ret;
    }

    void APAScheduler::invariant() {
#ifdef __DEBUG__
        typedef map<AbsRTTask*, TaskModel*>::iterator TASKIT; // _tasks
        typedef map<AbsRTTask*, CPU*>::iterator TCPUIT; // _task_cpu
        // Every task is present in all and only the cpus matching its affinity
        for (TASKIT ti = _tasks.begin(); ti != _tasks.end(); ti++) {
            for (SCHEDIT ci=_inner_sched.begin(); ci!=_inner_sched.end(); ci++){
                AbsRTTask* task = ti->first;   //TaskModel *model = ti->second;
                CPU *cpu = ci->first;           Scheduler *sched = ci->second; 
                //                WTF???????  why does this NOT compile??
//                myassert(apafind(task)->allowedOn(cpu) == bool(sched->find(task)));
            }
        }
        // Every task is present in at most one inner scheduler.
        // If it is present, it must be on the one where _task_cpu points
        for (TASKIT ti = _tasks.begin(); ti != _tasks.end(); ti++) {
            
        }
#endif
    }
}
