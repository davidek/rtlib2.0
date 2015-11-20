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
#ifndef __SRPSCHED_HPP__
#define __SRPSCHED_HPP__

#include <scheduler.hpp>
#include <edfsched.hpp>
#include <fpsched.hpp>
#include <srpresman.hpp>

namespace RTSim {

    using namespace MetaSim;

    class SRPBaseScheduler;
    class EDF_SRPScheduler;
    class FP_SRPScheduler;

    class SRPSchedExc : public BaseExc
    {
    public:
        SRPSchedExc(
                const string message,
                const string cl="SRPScheduler",
                const string md="srpsched.cpp")
            : BaseExc(message, cl, md)
        {}
    };

    /**
     * @brief Mixin class for deriving SRP models
     * @author Davide Kirchner
     */
    class SRPBaseModel
    {
    private:
        /** Static preemption level */
        const int _preemption_level;

    public:
        SRPBaseModel(int preemption_level)
            : _preemption_level(preemption_level)
        {}

        virtual int getPreemptionLevel() {
            return _preemption_level;
        }

        /**
         * Cast this to a TaskModel.
         */
        virtual TaskModel *_asTaskModel();
    };


    /**
     * @brief Task model for EDF with SRP support.
     * @author Davide Kirchner
     */
    class EDF_SRPModel: public EDFModel, public SRPBaseModel {
    public:
        EDF_SRPModel(AbsRTTask *t, int preemption_level)
          : EDFModel(t), SRPBaseModel(preemption_level)
        {}
    };


    /** \ingroup sched
     * @brief Mixin class for deriving SRP schedulers
     *
     * Mixin class for deriving SRP schedulers.
     *
     * @author Davide Kirchner
     */
    class SRPBaseScheduler
    {
    private:
        SRPResManager *_resman;
    public:
        SRPBaseScheduler(): _resman(NULL) {}
        virtual ~SRPBaseScheduler() {}

        /**
         * The SRP Scheduler needs access to the (subclass of) SRPResManager
         * in use.
         */
        virtual void setResManager(SRPResManager *resman) { _resman = resman; }
        virtual int systemCeiling() {
            if (!_resman)
                throw SRPSchedExc("You must call setResManager() before using an SRP-aware scheduler");
            return _resman->systemCeiling();
        }

        /** @brief Return this, casted to a Scheduler.
         *
         * Concrete subclasses should return themselves, as they are schedulers
         */
        virtual Scheduler *_asScheduler() = 0;

        /**
         * Override default, implementing the preemption level check
         */
        virtual AbsRTTask *getFirst();

    protected:
        /** Run getFirst() as in the scheduler the subclass is deriving from. */
        virtual AbsRTTask *getFirstOrig() = 0;

        /** Return the current running task, proxying from Scheduler */
        virtual AbsRTTask *_getCurrExe() = 0;

        friend class SRPResManager;
        /// properly-casted find() method
        virtual SRPBaseModel *srpfind(AbsRTTask *t) = 0;
    };


    /** \ingroup sched
     *
     * @brief An EDF scheduler with SRP support.
     *
     * @see EDFScheduler
     *
     * @author Davide Kirchner
     */
    class EDF_SRPScheduler: public EDFScheduler, public SRPBaseScheduler
    {
    public:
        /**
         * Implements pure virtual rasing an exception.
         * Doesn't actually make sense.
         */
        virtual void addTask(AbsRTTask *t) {
            throw(SRPSchedExc("Use the parametrized version of addTask"));
        }

        /**
         * @brief Add a task to the scheduler specifying its preemption level.
         *
         * @param params A string containing the preemption level, parsable to
         *               an integer.
         */
        virtual void addTask(AbsRTTask *t, const std::string &params);

        /**
         * @brief Add a task to the scheduler specifying its preemption level.
         */
        virtual void addTask(AbsRTTask *task, int preemption_level);

        virtual void removeTask(AbsRTTask *task);


        virtual EDF_SRPScheduler *_asScheduler() {
            return this;
        }

        virtual AbsRTTask *getFirst() {
            return SRPBaseScheduler::getFirst();
        }

    protected:
        virtual AbsRTTask *getFirstOrig() {
            return EDFScheduler::getFirst();
        }

        virtual SRPBaseModel *srpfind(AbsRTTask *t) {
            if (t == NULL)
                throw SRPSchedExc("Got a request for the model of NULL...");
            SRPBaseModel *ret = dynamic_cast<SRPBaseModel *>(find(t));
            if (ret == NULL)
                throw SRPSchedExc("A task model is not an SRPBaseModel");
            return ret;
        }

        virtual AbsRTTask *_getCurrExe() { return _currExe; }
    };


    /** \ingroup sched
     *
     * @brief A fixed priority scheduler with SRP support.
     *
     * @todo This is untested, but should give an idea of how to port SRP to the FP scheduler
     */
    class FP_SRPScheduler: public FPScheduler, public SRPBaseScheduler {
    protected:
        /**
         * \ingroup sched
         *
         * @brief Task model for FP with SRP support.
         */
        class FP_SRPModel: public FPScheduler::FPModel, public SRPBaseModel {
        public:
            FP_SRPModel(AbsRTTask *t, Tick prio, int preemption_level)
                : FPModel(t, prio), SRPBaseModel(preemption_level)
            {}
        };

    public:
        /**
         * Implements pure virtual rasing an exception.
         * Doesn't actually make sense.
         */
        virtual void addTask(AbsRTTask *t) {
            throw(SRPSchedExc("Use the parametrized version of addTask"));
        }

        /**
         * @brief Add a task, specifying priority and preemption level.
         *
         * @param params A string  of two integer comma-separated values:
         *               priotity and preemption level.
         */
        virtual void addTask(AbsRTTask *t, const std::string &params);

        /**
         * @brief Add a task, specifying priority and preemption level.
         * @TODO not implemented yet
         */
        virtual void addTask(
                AbsRTTask *task, Tick priority, int preemption_level) {}

        virtual void removeTask(AbsRTTask *task) {} // TODO


        virtual FP_SRPScheduler *_asScheduler() {
            return this;
        }

        virtual AbsRTTask *getFirst() {
            return SRPBaseScheduler::getFirst();
        }

    protected:
        virtual AbsRTTask *getFirstOrig() {
            return FPScheduler::getFirst();
        }

        virtual SRPBaseModel *srpfind(AbsRTTask *t) {
            if (t == NULL)
                throw SRPSchedExc("Got a request for the model of NULL...");
            SRPBaseModel *ret = dynamic_cast<SRPBaseModel *>(find(t));
            if (ret == NULL)
                throw SRPSchedExc("A task model is not an SRPBaseModel");
            return ret;
        }

        virtual AbsRTTask *_getCurrExe() { return _currExe; }
    };

} // namespace RTSim

#endif
