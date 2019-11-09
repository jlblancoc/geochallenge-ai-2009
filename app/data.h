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
#ifndef  MY_DATA_H
#define  MY_DATA_H

#include "screen_capture.h"
#include "CTicTac.h"
#include "COCREngine.h"


enum TAIState {
	stWaitMainScreen = 0,
	stLevel1,
	stAfterLevel1Wait,
	stLevel2,
	stAfterLevel2Wait,
	stLevel3,
	stAfterLevel3Wait,
	stLevel4,
	stAfterLevel4Wait,
	stEnd
};

extern COCREngine		ocr;
extern CScreenCapture	capture;
extern TAIState			ai_state;
extern bool				be_slow;

extern CvFont			my_font1;

extern std::map<std::string,CvImage>	lstImgs;		//<! List of useful images

typedef int TFlagID;
typedef int TCountryID;

struct TLearnData
{
	struct TLevel1
	{
		struct TFlagInfo
		{
			CvImage  img;
			CvScalar img_mean; //!< Used to quickly discard matchings
			std::set<TCountryID> yes;
			std::set<TCountryID> no;
		};

		struct TCountryInfo
		{
			CvImage  img;
			CvScalar img_mean; //!< Used to quickly discard matchings
		};

		std::vector<TFlagInfo>    flags;  
		std::vector<TCountryInfo> country_names;

		// Save/load to disk. Return false on error.
		bool save() const;
		bool load();
	} l1;

	struct TLevel2
	{
		struct TShapeInfo
		{
			CvImage		img;
			CvScalar	img_mean; //!< Used to quickly discard matchings
			std::string known_map;
		};

		std::vector<TShapeInfo>		shapes;  

		// Save/load to disk. Return false on error.
		bool save() const;
		bool load();
	} l2;

	struct TLevel3
	{
		std::map<std::string,CvPoint>  cities;	//!< Cities --> (x,y) location in pixels

		// Save/load to disk. Return false on error.
		bool save() const;
		bool load();
	} l3;

	struct TLevel4
	{
		struct TPlaceInfo
		{
			CvImage		img;
			CvScalar	img_mean; //!< Used to quickly discard matchings
			CvPoint		coords;
		};

		std::vector<TPlaceInfo> places;

		// Save/load to disk. Return false on error.
		bool save() const;
		bool load();
	} l4;
};

extern TLearnData learndata;

// Press the green button until it dissapears:
void doPressGreenButton();



#endif
