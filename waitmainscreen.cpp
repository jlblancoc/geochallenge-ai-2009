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

#include "data.h"

using namespace std;


/* -----------------------------------------------
				doWaitMainScreen
  ----------------------------------------------- */
void doWaitMainScreen()
{
	// Look for the known pattern:
	capture.openCapture(); // Look in the whole screen:
	CvImage scr = capture.captureNow();


	CvPoint pos1,pos2;
	
	CvImage pat1 = lstImgs["red_btn"];
	CvImage pat2 = lstImgs["large_green_btn"];

	image_to_grayscale(pat1);
	image_to_grayscale(pat2);
	
	image_to_grayscale(scr);

	double goodness1 = find_pattern(pat1,scr,pos1);
	double goodness2 = find_pattern(pat2,scr,pos2);

	int Ax=pos1.x-pos2.x;
	int Ay=pos2.y-pos1.y;
	bool pos_are_consistent = std::abs(Ax-185)<10 && std::abs(Ay-405)<10;

#ifdef _DEBUG
	cout << "goodness: " << goodness1 <<" " << goodness2 << endl;
#endif

	if (goodness1>0.85 && goodness2>0.85 && pos_are_consistent)
	{
		// Define the rectangle to capture:
		const int cap_x = pos1.x - 622;
		const int cap_y = pos1.y - 16;
		const int cap_w = 640;
		const int cap_h = 480;

		// Check if it's out of the screen:
		if (cap_x<0 || cap_y<0 || (cap_x+cap_w)>GetSystemMetrics(SM_CXSCREEN) || (cap_y+cap_h)>GetSystemMetrics(SM_CYSCREEN))
		{
			cout << endl;
			cout << "ERROR: GEO-CHALLENGE SCREEN FOUND OUT THE SCREEN BORDERS" << endl;
			cout << "       I'll keep trying in a sec..." << endl;
			Sleep(1000);
		}
		else
		{
			// OK!
			capture.openCapture(cap_x,cap_y,cap_w,cap_h);

			cout << endl;
			cout << "==== GEO-CHALLENGE SCREEN FOUND (" << cap_x <<","<<cap_y << ")x("<<cap_w<<","<<cap_h <<") ====" << endl;
			cout << endl;

			// Push the green button with the mouse:
			doPressGreenButton();

			// Go on:
			ai_state = stLevel1;
		}
	}
	else
	{
		cout << "."; //"NOT FOUND. I'll keep trying in a sec..." << endl;
		//cout << endl;
		Sleep(800);
	}
}



// Press the green button until it dissapears:
void doPressGreenButton()
{
	CvPoint ret_pnt = cvPoint(capture.getCaptureX()+2, capture.getCaptureY()+200 );

	emulate_cursor_move(ret_pnt.x,ret_pnt.y); 
	Sleep(400);

	bool btn_seen_once = false;

	for(int timeout=0;timeout<2000;timeout++)
	{
		CvImage scr = capture.captureNow();

		// Detect green btn by color count:

		CvScalar p1=cvGet2D(scr,434,434); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
		CvScalar p2=cvGet2D(scr,434,422);
		// [2]: R
		// [1]: G
		// [0]: B
		if ( ((std::abs(p1.val[2]-138)<10 && std::abs(p1.val[1]-186)<10 && std::abs(p1.val[0]-41)<10 ) && 
			  (std::abs(p2.val[2]-255)<10 && std::abs(p2.val[1]-255)<10 && std::abs(p2.val[0]-255)<10 )) 
			  ||
			 ((std::abs(p2.val[2]-138)<10 && std::abs(p2.val[1]-186)<10 && std::abs(p2.val[0]-41)<10 ) && 
			  (std::abs(p1.val[2]-255)<10 && std::abs(p1.val[1]-255)<10 && std::abs(p1.val[0]-255)<10 )))
		{
			btn_seen_once = true;

			CvPoint click = cvPoint(capture.getCaptureX()+440,capture.getCaptureY()+420);
			emulate_mouse_click_with_movement(click,ret_pnt,true,50);
			Sleep(300);
		}
		else
		{
			if (btn_seen_once)
			{
				// Btn seen and pushed:
				break;
			}
			else
			{
				// Btn is still not there!
				Sleep(200);
			}
		}
	}
}


void doEnd()
{
	cout << endl;
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ ALL 4 LEVELS ARE DONE!!! ============ " << endl;
	cout << "  Press ESC to exit." << endl;
	setConsoleColor(CONCOL_NORMAL);
	cout << endl;

	Sleep(200);
}