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

namespace RTSim {

    using namespace MetaSim;

    /** \ingroup sched
     * @brief Mixin class for deriving SRP models
     *
     * @author Davide Kirchner
     */
    class SRPBaseModel
    {
    private:
        /** Static preemption level */
        const int _preemption_level;

        /**
         * When task is running, points to the current system ceiling;
         * NULL otherways
         */
        const int *_system_ceiling;

    public:
        SRPBaseModel(int preemption_level)
            : _preemption_level(preemption_level), _system_ceiling(NULL) {}

        virtual int getPreemptionLevel() const {
            return _preemption_level;
        }

        /** @brief Notify the task that it started runnig
         *
         * @param ceiling Pointer to the System Ceiling
         */
        virtual void setRunning(const int *ceiling) {
            _system_ceiling = ceiling;
        }

        /** @brief Notify the task it's not running any more */
        virtual void setNotRunning() {
            _system_ceiling = NULL;
        }
    };


    /** \ingroup sched
     *
     * @brief Task model for EDF with SRP support.
     *
     * @author Davide Kirchner
     */
    class EDF_SRPModel: public EDFModel, public SRPBaseModel {
    public:
        EDF_SRPModel(AbsRTTask *t, int preemption_level)
            : EDFModel(t), SRPBaseModel(preemption_level)
        {}

        virtual int getPreemptionLevel() const {
            return SRPBaseModel::getPreemptionLevel();
        }
    };



    /** \ingroup sched
     *
     * @brief An EDF scheduler with SRP support.
     *
     * @see EDFScheduler
     *
     * @author Davide Kirchner
     */
    class EDF_SRPScheduler: public EDFScheduler
    {
    public:
        /**
         * Implements pure virtual rasing an exception.
         * Doesn't actually make sense.
         */
        virtual void addTask(AbsRTTask *t) {
            throw(RTSchedExc("Use the parametrized version of addTask"));
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
    };

    /** \ingroup sched
     *
     * @brief A fixed priority scheduler with SRP support.
     *
     * @todo This is untested, but should give an idea of how to port SRP to the FP scheduler
     */
    class FP_SRPSheduler: public FPScheduler {
    protected:
        /**
         * \ingroup sched
         *
         * @brief Task model for FP with SRP support.
         */
        class FP_SRPModel: public FPScheduler::FPModel, public SRPBaseModel {
        public:
            FP_SRPModel(AbsRTTask *t, Tick prio, int preemption_level)
                : FPScheduler::FPModel(t, prio), SRPBaseModel(preemption_level)
            {}

            virtual int getPreemptionLevel() const {
                return SRPBaseModel::getPreemptionLevel();
            }
        };

    public:
        FP_SRPSheduler() {}
    };

} // namespace RTSim

#endif
