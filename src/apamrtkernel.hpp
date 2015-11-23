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

#ifndef __PARTIONEDMRTKERNEL_HPP__
#define __PARTIONEDMRTKERNEL_HPP__

#include <vector>

#include <kernel.hpp>
#include <kernevt.hpp>
#include <mrtkernel.hpp>
#include <partionedmrtkernel.hpp>
#include <SchedulerFactory.hpp>

namespace RTSim {

    class absCPUFactory;
    class absSchedulerFactory;

    //class UndefinedCPUException;
    //class UndefinedCPUFactoryException
    //class UndefinedSchedulerFactoryException


    /** 
        \ingroup kernel

        @brief Multiprocessor kernel with
      
        @see MRTKernel, PartitionedMRTKernel, absCPUFactory, Scheduler, AbsRTTask

        @author Davide Kirchner. Heavily inspired from Danile Casini's PartitionedMRTKernel
    */
    class APAMRTKernel : public MRTKernel {
    public:
        using MRTKernel::MRTKernel;

        /** Call the per-cpu dispatch on all cpus */
        virtual void dispatch();

        /** Check if dispatching on the given CPU is needed. If so, start */
        virtual void dispatch(CPU *);

        /**
            Inherited from MRTKernel, this function handles the event "start of a context switch on a mp platform"
        */
        void onBeginDispatchMulti(BeginDispatchMultiEvt* e);

        /**
            Inherited from MRTKernel, this function handles the event "end of a context switch on a mp platform"
        */
        void onEndDispatchMulti(EndDispatchMultiEvt* e);

        /**
         Returns the CPU on which the given task currenlty lives
        */
        //virtual CPU* getProcessor(const AbsRTTask *t) const;

        /** @brief get the number or CPUs */
        int numCPUs();

        /**
        * Allocate the tasks using the task allocation strategy
        */
        void allocateTask();
    };
} // namespace RTSim

#endif
