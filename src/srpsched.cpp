/***************************************************************************
    begin                : 2015-11-07
    copyright            : (C) 2015 Davide Kirchner
    email                : davide.kirchner@yahoo.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <srpsched.hpp>
#include <kernel.hpp>
#include <task.hpp>
#include <sstream>
#include <iostream>
#include <limits>

namespace RTSim {


    TaskModel *SRPBaseModel::_asTaskModel() {
        TaskModel *ret = dynamic_cast<TaskModel *>(this);
        if (!ret)
            throw SRPSchedExc("SRPBaseModel instances must also inherit from TaskModel!");
        return ret;
    };


    AbsRTTask *SRPBaseScheduler::getFirst() {
        DBGENTER("SRPBaseScheduler");
        AbsRTTask *queue_first = getFirstOrig();
        AbsRTTask *curr = _getCurrExe();

        // DEBUG vvv
        if (queue_first)
            DBGPRINT_6("First in queue:    ",
                    dynamic_cast<Task*>(queue_first)->getName(), " with priority ",
                    srpfind(queue_first)->_asTaskModel()->getPriority(), " and preemption level ",
                    srpfind(queue_first)->getPreemptionLevel());
        else
            DBGPRINT("Nothing in the queue");
        if (curr)
            DBGPRINT_6("Currently running: ",
                    dynamic_cast<Task*>(curr)->getName(),
                    " with priority ", srpfind(curr)->_asTaskModel()->getPriority(),
                    " and preemption level ", srpfind(curr)->getPreemptionLevel());
        else
            DBGPRINT("Nothing running");
        DBGPRINT_2("System ceiling is ", systemCeiling());
        // DEBUG ^^^

        if (curr == NULL) {
            // resume the task that has been most recently preempted
            // it might be null, in which case the queue_first will run anyway
            curr = _resman->systemCeilingLocker();
        }

        if (curr && queue_first && curr != queue_first) {
            if ( // SRP preemption policy
                 (srpfind(queue_first)->getPreemptionLevel() > systemCeiling())
                 &&
                 // spare useless switches if priorities are equal
                 // (surely first.prio <= curr.prio, because the first is first)
                 // (note values for priorities are inverted!!)
                 (srpfind(queue_first)->_asTaskModel()->getPriority()
                  < srpfind(curr)->_asTaskModel()->getPriority())
               ) {
                return queue_first;
            }
            else {
                return curr;
            }
        }
        else {  // queue_first is NULL, or queue_first==curr
            return queue_first;
        }
    }


    void EDF_SRPScheduler::addTask(AbsRTTask *task, int preemption_level) {
        if (find(task) != NULL) 
            throw RTSchedExc("Element already present");
        _tasks[task] = new EDF_SRPModel(task, preemption_level);
    }

    void EDF_SRPScheduler::addTask(AbsRTTask *t, const std::string &params) {
        int lvl = INT_MIN;
        std::istringstream(params) >> lvl;
        if (lvl != INT_MIN)
            addTask(t, lvl);
        else
            throw RTSchedExc("Can't add a task without preemption level.");
    }

    void EDF_SRPScheduler::removeTask(AbsRTTask *task) {
        // In EDFScheduler and FPscheduler this method does nothing.
        // Unless I'm missing something, there is a memory leak here.
        // luckly, there is a very low chance that anybody will ever call this
    }

}
