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
#ifndef  MY_UTILS_H
#define  MY_UTILS_H

#ifdef _WIN32
	#include <windows.h>
#endif

#include <cv.h>
#include <highgui.h>

#include <cstdlib>
#include <direct.h>
#include <cstdio>
#include <cerrno>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <set>


#ifndef M_PI
#define M_PI  3.1415926535897932384626433832795
#endif

#ifndef M_PIf
#define M_PIf  3.141592653589793f
#endif


#define MY_ASSERT(exp) {if (!(exp)) { throw std::logic_error("ASSERT failed: "#exp); }}

/** Looks for a pattern in a bigger image, and return its position and a goodness value for the found match */
double find_pattern(const IplImage* pattern,const IplImage* img, CvPoint &out_position );

/** Extracts a part of the image */
CvImage sub_image(const CvImage &img,int org_x, int org_y,int width, int height);

/** Rotate an image */
void rotateImage( CvImage &img, double angle_radians, unsigned int center_x, unsigned int center_y, double scale );

void image_to_grayscale(CvImage &img);

void image_binarize(CvImage &img, double thredhold=90);

/** Sum of the absolute value of difference of two images */
double image_sum_abs_diff(const CvImage &a, const CvImage& b);

/** The result of a cross-correlation with a margin of a few pixels */
double image_similarity(const CvImage &a, const CvImage& b, int MARGIN = 6);

CvImage image_scale( const CvImage& img, int width, int height);

void image_drawImage(CvImage &canvas, const CvImage &img, int x,int y);

CvImage image_extract_color_channel(CvImage& img, int channel=1);


std::string str_trim(const std::string &s);
std::string str_uppercase(const std::string &s);


void emulate_cursor_move(int x,int y);
void emulate_cursor_click(int x,int y);
void emulate_mouse_click_with_movement(const CvPoint &click,const CvPoint &ret_pnt, bool be_slow, int nSteps=10 );

/**	Extracts just the directory of a filename from a
      complete path plus name plus extension */
std::string  extractFileDirectory(const std::string& filePath);

bool directoryExists(const std::string& _path);

/** A sprintf-like function for std::string  */
std::string format(const char *fmt, ...);

/** For use in  setConsoleColor */
enum TConsoleColor
{
	CONCOL_NORMAL = 0,
	CONCOL_BLUE   = 1,
	CONCOL_GREEN  = 2,
	CONCOL_RED    = 4
};

/** Changes the text color in the console for the text written from now on.
  * The parameter "color" can be any value in TConsoleColor.
  *
  * By default the color of "cout" is changed, unless changeStdErr=true, in which case "cerr" is changed.
  */
void setConsoleColor( TConsoleColor color, bool changeStdErr=false );



/** Degrees to radians */
inline double DEG2RAD(const double &x) { return x*M_PI/180.0;	}
/** Degrees to radians */
inline float DEG2RAD(const float &x) { return x*M_PIf/180.0f; }
/** Degrees to radians */
inline float DEG2RAD(const int &x) { return x*M_PIf/180.0f; }

/** Radians to degrees */
inline double RAD2DEG(const double &x) { return x*180.0/M_PI; }
/** Radians to degrees */
inline float RAD2DEG(const float &x) { return x*180.0f/M_PIf; }


#endif
