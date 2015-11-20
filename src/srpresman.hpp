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
#ifndef __SRPRESMAN_HPP__
#define __SRPRESMAN_HPP__

//#include <deque>
#include <map>
#include <stack>

#include <resmanager.hpp>

#define _SRP_RES_MAN_DBG_LEV "SRPResManager"

namespace RTSim {

    class AbsRTTask;
    class Task;
    class SRPBaseModel;
    class SRPResManager;

    /**
     * Event that records a change in the system ceiling.
     * 
     * @todo add a getCPU() (or similar) method
     *   (and attribute where to store CPU id or a pointer to it)
     */
    class SystemCeilingChangedEvt: public MetaSim::Event {
    private:
        SRPResManager *_resman;
    public:
        SystemCeilingChangedEvt(SRPResManager *resman)
            : _resman(resman)
        {}

        /// Return the _current_ system ceiling (useful in event handlers).
        int systemCeiling() const;

        void doit() {}
    };

    class SRPResourceExc : public BaseExc
    {
    public:
        SRPResourceExc(
                const string message,
                const string cl="SRPResourceManager",
                const string md="srpresman.cpp")
            : BaseExc(message, cl, md)
        {}
    };


    /** \ingroup resman
     *
     * @brief Resource manager implementing the Stack Resource Policy (SRP)
     *
     * @see Resource 
     * @author Davide Kirchner
    */
    class SRPResManager : public ResManager {
    public:
        SRPResManager(const string &n = "");
        virtual ~SRPResManager();
 
        virtual void addResource(const std::string &name, int n=1);

        /** @brief Set ceiling = max(ceiling, lvl) for the given resource
         *
         * @param resname Resource name
         * @param lvl Preemption level to be stored as ceiling,
         *            if greater than the current one
         */
        virtual void updateCeiling(const std::string &resname, int lvl);

        /** @brief Return the current system ceiling
         * Note the system ceiling is 0 when no resource is locked
         */
        virtual int systemCeiling() const;

        /** @brief Return the task that most recently increased the ceiling,
         *  @brief or NULL if no task has locked resources.
         */
        virtual AbsRTTask *systemCeilingLocker() const;

        virtual void newRun();
        virtual void endRun();
 
        /** @brief update resource ceilings by inspecting the give task */
        virtual void ceilingsFromTask(AbsRTTask *t);

        /** Instance of SystemCeilingChangedEvt that gets posted each time */
        SystemCeilingChangedEvt _ceilingChangedEvt;

    protected:
        virtual bool request(AbsRTTask*, Resource*, int n=1);
        virtual void release(AbsRTTask*, Resource*, int n=1); 

        /** @brief Gets a resource object by name
         * Throws an exception if such resource does not exist.
         */
        Resource *_findRes(const std::string name);

        /** @brief Preemption level ceiling for each resource */
        std::map<Resource *, int> _ceilings;

        /** @brief Stack of system ceilings: current one is the top element */
        std::stack<int> _system_ceilings;

        /** @brief Parallel stack to the _system_ceilings
         * Keeps track of the tasks which caused the system ceilings to rise
         */
        std::stack<AbsRTTask *> _ceiling_lockers;

        /** @brief update resource ceilings by inspecting the give task */
        virtual void ceilingsFromTask(SRPBaseModel *t);
    };

} // namespace RTSim 

#endif
