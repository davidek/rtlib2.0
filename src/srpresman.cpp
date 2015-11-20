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
#include <srpresman.hpp>
#include <srpsched.hpp>
#include <abskernel.hpp>
#include <resource.hpp>
#include <task.hpp>
#include <waitinstr.hpp>
#include <sstream>

namespace RTSim {

    int SystemCeilingChangedEvt::systemCeiling() const {
        return _resman->systemCeiling();
    }


    SRPResManager::SRPResManager(const string &name)
        : ResManager(name), _ceilingChangedEvt(this)
    {}
 
    SRPResManager::~SRPResManager()
    {
        _ceilings.clear();
        while (! _system_ceilings.empty()) {
            _system_ceilings.pop();
            _ceiling_lockers.pop();
        }
    }

    void SRPResManager::newRun() {}

    void SRPResManager::endRun() {}

    void SRPResManager::addResource(const std::string &name, int n)
    {
        ResManager::addResource(name, n);
        Resource *r = _findRes(name);
        if (_ceilings.count(r))
            throw SRPResourceExc("addResource: Resource already exists!");
        _ceilings[r] = 0;
    }

    void SRPResManager::updateCeiling(const std::string &resname, int lvl)
    {
        if (lvl < 0)
            throw SRPResourceExc("Resource ceilings must be non-negative.");

        Resource *r = _findRes(resname);
        _ceilings[r] = max<int>(_ceilings[r], lvl);
    }
 
    bool SRPResManager::request(AbsRTTask *t, Resource *r, int n) 
    { 
        DBGENTER(_SRP_RES_MAN_DBG_LEV);

        DBGPRINT_6("Task ", dynamic_cast<MetaSim::Entity*>(t)->getName(),
                " requesting ", n, " instance(s) of ", r->getName());  

        if (r->isLocked()) {
            std::ostringstream msg;
            msg << "SRP invariant not satisfied: found locked resrouce "
                << r->getName();
            throw SRPResourceExc(msg.str());
        }
        r->lock(t);

        //  NO!! int new_ceiling = std::max<int>(_ceilings.at(r), systemCeiling());
        _system_ceilings.push(_ceilings.at(r));
        _ceiling_lockers.push(t);
        _ceilingChangedEvt.post(SIMUL.getTime());

        return true;
    }

    void SRPResManager::release(AbsRTTask *t, Resource *r, int n) 
    { 
        DBGENTER(_SRP_RES_MAN_DBG_LEV);

        std::ostringstream msg;
        msg << "Task " << dynamic_cast<MetaSim::Entity*>(t)->getName()
          << " unlocking " << n << " instance(s) of " << r->getName();
        DBGPRINT(msg.str());

        if ((!r->isLocked()) || _system_ceilings.empty()) {
            throw SRPResourceExc(
                    "Invalid resource nesting detected (too many releases)!");
        }
        r->unlock();
        _system_ceilings.pop();
        _ceiling_lockers.pop();
        _ceilingChangedEvt.post(SIMUL.getTime());
    }

    Resource *SRPResManager::_findRes(const std::string name)
    {
        Resource *r = dynamic_cast<Resource *>( Entity::_find(name) );
        if (r == NULL) {
            std::ostringstream msg;
            msg << "Resource not found: " << name << ".";
            throw SRPResourceExc(msg.str());
        }
        return r;
    }

    int SRPResManager::systemCeiling() const
    {
        if (_system_ceilings.empty())
            return 0;
        else
            return _system_ceilings.top();
    }

    AbsRTTask *SRPResManager::systemCeilingLocker() const
    {
        if (_ceiling_lockers.empty())
            return NULL;
        else
            return _ceiling_lockers.top();
    }

    void SRPResManager::ceilingsFromTask(AbsRTTask *t) {
        SRPBaseScheduler * sched = dynamic_cast<SRPBaseScheduler *>(_sched);
        if (sched == NULL)
            throw SRPResourceExc("SRPResManager was not given an SRP scheduler");
        ceilingsFromTask(sched->srpfind(t));
    }

    void SRPResManager::ceilingsFromTask(SRPBaseModel *srpt) {
        DBGENTER(_SRP_RES_MAN_DBG_LEV);
        TaskModel *tm = srpt->_asTaskModel();
        Task *t = dynamic_cast<Task *>(tm->getTask());
        if (t == NULL)
            throw SRPResourceExc("ceilingsFromTask argument must be a Task");
        typedef std::vector< Instr * > InstrList;
        typedef std::vector< Instr * >::const_iterator  ConstInstrIT;

        InstrList instructions = t->getInstrQueue();
        for (ConstInstrIT it=instructions.begin(); it<instructions.end(); it++){
            if (WaitInstr *wi = dynamic_cast<WaitInstr *>(*it)) {
                DBGPRINT_6("Task ", t->getName(),
                        " with preemption level ", srpt->getPreemptionLevel(),
                        " uses resource ", wi->getResource());
                updateCeiling(wi->getResource(), srpt->getPreemptionLevel());
            }
            //else if (SignalInstr *si = dynamic_cast<SignalInstr *>(*it)) {
            //    DBGPRINT_6("Task ", t->getName(),
            //            " with preemption level ", srpt->getPreemptionLevel(),
            //            " signals resource ", si->getResource());
            //}
        }
    }
}
