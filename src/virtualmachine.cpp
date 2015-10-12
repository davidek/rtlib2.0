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

#include <virtualmachine.hpp>


namespace RTSim{

VirtualMachine::VirtualMachine()
{

}

string VirtualMachine::getName() const
{
    return name;
}

void VirtualMachine::setName(const string &value)
{
    name = value;
}



}//namespace
