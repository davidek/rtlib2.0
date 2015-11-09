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
#include <sstream>
#include <iostream>
#include <limits>

namespace RTSim {

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
