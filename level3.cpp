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
				doLevel3
  ----------------------------------------------- */
void doLevel3()
{
	static CTicTac tictac;

	// Location of cities around the world
	// -------------------------------------
	CvImage scr = capture.captureNow();

#if 0
	{
		static int i=0;
		cvSaveImage(format("level3_screen_%05i.png",++i).c_str(),scr);
		Sleep(60);
		return;
	}
#endif

	// Check if we have a ticket already on the screen:
	{
		CvImage hematoma = sub_image(scr, 190,45,  85,45);
		CvPoint pos;
		double quality = find_pattern( lstImgs["l3_corner_br"],hematoma, pos);
		if (quality<0.92)
		{
			cout << "It seems there is no city name yet... waiting a sec..." << endl;
			Sleep(50);
			return;
		}
	}


	// Did we run out of time??
	{
		CvImage fistro = sub_image(scr, 472,16, 57,43);
		CvPoint pos;
		double quality = find_pattern( lstImgs["l1_time_over"],fistro, pos);
		if (quality>0.90)
		{
			cout << "     ** TIME IS OVER ** " << endl;
			
			ai_state = stAfterLevel3Wait;
			Sleep(1000);
			return;
		}
	}

	// Extract the city name:
	// --------------------------------------
	const int city_img_w = 170;
	const int city_img_h = 34;
	CvImage city_name_img_org = sub_image(scr,71,25,city_img_w,city_img_h);
	
	// A binarization of an enlarged image must be better:
	CvImage city_name_img = image_scale( city_name_img_org , city_name_img_org.width()*2,city_name_img_org.height()*2);

	// Rotate:
	rotateImage(city_name_img,DEG2RAD(3.7),city_name_img.width()>>1,city_name_img.height()>>1,1);


#if defined(_DEBUG)
	cvSaveImage("cityname.png",city_name_img);
#endif

	const double thres = 127;
	image_binarize(city_name_img,thres );

	// Erode it to make OCR easier:
	//CvImage city_name_img (cvGetSize(city_name_img2),city_name_img2.depth(),city_name_img2.channels());
	//cvErode( city_name_img2,city_name_img );	
	//CvScalar city_name_img_mean = cvAvg(city_name_img);

	string city_name_str;
	ocr.ocr_on_image(city_name_img, city_name_str, true);

#if defined(_DEBUG)
	cvSaveImage("cityname_bin.png",city_name_img);
#endif

	if (city_name_str.empty())
	{
		cout << "It seems there is no city name yet... waiting a sec..." << endl;
		Sleep(100);
		return;
	}

	// Check if we are still reading the last city!!
	static std::string last_city;
	if (last_city==city_name_str)
	{
		// Comoooorl?
		cout << "Waiting for a new city name..." << endl;
		Sleep(100);
		return;
	}

	// For the future...
	last_city=city_name_str;


	cout << "===============> City name : ";
	setConsoleColor(CONCOL_BLUE);
	cout << city_name_str << endl;
	setConsoleColor(CONCOL_NORMAL);


	// Look for the known coordinates??
	// -------------------------------------------
	CvPoint city_click = cvPoint(-1,-1);

	{
		std::map<std::string,CvPoint>::const_iterator it = learndata.l3.cities.find(city_name_str);
		if (it != learndata.l3.cities.end())
		{
			// I know this one!
			city_click.x = capture.getCaptureX()+ it->second.x;
			city_click.y = capture.getCaptureY()+ it->second.y;
			
			setConsoleColor(CONCOL_GREEN);
			cout << "=====> I know this one! coords = (" << city_click.x << ", " << city_click.y << ")." << endl;
			setConsoleColor(CONCOL_NORMAL);
		}
		else
		{
			// Random?
			setConsoleColor(CONCOL_GREEN);
			cout << "=====> I do NOT know this one... random pick!" << endl;
			setConsoleColor(CONCOL_NORMAL);
			
			city_click.x = capture.getCaptureX()+ 50+ (rand()%450);
			city_click.y = capture.getCaptureY()+ 50+ (rand()%350);
		}
	}

	// ---------------------------------
	// Create the display image:
	// ---------------------------------
	CvImage display_img(cvSize(400,350),8,3);
	cvRectangle(display_img,cvPoint(0,0),cvPoint(display_img.width(),display_img.height()),cvScalar(120,120,120,120),CV_FILLED);

	// Draw detected city name:
	{
		CvImage country_bw = city_name_img;
		CvImage country_col( cvGetSize( country_bw ), country_bw.depth(), 3 );
		cvCvtColor( country_bw, country_col, CV_GRAY2BGR );

		image_drawImage(display_img,country_col,10,10);
	}

	// Draw the OCR detected name:
	cvPutText(
		display_img,
		format("OCR: %s",city_name_str.c_str()).c_str(),
		cvPoint(20,140),
		&my_font1,
		CV_RGB(0,0,0));

	// Show image of my decision
	cvShowImage("Geochallenge AI",display_img);
	cvWaitKey(1);


	// Do the click:
	CvPoint ret_pnt = cvPoint(  capture.getCaptureX()+2+(rand()%30), capture.getCaptureY()+200+(rand()%30) );
	emulate_mouse_click_with_movement(city_click,ret_pnt,be_slow,3);


	// Look for the real position, given by the green stick:
	// ---------------------------------------------------------
	bool	chincheta_jaaaarl = false; // Found?
	CvPoint chincheta_pos = cvPoint(-1,1);

	CvImage more_recent_screen; // Used later...

	{
		int timeout=0;
		do
		{
			CvImage pantallaca = capture.captureNow();
			if (!timeout) more_recent_screen = pantallaca;

			const int offset_x = 40;
			const int offset_y = 90;

			// Look in the map area only!
			CvImage mapilla = sub_image(pantallaca,offset_x,offset_y,  480,325);

			// Debugging...
			//cvSaveImage(format("chinch_%05i.png",timeout).c_str(),pantallaca);
			
			double quality = find_pattern( lstImgs["chincheta"],mapilla, chincheta_pos);
			
			chincheta_pos.x+=offset_x;
			chincheta_pos.y+=offset_y;
			
			cout << "Chincheta: " << quality << " -> " << chincheta_pos.x << ", " << chincheta_pos.y << endl;

			chincheta_jaaaarl = (quality>0.85);

#if 0
			// Debug
			if (chincheta_jaaaarl)
			{
				static int a=0;
				cvSaveImage(format("%05i_detected_chinch_%i,%i_%f.png",++a,chincheta_pos.x,chincheta_pos.y,quality).c_str(),pantallaca);
			}
#endif

			if (!chincheta_jaaaarl)
			{
				Sleep(80);

				// Did we run out of time??
				{
					CvImage fistro = sub_image(pantallaca, 472,16, 57,43);
					CvPoint pos;
					double quality = find_pattern( lstImgs["l1_time_over"],fistro, pos);
					if (quality>0.90)
					{
						cout << "     ** TIME IS OVER ** " << endl;
						ai_state = stAfterLevel3Wait;
						Sleep(1000);
						return;
					}
				}
			}

		} while (!chincheta_jaaaarl && ++timeout<35);
	}


	// learn the lesson:
	if (chincheta_pos.x>=0)
	{
		// ala, aprendetelo:
		chincheta_pos.x+=8;
		chincheta_pos.y+=9;
		learndata.l3.cities[city_name_str] = chincheta_pos;

		setConsoleColor(CONCOL_GREEN);
		cout << "Learned: " << city_name_str << " << in at: " << chincheta_pos.x << ", " << chincheta_pos.y << endl;
		setConsoleColor(CONCOL_NORMAL);
	}
	else
	{
		setConsoleColor(CONCOL_RED);
		cout << "Mmm... it seems I missed the green stick... :-("  << endl;
		setConsoleColor(CONCOL_NORMAL);
	}

	// Now, wait until the next country name starts to appear, or a timeout, whaever first.
	CvImage compare_patch = sub_image(more_recent_screen,0,0,  128,32);
	bool waiting_end_animation=false;
	for (int timeout=30;timeout>=0;--timeout)
	{
		CvImage pantallaca = capture.captureNow();
		CvImage compare_patch2 = sub_image(pantallaca,0,0,  128,32);

		const double d = image_sum_abs_diff(compare_patch2 ,compare_patch);
		//cout << d << endl;

		// For the next iter:
		compare_patch = compare_patch2;

		bool end = false;

		if (!waiting_end_animation )
		{
			cout << "Waiting for START of animation in city name..." << endl;
			if (d>0.1)
				waiting_end_animation=true;
		}
		else 
		{
			cout << "Waiting for END of animation in city name..." << endl;
			if (d<0.01)
			{
				end = true;
				Sleep(150); // Make sure the animation is really over...
			}
		}
		
		// Did we run out of time??
		{
			CvImage fistro = sub_image(pantallaca, 472,16, 57,43);
			CvPoint pos;
			double quality = find_pattern( lstImgs["l1_time_over"],fistro, pos);
			if (quality>0.90)
			{
				cout << "     ** TIME IS OVER ** " << endl;
				ai_state = stAfterLevel3Wait;
				Sleep(1000);
				return;
			}
		}

		Sleep(80);
		if (end) break;
	}
}



void doAfterLevel3Wait()
{
	cout << endl;
	cout << endl;
	cout << endl;
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Waiting for Level 4 ============ " << endl;
	cout << "  I will press the green button until Level 4 begins... " << endl;
	setConsoleColor(CONCOL_NORMAL);
	cout << endl;

	Sleep(3000);

	// Push the green button with the mouse:
	doPressGreenButton();

	learndata.l3.save();
	// Go on:
	ai_state = stLevel4;
}



bool TLearnData::TLevel3::save() const 
{
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Saving data of LEVEL 3...";
	setConsoleColor(CONCOL_NORMAL);

	_mkdir("learndata");
	_mkdir("learndata/level3");
#ifdef _WIN32
	system("del learndata\\level3\\*.* /Q");
#endif
	
	std::ofstream f;
	f.open("learndata/level3/cities.txt");
	if (!f.is_open()) return false;
	for (map<string,CvPoint>::const_iterator it=cities.begin();it!=cities.end();it++)
		f << it->first << " " << it->second.x << " " << it->second.y << endl;

	setConsoleColor(CONCOL_BLUE);
	cout << "Done!"<< endl;
	setConsoleColor(CONCOL_NORMAL);
	return true;
}


bool TLearnData::TLevel3::load() 
{
	cities.clear();

	if (!directoryExists("learndata/level3"))
		return false;

	ifstream f;
	f.open("learndata/level3/cities.txt");
	if(!f.is_open()) 
		return false;

	while (!f.eof())
	{
		string city;
		int x,y;
		f >> city >> x >> y;
		if (!f.eof())
		{
			cities[city].x = x;
			cities[city].y = y;
		}
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "[LEVEL3] ";
	setConsoleColor(CONCOL_NORMAL);
	cout << "Loaded " << cities.size() << " cities." << endl;
	return true;
}

