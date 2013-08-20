/* +---------------------------------------------------------------------------+
   |          The Mobile Robot Programming Toolkit (MRPT) C++ library          |
   |                                                                           |
   |                   http://mrpt.sourceforge.net/                            |
   |                                                                           |
   |   Copyright (C) 2005-2009  University of Malaga                           |
   |                                                                           |
   |    This software was written by the Machine Perception and Intelligent    |
   |      Robotics Lab, University of Malaga (Spain).                          |
   |    Contact: Jose-Luis Blanco-Claraco  <jlblanco@ctima.uma.es>                     |
   |                                                                           |
   |  This file is part of the MRPT project.                                   |
   |                                                                           |
   |     MRPT is free software: you can redistribute it and/or modify          |
   |     it under the terms of the GNU General Public License as published by  |
   |     the Free Software Foundation, either version 3 of the License, or     |
   |     (at your option) any later version.                                   |
   |                                                                           |
   |   MRPT is distributed in the hope that it will be useful,                 |
   |     but WITHOUT ANY WARRANTY; without even the implied warranty of        |
   |     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         |
   |     GNU General Public License for more details.                          |
   |                                                                           |
   |     You should have received a copy of the GNU General Public License     |
   |     along with MRPT.  If not, see <http://www.gnu.org/licenses/>.         |
   |                                                                           |
   +---------------------------------------------------------------------------+ */

#include <windows.h>
#include "CTicTac.h"


// Macros for easy access to memory with the correct types:
#define	LARGE_INTEGER_NUM(i)  largeInts[i]

/*---------------------------------------------------------------
						Constructor
 ---------------------------------------------------------------*/
CTicTac::CTicTac()
{
	QueryPerformanceFrequency( & LARGE_INTEGER_NUM(0) );
	Tic();
}

/*---------------------------------------------------------------
						Destructor
 ---------------------------------------------------------------*/
CTicTac::~CTicTac()
{
}

/*---------------------------------------------------------------
						Tic
	Starts the stopwatch
 ---------------------------------------------------------------*/
void	CTicTac::Tic()
{
	QueryPerformanceCounter( &LARGE_INTEGER_NUM(1) );
}

/*---------------------------------------------------------------
						Tac
   Stop. Returns ellapsed time in seconds
 ---------------------------------------------------------------*/
double	CTicTac::Tac()
{
	QueryPerformanceCounter( &LARGE_INTEGER_NUM(2) );
	return (LARGE_INTEGER_NUM(2).QuadPart-LARGE_INTEGER_NUM(1).QuadPart)/static_cast<double>(LARGE_INTEGER_NUM(0).QuadPart);
}
