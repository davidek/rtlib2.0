/***************************************************************************
    begin                : Tue Sep 15 15:54:58 CEST 2015
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
#ifndef __REPLENISHMENTSERVER_HPP__
#define __REPLENISHMENTSERVER_HPP__

#include <server.hpp>
#include <serverevt.hpp>
#include <capacitytimer.hpp>
#include <list>

//#define _SERVER_DBG_LEV "repl_server"

namespace RTSim {

    using namespace MetaSim;
    /** 
        @ingroup replenishment_server

        Implements an interface used by the servers with a replenishment event
        different from the "recharging" owned by the server base class
        @see Server, CBServer, SporadicServer

        @author Daniel Casini
    */
    class ReplenishmentServer : public Server{
    protected: 
        friend class ServerReplenishmentEvt;
        friend class ServerIdleEvt;

        /// Budget (full) and period of the server
        Tick Q, P;

        /// Current budget of the server
        Tick cap;

        Tick last_time;
        Tick recharging_time;

        /// replenishment: it is a pair of <t,b>, meaning that
        /// at time t the budget should be replenished by b.
        typedef std::pair<Tick, Tick> repl_t;

        /// queue of replenishments
        /// all times are in the future!
        std::list<repl_t> repl_queue;

        /// at the replenishment time, the replenishment is moved
        /// from the repl_queue to the capacity_queue, so
        /// all times are in the past.
        std::list<repl_t> capacity_queue;

        CapacityTimer vtime;

    public:
        /// A new event replenishment, different from the general
        /// "recharging" used in the Server class
        ServerReplenishmentEvt _replEvt;

        /// when the server becomes idle
        ServerIdleEvt _idleEvt;

        /// Replenishment Server constructor
        ReplenishmentServer(const string &name, const string &s, Tick q, Tick p) : Server(name, s),
            Q(q), P(p), cap(0), last_time(0),recharging_time(0),
            repl_queue(), capacity_queue(), vtime(), _replEvt(this), _idleEvt(this){}

        /// Returns the server budget
        virtual Tick getBudget() const { return Q;}

        /// Returns the server period
        virtual Tick getPeriod() const { return P;}

        /// Replenishment event handler
        virtual void onReplenishment(Event *e) = 0;

        /// Idle event handler
        virtual void onIdle(Event *e) = 0;

        /// Returns the current budget
        virtual Tick getCurrentBudget() const { return cap;}

        /// inherited from task
        virtual Tick getMaxExecutionTime() const { return Q;} //budget
    };


} // namespace RTSim

#endif
