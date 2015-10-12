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

#ifndef SERVERVM_H
#define SERVERVM_H

#include "virtualmachine.hpp"

namespace RTSim{

using namespace MetaSim;
class Server;


/**
    Exception class that is thrown in case of access to a not defined
    VM implementation (null server object)

    @author Casini Daniel
*/
class UndefinedVMImplementationException : public BaseExc
{
public:
    inline UndefinedVMImplementationException(const string message, const string cl = "PeriodicServerVM", const string md = "RT_SIM") : BaseExc(message, cl, md) {}
};

/**
    PeriodicServerVM is an implementation of the abstract class Virtual Machine.
    It implements a VM with a PeriodicServer

    @author Casini Daniel
*/
class PeriodicServerVM : public VirtualMachine
{

public:

    /**
        Periodic Server Virtual Machine costructor
    */
    PeriodicServerVM();
    ~PeriodicServerVM();

    /**
        Initializes all the VM Parameter, with budget and period as input
    */
    void createFromQP(string name, Tick budget, Tick period, bool highestPriority = false);

    /*
        Initializes all the VM Parameter, with bandwidth e maximum delay as input

    void createFromAlphaDelta(string name, float alpha, float delta, bool highestPriority = false);
    */

    /**
        Returns the budget
    */
    Tick getBudget() const;

    /**
        Returns the budget
    */
    void setBudget(const Tick &value);

    /**
        Returns the period
    */
    Tick getPeriod() const;

    /**
        Sets the period
    */
    void setPeriod(const Tick &value);

    /**
        Returns the bandwidth
    */
    float getBandwidth() const;

    /**
        Sets the bandwidth
    */
    void setBandwidth(float value);

    /**
        Returns maximum delay
    */
    float getMaxDelay() const;

    /**
        Sets maximum delay
    */
    void setMaxDelay(float value);

    /**
        Returns the server that implements the VM
    */
    Server *getImplementation();

    /**
        Sets the server that implements the VM
    */
    void setImplementation(Server *value);

    /**
        Inherited (and implemented) from Virtual Machine, adds a task
    */
    virtual void addTask(AbsRTTask &task, const string &params);


private:
    /// Virtual Machine budget
    Tick    _budget;

    /// Virtual Machine Period
    Tick    _period;

    /// Virtual Machine bandwidth (alpha parameter)
    float   _bandwidth;

    /// Virtual Machine maximum delay (delta parameter)
    float   _maxDelay;

    /// Virtual Machine implementation as Periodic Server
    Server  *_implementation;

};


} //namespace RTSim

#endif // SERVERVM_H
