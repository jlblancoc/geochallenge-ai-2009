/* --------------------------------------------------------------------
  Facebook Geo-Challenge AI automatic player software

  Written by: 
   Jose-Luis Blanco-Claraco  <joseluisblancoc at gmail dot com>
   AUG 2009 - Spain

  This file is part of Geo-Challenge AI.
 
     Geo-Challenge AI is free software: you can redistribute it and/or 
     modify it under the terms of the GNU General Public License as 
     published by the Free Software Foundation, either version 3 of 
     the License, or (at your option) any later version.
 
     Geo-Challenge AI is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
 
     You should have received a copy of the GNU General Public License
     along with Geo-Challenge AI.  If not, see <http://www.gnu.org/licenses/>.
 -------------------------------------------------------------------- */


#ifndef SCREEN_CAPTURE_H
#define SCREEN_CAPTURE_H

#include "utils.h"

/** Captures the screen contents (Win32-only)
  * Inspired by: http://www.codeproject.com/KB/dialog/screencap.aspx
  */
class CScreenCapture
{
public:
	CScreenCapture(); //!< Default constructor, starts capturing the whole screen.
	virtual ~CScreenCapture(); //!< Destructor

	/** Start capture for the whole screen. */
	void openCapture();

	/** Start capture for a given area only. */
	void openCapture(int x, int y, int width, int height);

	int getCaptureX() const { return m_capturePos.x; }
	int getCaptureY() const { return m_capturePos.y; }

	/** Capture one frame
	  */
	CvImage CScreenCapture::captureNow();

private:
	CvPoint m_capturePos;	// Rectangle to capture
	CvSize  m_captureSize;

	HBITMAP hCaptureBitmap;
	HDC hDesktopDC;
	HDC hCaptureDC;
	HWND hDesktopWnd;

};



#endif

