/***************************************************************************
    begin                : Tue Sep 15 18:07:04 CEST 2015
    copyright            : (C) 2015 by Daniel Casini
    email                : daniel.casini@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <server.hpp>
#include <serverevt.hpp>
#include <ReplenishmentServer.hpp>
#include <cstdlib>

namespace RTSim {
    
    void ServerBudgetExhaustedEvt::doit()
    {
        _server->onBudgetExhausted(this);
    }
    
    void ServerDMissEvt::doit()
    {
        _server->onDlineMiss(this);    }
    
    void ServerRechargingEvt::doit()
    {
        _server->onRecharging(this);
    }
    
    void ServerScheduledEvt::doit()
    {
        _server->onSched(this);
    }
    
    void ServerDescheduledEvt::doit()
    {
        _server->onDesched(this);
    }

    void ServerDispatchEvt::doit()
    {
        _server->onDispatch(this);
    }

    void ServerIdleEvt::doit()
    {
        ReplenishmentServer *rServ = dynamic_cast<ReplenishmentServer *> (_server);
        rServ->onIdle(this);
    }

    void ServerReplenishmentEvt::doit()
    {
        ReplenishmentServer *rServ = dynamic_cast<ReplenishmentServer *> (_server);
        rServ->onReplenishment(this);
    }

    
} // namespace RTSim
