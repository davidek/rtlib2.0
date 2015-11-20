/***************************************************************************
 begin                : Thu Apr 24 15:54:58 CEST 2003
 copyright            : (C) 2003 by Giuseppe Lipari
 email                : lipari@sssup.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef __JSONTRACE_HPP__
#define __JSONRACE_HPP__

#include <fstream>
#include <iostream>
#include <string>

#include <baseexc.hpp>
#include <basetype.hpp>
#include <event.hpp>
#include <particle.hpp>
#include <trace.hpp>

#include <task.hpp>
#include <rttask.hpp>

#include <exeinstr.hpp>
#include <waitinstr.hpp>
#include <suspend_instr.hpp>
#include <threinstr.hpp>
#include <schedinstr.hpp>

#include <replenishmentserver.hpp>

#include <srpresman.hpp>

namespace RTSim {


    class JSONTrace {
    protected:
        std::ofstream fd;
        bool first_event;

        void writeTaskEvent(TaskEvt& e, const std::string &evt_name, const std::string &resource="", const std::string &cl_name="");

        void writeServerEvent(const Server &s, const std::string &evt_name);
        void writeServerEvent(const ReplenishmentServer &s, const std::string &evt_name);
        void writeServerEventCPU(const Server &s, const std::string &evt_name, ServerEvt& e);
        void writeServerEventCPU(const ReplenishmentServer &s, const std::string &evt_name, ServerEvt& e);

        void _start();
        void _end();
        void _sep();
        void _pair(const std::string &key, const std::string &val);
        void _pair(const std::string &key, const MetaSim::Tick &val);
        void _time();

        void _task_info(const Task &t);
        void _cpu_num(int cpu);
    public:
        JSONTrace(const std::string& name);
        
        ~JSONTrace();
        

        void probe(Event &e);

        void probe(TaskEvt &e);
        void probe(ArrEvt& e);
        void probe(EndEvt& e);
        void probe(SchedEvt& e);
        void probe(DeschedEvt& e);
        void probe(DeadEvt& e);
        void probe(WaitEvt& e);
        void probe(SignalEvt& e);

        void probe(ServerBudgetExhaustedEvt& e);
        void probe(ServerDMissEvt& e);
        void probe(ServerRechargingEvt& e);
        void probe(ServerScheduledEvt& e);
        void probe(ServerDescheduledEvt& e);
        void probe(ServerReplenishmentEvt& e);
        
        void probe(EndInstrEvt &e);

        void probe(SystemCeilingChangedEvt &e);


        void attachToTask(Task* t);

        void attachToServer(Server* s);
        void attachToPeriodicServerVM(PeriodicServerVM* s);

        void attachToSRPResMan(SRPResManager *resman);
        
        void attachToInstr(Instr *i);
    };
}

#endif
