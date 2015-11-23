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
#ifndef __APASCHED_HPP__
#define __APASCHED_HPP__

#include <scheduler.hpp>
#include <cpu.hpp>

#define _APASCHED_DBG_LVL "APAScheduler"

namespace RTSim {

    using namespace MetaSim;

    class APASchedExc : public BaseExc
    {
        public:
            APASchedExc(
                    const string message,
                    const string cl="APAScheduler",
                    const string md="apasched.cpp")
                : BaseExc(message, cl, md)
            {}
    };

    class AbsSchedulerFactory;


    /**
     * A set of CPU indexes where a task can run.
     */
    class Affinity: public std::set<int>
    {
    public:
        /// Initialize an affinity set given a CPU mask
        Affinity(uint64_t mask) {
            for (int i=0; i<64; i++) {
                if (mask & 1)
                    insert(i);
                mask >>= 1;
            }
        }
    };

    /**
       TaskModel for the APA scheduler.

       Because the APA scheduler relies on an inner scheduler for queuing,
       this taskmodel does not define task priority.
    */
    class APAModel: public TaskModel
    {
    protected:
        Affinity _affinity;

    public:
        APAModel(AbsRTTask* t, uint64_t mask)
            : TaskModel(t), _affinity(mask)
        {}
        APAModel(AbsRTTask* t, Affinity affinity)
            : TaskModel(t), _affinity(affinity)
        {}

        const Affinity &getAffinity() const { return _affinity; }

        //bool allowedOn(int cpu_index) {
        //    return _affinity.count(cpu_index);
        //}
        bool allowedOn(CPU *cpu) {
            return _affinity.count(cpu->getIndex());
        }

        /// Not implemented: throws an exception
        Tick getPriority() { throw APASchedExc("Not implemented"); }
        /// Not implemented: throws an exception
        void changePriority(Tick p) { throw APASchedExc("Not implemented"); }
    };


    /**
       @brief Scheduler with Arbitrary Processor Affinity

       Sheduler with Arbitrary Processor Affinity,
       to be used with the APA kernel.

       This implementation refers to the one described in:

       Gujarati, Cerqueira, Brandenburg,  Schedulability Analysis of the
       Linux Push and Pull Scheduler with Arbitrary Processor Affinities,
       ECRTS 2013
       
       The main difference w.r.t. the paper is that some push/pull operations
       are first "scheduled" (when the scheduler realizes that a higher-priority
       task is queuing) and are performend when the kernel `notify`s
       the scheduler that the context switch occurred.  This is an artifact
       of the Scheduler public API which is based on ad idempotent `getFirst`
       operation and a successive `notify` whenever a task is [de]scheduled
    */
    class APAScheduler: public Scheduler
    {
    protected:
        typedef map<CPU *, Scheduler *>::iterator SCHEDIT;

        /// map cpu  to  instance of InnerScheduler
        std::map<CPU *, Scheduler *> _inner_sched;

        /// Map task  to  cpu which is currently handling it
        std::map<AbsRTTask *, CPU *> _task_cpu;

        /// Factory for the inner schedulers
        AbsSchedulerFactory *_inner_factory;

        /// Priority of the latest task that ran on each processor
        /// Negative values means no processor was running
        std::map<CPU *, Tick> _latest_prio;

    private:
        /// Used for checking that CPU ids (aka indexes) are unique
        std::set<int> _cpuids;

        /// Keep track of pending push() operations
        //std::map<CPU *, bool> _pending_push;

    public:
        // ****** Constructors and initialization

        APAScheduler() {}

        APAScheduler(AbsSchedulerFactory *inner_factory)
            : _inner_factory(inner_factory)
        {}

        virtual void setInnerSchedFactory(AbsSchedulerFactory *inner_factory) {
            _inner_factory = inner_factory;
        }

        /**
         * @param params String composed of:
         *   affinity mask, whitespace, params for inner scheduler
         */
        virtual void addTask(AbsRTTask* task, const std::string &params); 

        /**
         * @param other_params params for inner scheduler
         */
        virtual void addTask(AbsRTTask* task, Affinity affinity,
                const std::string &other_params = ""); 

        virtual void removeTask(AbsRTTask* task);

        virtual void addCPU(CPU *c);

        // ****** Run-time functionalities

        /**
         * Insert a task in the queue for the processor handling it.
         */
        virtual void insert(AbsRTTask *) throw(RTSchedExc, BaseExc);

        /**
         * Extract a task from the queue of the processor handling it.
         */
        virtual void extract(AbsRTTask *) throw(RTSchedExc, BaseExc);

        /**
         * Throws an exception, only the multiprocessor version is supported.
         * @TODO could be implemented if there is a single CPU, but I'm not
         * sure it would be useful
         */
        virtual AbsRTTask *getFirst() {
            throw APASchedExc("Multiprocessor-unaware API is not supported");
        }
  
        /**
         * Throws an exception, only the multiprocessor version is supported.
         * @TODO could be implemented if there is a single CPU, but I'm not
         * sure it would be useful
         */
        virtual AbsRTTask * getTaskN(unsigned int) {
            throw APASchedExc("Multiprocessor-unaware API is not supported");
        }

        /**
         * Get the first task for the given cpu. Might trigger a pull operation.
         */
        virtual AbsRTTask *getFirst(CPU *c);

        /**
         * Get called when a dispatch was performed.
         * May trigger a push, in which case a new dispatch will be triggered
         * on the target processor.
         */
        virtual void notify(AbsRTTask *task);

    protected:
        // ****** Initialization

        /// Internal helper for addTask, extended to handle inner schedulers
        virtual void enqueueModel(
                TaskModel* model, const std::string &other_params);

        // ****** Run-time functionalities

        /// perform scheduled push or pull operations on a processor
        //virtual void pushPullRun(int cpuid);

        /// run push operation from processor `cpuid` for the first
        /// non-running task
        virtual void push(CPU *c);

        /// run pull operation targeting processor `cpuid`
        virtual void pull(CPU *c);

        // ******* Other helpers

        /// Like find but return APAModel (and throw exception if not found)
        virtual APAModel *apafind(AbsRTTask *task);

        /// Perform some sanity checks, throw exception on failure.
        /// No effect unless compiled wiht __DEBUG__ defined
        virtual void invariant();
    };


} // namespace RTSim

#endif
