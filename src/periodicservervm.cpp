/***************************************************************************
 *   begin                : Tue Sep 15 17:28:58 CEST 2015
 *   copyright            : (C) 2015 by Daniel Casini
 *   email                : daniel.casini@gmail.com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "periodicservervm.hpp"
#include "server.hpp"
#include <math.h>

#define MAX(x, y)       (x > y) ? x : y
#define CEILING(x)      (int)ceil(x)
#define FLOOR(x)        (int)floor(x)

namespace RTSim {

PeriodicServerVM::PeriodicServerVM() : _implementation(0)
{
}


PeriodicServerVM::~PeriodicServerVM()
{
    if(_implementation)
        delete _implementation;
}

void PeriodicServerVM::createFromQP(string name, Tick budget, Tick period, bool highestPriority)
{
    setName(name);
    _budget = budget;
    _period = period;
    int factor = (highestPriority) ? 1 : 2;
    _bandwidth = ((int)period / (int)budget);
    _maxDelay = (factor * ((int)period - (int)budget));
}

/* TO DO, the reasoning below I think is wrong
 *
 * Note for the computation of P/Q from alpha/delta:
 * since alpha is the minimum required bandwith and delta
 * is the maximum delay (and it is allowed to specify
 * floating point alpha/delta values, but only integral P/Q
 * values) it is necessary to derive a version of the
 * P/Q <-> alpha/delta conversion formula with the following
 * features:
 *  Given alpha, delta as float, then it is possibile to derive:
 *      P*, Q*, obtained by rounding in some manner the conventional
 *      conversion formula
 *          Q = ((alpha * delta) / (factor * (1 - alpha)));
            P = (delta / (factor * (1 - alpha)));
        The "rounding" has to be done in a manner that allows to
        obtain alpha*, delta* (from the conventional conversion formula,
        specified below)
            alpha = (Q / P);
            delta = (factor * (P - (int)budget));
            (factor = 1 if the server has maximum priority,
            factor = 2 otherwise)
        such that
            alpha* > alpha
            delta* < delta
        With this consideration the modified formulas are:
            Q* = CEILING(Q)
            P* = max{Q*, FLOOR(P)
        where Q, P can be substituded (to be derived from alpha, delta)
        with the classical formulas above.
    void PeriodicServerVM::createFromAlphaDelta(string name, float alpha, float delta, bool highestPriority)
    {
        setName(name);
        setBandwidth(alpha);
        setMaxDelay(delta);
        int factor = (highestPriority) ? 1 : 2;
        float budget, period;
        budget = ((alpha * delta) / (factor * (1 - alpha)));
        period = (delta / (factor * (1 - alpha)));
        _budget = CEILING(budget);
        _period = MAX(_budget, FLOOR(period));
    }
 */


Tick PeriodicServerVM::getBudget() const
{
    return _budget;
}

void PeriodicServerVM::setBudget(const Tick &value)
{
    _budget = value;
}

Tick PeriodicServerVM::getPeriod() const
{
    return _period;
}

void PeriodicServerVM::setPeriod(const Tick &value)
{
    _period = value;
}

float PeriodicServerVM::getBandwidth() const
{
    return _bandwidth;
}

void PeriodicServerVM::setBandwidth(float value)
{
    _bandwidth = value;
}

float PeriodicServerVM::getMaxDelay() const
{
    return _maxDelay;
}

void PeriodicServerVM::setMaxDelay(float value)
{
    _maxDelay = value;
}

Server *PeriodicServerVM::getImplementation()
{
    if(!_implementation) throw UndefinedVMImplementationException("The virtual machine implementation is not specified");
        return _implementation;
}

void PeriodicServerVM::setImplementation(Server *value)
{
    _implementation = value;
}

void PeriodicServerVM::addTask(AbsRTTask &task, const string &params)
{
    if(!_implementation) throw UndefinedVMImplementationException("The virtual machine implementation is not specified");

    _implementation->addTask(task, params);
}

}
