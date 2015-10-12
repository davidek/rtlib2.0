/***************************************************************************
 *   begin                : Sun Aug 15 17:28:58 CEST 2015
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
#ifndef __ABSTASKALLOCATION_HPP__
#define __ABSTASKALLOCATION_HPP__

#include <vector>

#include <kernel.hpp>
#include <kernevt.hpp>

namespace RTSim {

	class AbsTaskAllocation
    {
		public:
			virtual ~AbsTaskAllocation() {}
			void operator(PartionedRTKernel &kern) = 0;
		
	}
	
	class NextFitTaskAllocation
    {

		
		public:
			//~AbsTaskAllocation() {}
			void operator(PartionedRTKernel &kern);
	}
	
}