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
#ifndef __SERVEREVT_HPP__
#define __SERVEREVT_HPP__

#include <event.hpp>


namespace RTSim {

using namespace MetaSim;

class Server;
class ReplenishmentServer;

/**
   \ingroup server

   This class is the base class for all server events.
   @note The server events have been transformed from the "GEvent template class mode"
   in the "class hierarchy mode" in order to expand the tracer, during the VM implementation

   @see Server, ReplenishmentServer

   @author Daniel Casini
*/
class ServerEvt : public MetaSim::Event
{
protected:

    /// Server to which the event refers
    Server* _server;

    /// CPU in which the event happens
    int _cpu;

public:
    ServerEvt(Server* t, int p = _DEFAULT_PRIORITY) :
        MetaSim::Event(p), _cpu(-1)  {_server = t;}

    /// Returns the server
    Server* getServer() const {return _server;}

    /// Sets the server
    void setServer(Server* t) {_server = t;}

    /// Returns the CPU
    int getCPU() {return _cpu;}

    /// Sets the CPU
    void setCPU(int cpu) {_cpu = cpu;}
};

/**
   \ingroup server

   This class is the base class for all replenishment server events.
   A "replenishment server" is a server that derives from this abstract class and then
   implements the replenishment and idle events.

   @see Server, ReplenishmentServer, CBServer, SporadicServer

   @author Daniel Casini
*/
class ReplenishmentServerEvt : public MetaSim::Event
{
protected:

    /// Server to which the event refers
    ReplenishmentServer* _server;

    /// CPU in which the event happens
    int _cpu;

public:
    ReplenishmentServerEvt(ReplenishmentServer* t, int p = _DEFAULT_PRIORITY) :
        MetaSim::Event(p), _cpu(-1)  {_server = t;}

    /// Returns the server
    ReplenishmentServer* getServer() const {return _server;}

    /// Sets the server
    void setServer(ReplenishmentServer* t) {_server = t;}

    /// Returns the CPU
    int getCPU() {return _cpu;}

    /// Sets the CPU
    void setCPU(int cpu) {_cpu = cpu;}
};

/// Budget exhausted event for a server
/// \ingroup server
class ServerBudgetExhaustedEvt: public ServerEvt
{
public:
    ServerBudgetExhaustedEvt(Server* s) :ServerEvt(s, Event::_DEFAULT_PRIORITY + 4) {}
    virtual void doit();

};

/// Deadline miss event for a server
/// \ingroup server
class ServerDMissEvt: public ServerEvt
{
public:
    ServerDMissEvt(Server* s) :ServerEvt(s, Event::_DEFAULT_PRIORITY + 6) {}
    virtual void doit();

};

/// Recharging event for a server
/// \ingroup server
class ServerRechargingEvt: public ServerEvt
{
public:
    ServerRechargingEvt(Server* s) :ServerEvt(s, Event::_DEFAULT_PRIORITY - 1) {}
    virtual void doit();

};

/// Scheduling server event
/// \ingroup server
class ServerScheduledEvt: public ServerEvt
{
public:
    ServerScheduledEvt(Server* s) :ServerEvt(s) {}
    virtual void doit();

};

/// Descheduling server event
/// \ingroup server
class ServerDescheduledEvt: public ServerEvt
{
public:
    ServerDescheduledEvt(Server* s) :ServerEvt(s) {}
    virtual void doit();

};

/// Dispatch server event
/// \ingroup server
class ServerDispatchEvt: public ServerEvt
{
public:
    ServerDispatchEvt(Server* s) :ServerEvt(s, Event::_DEFAULT_PRIORITY + 5) {}
    virtual void doit();

};

/// Scheduling idle event
/// \ingroup server
class ServerIdleEvt: public ReplenishmentServerEvt
{
public:
    ServerIdleEvt(ReplenishmentServer* s) : ReplenishmentServerEvt(s) {}
    virtual void doit();

};

/// Scheduling replenishment event
/// \ingroup server
class ServerReplenishmentEvt: public ReplenishmentServerEvt
{
public:
    ServerReplenishmentEvt(ReplenishmentServer* s) : ReplenishmentServerEvt(s, Event::_DEFAULT_PRIORITY - 1) {}
    virtual void doit();

};

} // namespace RTSim

#endif
