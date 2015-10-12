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

#ifndef VIRTUALMACHINE_H
#define VIRTUALMACHINE_H

#include <string>
#include <abstask.hpp>

namespace RTSim{

    /**
        This is the virtual base class representing a virtual machine.
        It is supposed to be represented by a name.
        Possible implementations of a VM should derive this class, that is
        quite general to be compatible with both VMs implemented as
        periodic server and static time partioning

        @author Casini Daniel
    */
    class VirtualMachine
    {
    public:

        /**
            Virtual Machine Constructor
        */
        VirtualMachine();

        /**
            Adds a task to the virtual machine
        */
        virtual void addTask(AbsRTTask &task, const string &params) = 0;

        /**
            Returns the VM name
        */
        string getName() const;

        /**
            Sets the VM name
        */
        void setName(const string &value);

    private:
        /// Virtual Machine name
        string name;

    };


}

#endif // VIRTUALMACHINE_H

