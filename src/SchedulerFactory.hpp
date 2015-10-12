/***************************************************************************
 *   begin                : Tue Sep 15 17:28:58 CEST 2015
 *   copyright            : (C) 2015 by Daniel Casini
 *   email                : daniel.casini@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __SCHEDULERFACTORY_HPP__
#define __SCHEDULERFACTORY_HPP__

#include <partionedmrtkernel.hpp>
#include <scheduler.hpp>
#include <edfsched.hpp>
#include <rmsched.hpp>
#include <fpsched.hpp>

namespace RTSim {

class PartionedMRTKernel;

/**
    Abstract Scheduler Factory

    @author Casini Daniel
*/
class AbsSchedulerFactory {

public:
    virtual Scheduler* createScheduler() = 0;

};
	
/**
    EDF Scheduler Factory

    @author Casini Daniel
*/
class EDFSchedulerFactory : public AbsSchedulerFactory {

public:
    virtual Scheduler* createScheduler(){ return new EDFScheduler; }

};
	
/**
    Rate Monotonic Scheduler Factory

    @author Casini Daniel
*/
class RMSchedulerFactory : public AbsSchedulerFactory {
public:
    virtual Scheduler* createScheduler(){ return new RMScheduler; }
};
	
}//namespace
#endif
