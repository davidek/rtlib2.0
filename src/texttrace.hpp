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
#ifndef __TEXTTRACE_HPP__
#define __TEXTTRACE_HPP__

#include <fstream>
#include <iostream>
#include <string>

#include <baseexc.hpp>
#include <basetype.hpp>
#include <event.hpp>
#include <particle.hpp>
#include <trace.hpp>

#include <rttask.hpp>
#include <taskevt.hpp>
#include <serverevt.hpp>

#include <replenishmentserver.hpp>
#include <periodicservervm.hpp>

namespace RTSim {
    using namespace std;
    using namespace MetaSim;
    
    class TextTrace {
    protected:
        ofstream fd;
    public:
        TextTrace(const string& name);
        
        ~TextTrace();
        
        void probe(ArrEvt& e);
        
        void probe(EndEvt& e);
        
        void probe(SchedEvt& e);
        
        void probe(DeschedEvt& e);
        
        void probe(DeadEvt& e);

        void probe(ServerBudgetExhaustedEvt& e);

        void probe(ServerRechargingEvt& e);

        void probe(ServerScheduledEvt& e);

        void probe(ServerDescheduledEvt& e);

        void probe(ServerReplenishmentEvt& e);
        
        void attachToTask(Task* t);

        void attachToServer(Server* s);

        void attachToPeriodicServerVM(PeriodicServerVM* s);

    };
    
    class VirtualTrace {
        map<string, int> *results;
    public:
        
        VirtualTrace(map<string, int> *r);
        
        ~VirtualTrace();
        
        void probe(EndEvt& e);
        
        void attachToTask(Task* t);
    };
}

#endif
