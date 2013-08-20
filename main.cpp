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

#include <conio.h>
#include <signal.h>

using namespace std;

/********** Global vars **********/
COCREngine				ocr;
CScreenCapture			capture;
map<string,CvImage>		lstImgs;		//<! List of useful images
TLearnData				learndata;
bool					be_slow = false;
bool					gonna_exit = false;

CvFont			my_font1;

TAIState	ai_state;

void doWaitMainScreen();
void doLevel1();	void doAfterLevel1Wait();
void doLevel2();	void doAfterLevel2Wait();
void doLevel3();	void doAfterLevel3Wait();
void doLevel4();	void doAfterLevel4Wait();
void doEnd();
void initialize_all();

/* -----------------------------------------------
				Main
  ----------------------------------------------- */
int main(int narg, char** args)
{
	try
	{
		// Intro text:
		setConsoleColor(CONCOL_GREEN);
		cout << " ----------------------------------------------------------------------------" << endl;
		cout << "Geochallenge AI bot - version 1.0" << endl;
		cout << "Copyright (C) 2009 Jose Luis Blanco" << endl;
		cout << "This program comes with ABSOLUTELY NO WARRANTY;for details see GNU GPL v3" << endl;
		cout << " This is free software, and you are welcome to redistribute it" << endl;
		cout << " under certain conditions." << endl;
		cout << endl;
		cout << "                                    http://code.google.com/p/open-cv-bots/" << endl;
		cout << " ----------------------------------------------------------------------------" << endl;
		setConsoleColor(CONCOL_NORMAL);
	
		cout << endl;

		// Set cwd:
		_chdir(extractFileDirectory(args[0]).c_str());

		initialize_all();

		cvNamedWindow("Geochallenge AI");
		cvShowImage("Geochallenge AI",lstImgs["intro"]);


		cout << endl;
		cout << "Waiting for the main screen of GeoChallenge" << endl;

	

		for(;;) // Main loop
		{
			switch(ai_state)
			{
			case stWaitMainScreen:	doWaitMainScreen(); break;
			case stLevel1:			doLevel1();			break;
			case stAfterLevel1Wait: doAfterLevel1Wait(); break;
			case stLevel2:			doLevel2();			break;
			case stAfterLevel2Wait: doAfterLevel2Wait(); break;
			case stLevel3:			doLevel3();			break;
			case stAfterLevel3Wait: doAfterLevel3Wait(); break;
			case stLevel4:			doLevel4();			break;
			case stAfterLevel4Wait: doAfterLevel4Wait(); break;
			case stEnd:				doEnd(); break;
			}

			if (gonna_exit) break; // Ctrl-C, etc..

			if (27==cvWaitKey(1))	// needed to refresh windows.
			{
				break;	// El fistro quiere salirrrr
			}

			// exit with ESC?
			if (_kbhit())
				if (_getch()==27)
					break;	// El fistro quiere salirrrr
		}

		

		cvDestroyAllWindows();

		// Bye bye julay
		return 0;
	}
	catch(exception &e)
	{
		cerr << e.what() << endl;
		cin.get();
		return 1;
	}
}


IplImage* MY_cvLoadImage(const char* fil)
{
	IplImage *ret = cvLoadImage(fil);
	if (!ret)
		throw std::logic_error(format("ERROR: Cannot find image file: %s",fil));
	return ret;
}


void my_sighandler(int sig)
{
    cout<< "Signal " << sig << " caught. Closing..." << endl;
	gonna_exit = true;
}

BOOL WINAPI my_closehandler(DWORD dwCtrlType)
{
    cout<< "Closing..."<< endl;
	gonna_exit = true;
	return TRUE;
}

/* -----------------------------------------------
			Initialize All
  ----------------------------------------------- */
void initialize_all()
{
	MY_ASSERT(directoryExists("data"));

	signal(SIGABRT, &my_sighandler);
	signal(SIGTERM, &my_sighandler);
	signal(SIGINT, &my_sighandler);

	SetConsoleCtrlHandler(&my_closehandler,TRUE);


	my_font1 = cvFont(1);


	capture.openCapture(); // Reset capture to the whole screen

	// Load list of images:
	lstImgs.clear();
	lstImgs["red_btn"] = MY_cvLoadImage("data/red_btn.png");
	lstImgs["large_green_btn"] = MY_cvLoadImage("data/large_green_btn.png");
	lstImgs["handle"] = MY_cvLoadImage("data/handle.png");
	lstImgs["shaded_bag"] = MY_cvLoadImage("data/shaded_bag.png");
	lstImgs["l1_time_over"] = MY_cvLoadImage("data/l1_time_over.png");
	lstImgs["ticket_corner"] = MY_cvLoadImage("data/ticket_corner.png");
	lstImgs["l3_corner_br"] = MY_cvLoadImage("data/l3_corner_br.png");
	lstImgs["chincheta"] = MY_cvLoadImage("data/chincheta.png");
	
	lstImgs["intro"] = MY_cvLoadImage("data/intro.png");
	lstImgs["end"] = MY_cvLoadImage("data/end.png");

	// State:
	ai_state = stWaitMainScreen;

	// Learn data:
	learndata.l1.flags.clear();

	// Try to load from files:
	if (!learndata.l1.load()) cerr << "** WARNING **: Level 1 learn data was not found. Starting from scratch." << endl;
	if (!learndata.l2.load()) cerr << "** WARNING **: Level 2 learn data was not found. Starting from scratch." << endl;
	if (!learndata.l3.load()) cerr << "** WARNING **: Level 3 learn data was not found. Starting from scratch." << endl;
	if (!learndata.l4.load()) cerr << "** WARNING **: Level 4 learn data was not found. Starting from scratch." << endl;
}




