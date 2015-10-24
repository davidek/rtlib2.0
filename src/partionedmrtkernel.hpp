/***************************************************************************
 *   begin                : Sat Aug 15 17:28:58 CEST 2015
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

#ifndef __PARTIONEDMRTKERNEL_HPP__
#define __PARTIONEDMRTKERNEL_HPP__

#include <vector>

#include <kernel.hpp>
#include <kernevt.hpp>
#include "TaskAllocation.hpp"
#include "mrtkernel.hpp"
#include "SchedulerFactory.hpp"

#define _PARTIONEDMRTKERNEL_DBG_LEV "PartionedMRTKernel"

namespace RTSim {

    class absCPUFactory;

	class absSchedulerFactory;

    class AbsTaskAllocation;

    /**
        Basic exception class for Partioned Multiprocessor Real Time Kernel.

        @author Casini Daniel
    */
    class PartionedMRTKernelException : public BaseExc{

    public:
        inline PartionedMRTKernelException(const string message, const string cl = "PartionedMRTKernel", const string md = "RT_SIM") : BaseExc(message, cl, md) {}

    };

    /**
        Exception raised whenever a generalized task is associated with a processor
        not present in the kernel

        @author Casini Daniel
    */
    class UndefinedCPUException : public PartionedMRTKernelException{

    public:
        inline UndefinedCPUException(const string message = "The CPU is not specified!",
                                     const string cl = "PartionedMRTKernel", const string md = "RT_SIM") : PartionedMRTKernelException(message, cl, md) {}

    };

    /**
        Exception raised whenever a method that uses the factory is called without
        having defined it

        @author Casini Daniel
    */
    class UndefinedCPUFactoryException : public PartionedMRTKernelException{

    public:
        inline UndefinedCPUFactoryException(const string message = "The CPU Factory is not specified!",
                                            const string cl = "PartionedMRTKernel", const string md = "RT_SIM") : PartionedMRTKernelException(message, cl, md) {}

    };

    /**
        Exception raised whenever a method that uses the factory is called without
        having defined it

        @author Casini Daniel
    */
    class UndefinedSchedulerFactoryException : public PartionedMRTKernelException{

    public:
        inline UndefinedSchedulerFactoryException(const string message = "The SchedulerFactory is not specified!",
                                                  const string cl = "PartionedMRTKernel", const string md = "RT_SIM") : PartionedMRTKernelException(message, cl, md) {}

    };

    /**
        Exception raised whenever the allocate() method is called without having
        defined an allocation strategy
        @author Casini Daniel
    */
    class UndefinedAllocationStrategyException : public PartionedMRTKernelException{

    public:
        inline UndefinedAllocationStrategyException(const string message = "The TaskAllocation strategy is not specified!",
                                                    const string cl = "PartionedMRTKernel", const string md = "RT_SIM") : PartionedMRTKernelException(message, cl, md) {}

    };


    /** 
        \ingroup kernel
      
        An implementation of a real-time multi processor kernel with
        partioned scheduling. It contains:
 
        - a pointer to the CPU factory used to create CPUs managed by
          the kernel;

		- a pointer to the Scheduler factory used to create Schedulers for each CPUs;

        - a map of pointers to CPU and scheduler, which keeps the
          information about CPUs assignment to Schedulers;

		- a map of pointers to Task and CPU, which keeps the
          information about tasks assignment to CPUs;

		- a map of pointers to task and string, which keeps the
          information about the parameter of a task in the case in which the
		  allocation of the task to a cpu is not performed directly, but with
		  a task allocation (bin packing) strategy, calling the method addTask 
		  without specyfing the CPU;

		- the number of CPUs of the architecture

		- a pointer to a taskAllocator, which allocate the tasks to the CPUs
		accordingly to a task allocation (bin packing) algorithm

        - a pointer to a Resource Manager, which is responsable for
          resource access related operations and thus implements a
          resource allocation policy; --------------------------------------> TO DO

        - a map of pointers to CPU and task, which keeps the
          information about current task assignment to CPUs;
        
        - the set of tasks handled by this kernel.
      
        This implementation is quite general: it lets the user of this
        class the freedom to adopt any scheduler derived form
        Scheduler and a resorce manager derived from ResManager or no
        resorce manager at all.  The kernel for a multiprocessor
        system with different CPUs can be also be simulated. It is up
        to the instruction class to implement the correct duration of
        its execution by asking the kernel of its task the speed of
        the processor on which it's scheduled.
      
      
        @see MRTKernel, absCPUFactory, Scheduler, AbsRTTask, PeriodicServerVM

        @author Casini Daniel
    */
    class PartionedMRTKernel : public MRTKernel {

		friend class AbsTaskAllocation;
		friend class FirstFitDecreasingTaskAllocation;
        friend class Server;

    protected:

		/// The scheduler of each CPU
		map<CPU *, Scheduler*> _cpuSchedulerMap;

		/// The CPU of each task
		map<const AbsRTTask *, CPU*> _taskCPUMap;

		/// The scheduler of each task
		map<const AbsRTTask *, Scheduler*> _taskSchedulerMap;

		/// The parameter of each task not directly added to a CPU (taskAllocator will do this)
		map<const AbsRTTask *, string> _taskParam;

		/// Number of CPUs of the architecture
		unsigned int _nCPU;	

		/// Task allocator, to allocate tasks to CPUs using a task allocation (bin packing) strategy
        AbsTaskAllocation *_taskAllocator;

		/// Scheduler Factory. Used in one of the constructors.
		AbsSchedulerFactory *_schedFactory;

        /**
         * Returns the CPU associated with the given task
        */
        CPU *getCPUFromTask(AbsRTTask *task);

    public:
  

        /**
            Needs to know only the name, does not set the number of CPUs, the CPU/Scheduler factory and .
            Used to directly allocate tasks to CPU (without task allocator)
        */
        PartionedMRTKernel(const string& name);

        /**
            Needs to know the name and the number of CPUs, uses default choices for CPU/Scheduler factory
            and task allocation strategy.
        */
        PartionedMRTKernel(int n, const string& name);

        /**
            Allows to specify the number of CPUs, the scheduler factory, and the task allocation (bin packing) strategy
        */
        PartionedMRTKernel(int n, const string& name, AbsSchedulerFactory *schedFactory, AbsTaskAllocation *taskAllocationStrategy);

        /**
            Allows to specify the number of CPUs, the scheduler and cpu factory, and the task allocation (bin packing) strategy
        */
        PartionedMRTKernel(int n, const string& name, absCPUFactory *cpuFactory, AbsSchedulerFactory *schedFactory, AbsTaskAllocation *taskAllocationStrategy);


        ~PartionedMRTKernel();

        /**
            Adds a new CPU with a given scheduler
        */
        void addCPU(CPU *c, Scheduler *s);

        /**
            Adds a new CPU is automatically combined with a scheduler created by the factory (that must be specified)
        */
        void addCPU(CPU *c);

        /**
        *   Adds a CPU using the CPUFactory, combined with a scheduler created by the SchedulerFactory
        */
        void addCPU();

        /**
            Adds a task to the kernel, but does not bind it to a CPU (the job will be done by the task allocator, that must be
            defined)
        */
        void addTask(AbsRTTask &t, const string &param);

        /**
            Adds a new CPU is automatically combined with a scheduler created by the factory (that must be specified)
        */
        void addTask(AbsRTTask &t, const string &param, CPU *c);

        /**
            Only adapter from MRTKernel with the PartionedMRTKernel data structure,
            to be revised
        */
        void suspend(AbsRTTask *task);

        /**
            Inherited from RTKernel, adds the task to the appropriate scheduler
        */
        void onArrival(AbsRTTask *t);

        /**
            Inherited from MRTKernel, this function handles the event "start of a context switch on a mp platform"
        */
        void onBeginDispatchMulti(BeginDispatchMultiEvt* e);

        /**
            Inherited from MRTKernel, this function handles the event "end of a context switch on a mp platform"
        */
        void onEndDispatchMulti(EndDispatchMultiEvt* e);

        /**
            Inherited from RTKernel, it removes the task from its ready queue
        */
        void onEnd(AbsRTTask *task);

        /**
         Returns the CPU on which the given task is allocated
        */
        virtual CPU* getProcessor(const AbsRTTask *t) const;

        /**
        This function is common to all Entity
        objects. Before every new simulation run, this
        function is invoked.
        Removes each running task from the
        relative scheduler, and sets to NULL the dispatching
        correspondence (map) between tasks and CPUs
        */
        virtual void newRun();

        /**
        This function is common to all Entity
        objects. after every new simulation run, this
        function is invoked. Removes each running task from the
        relative scheduler
        */
        virtual void endRun();

        /**
        * Allocate the tasks using the task allocation strategy
        */
        void allocateTask();

        /**
        * Add a VM to the specified CPU
        */
        void addVM(PeriodicServerVM &VM, CPU *cpu);

        /**
        * Add a VM to the kernel (taskAllocator must be used
        * to assign a CPU)
        */
        void addVM(PeriodicServerVM &VM);

        /**
        * Returns the Task-CPU Map
        */
        map<const AbsRTTask *, CPU *> getTaskCPUMap() const;

        /**
        * Returns the Task Allocator
        */
        AbsTaskAllocation *getTaskAllocator() const;

    private:

        /**
         * Creates a CPU and the related scheduler using the factories
        */
        void createCPUFromFactory(int index);

        /**
         * Creates n CPUs using the factories
        */
        void internalConstructor(int n);


    };
} // namespace RTSim

#endif
