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
				doLevel4
  ----------------------------------------------- */
void doLevel4()
{
	static CTicTac tictac;

	// Location of landmarks around the world
	// -------------------------------------
	CvImage scr = capture.captureNow();

#if 0
	{
		static int i=0;
		cvSaveImage(format("level4_screen_%05i.png",++i).c_str(),scr);
		Sleep(60);
		return;
	}
#endif

	// Check if we have a picture on the screen...
	{
		CvScalar p1=cvGet2D(scr,135,192);   // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
		//cout << p1.val[2] << " " << p1.val[1] << " " << p1.val[0] << endl;
		if ( p1.val[2]<254 || p1.val[1]<254 || p1.val[0]<254)
		{
			cout << "It seems there is no picture yet... waiting a sec..." << endl;
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
			
			ai_state = stAfterLevel4Wait;
			Sleep(1000);
			return;
		}
	}

	// Extract the picture of the landmark/place:
	// -------------------------------------------------
	const CvImage  place_img = sub_image(scr,55,43, 140,88);
	const CvScalar place_img_mean = cvAvg(place_img );

	// Identify the place among known ones:
	// -------------------------------------------
	tictac.Tic();
	int place_idx = -1;
	{
		// Compare to all existing ones:
		int		best_idx = -1;
		double  best_simil = 0;
		for (size_t i=0;i<learndata.l4.places.size();i++)
		{
			const double brute_diff = (1.0/3)*(
				std::abs(place_img_mean.val[0]-learndata.l4.places[i].img_mean.val[0])+
				std::abs(place_img_mean.val[1]-learndata.l4.places[i].img_mean.val[1])+
				std::abs(place_img_mean.val[2]-learndata.l4.places[i].img_mean.val[2]));

			if (brute_diff>1.0)
				continue; // Ey, these images don't look very alike...
			 
			const double d = image_similarity(place_img,learndata.l4.places[i].img);
			cout << brute_diff << " -> " << d << endl;
			if (d>best_simil)
			{
				best_simil = d;
				best_idx = i;
			}
		}

		// good enough?
		if (best_simil>0.90)
		{
			place_idx = best_idx;
		}
		else
		{
			place_idx = learndata.l4.places.size(); // New index

			// It's a new picture: Add to the list
			TLearnData::TLevel4::TPlaceInfo dat;
			dat.img = place_img;
			dat.img_mean = place_img_mean;
			dat.coords.x =
			dat.coords.x = -1;

			learndata.l4.places.push_back( dat );
		}
	} 

	MY_ASSERT(place_idx>=0);
	cout << "Photo identified in " << 1e3*tictac.Tac() << " ms as #" << place_idx << endl;


	// Look for the known coordinates??
	// -------------------------------------------
	CvPoint city_click = cvPoint(-1,-1);

	if ( learndata.l4.places[place_idx].coords.x>=0 )
	{
		// I know this one!
		city_click.x = capture.getCaptureX()+ learndata.l4.places[place_idx].coords.x;
		city_click.y = capture.getCaptureY()+ learndata.l4.places[place_idx].coords.y;
		
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

	// ---------------------------------
	// Create the display image:
	// ---------------------------------
	CvImage display_img(cvSize(400,350),8,3);
	cvRectangle(display_img,cvPoint(0,0),cvPoint(display_img.width(),display_img.height()),cvScalar(120,120,120,120),CV_FILLED);

	// Draw detected place:
	image_drawImage(display_img, learndata.l4.places[place_idx].img ,10,10);

	// Draw the OCR detected name:
	{
		string str;

		if (place_idx == (learndata.l4.places.size()-1))
				str = format("New place (#%u)",(unsigned)place_idx);
		else	str = format("Known place (#%u)",(unsigned)place_idx);

		cvPutText(
			display_img,
			str.c_str(),
			cvPoint(20,120),
			&my_font1,
			CV_RGB(0,0,0));
	}


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
						ai_state = stAfterLevel4Wait;
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
		learndata.l4.places[place_idx].coords = chincheta_pos;

		setConsoleColor(CONCOL_GREEN);
		cout << "Learned, place #" << place_idx << " << is at: " << chincheta_pos.x << ", " << chincheta_pos.y << endl;
		setConsoleColor(CONCOL_NORMAL);
	}
	else
	{
		setConsoleColor(CONCOL_RED);
		cout << "Mmm... it seems I missed the green stick... :-("  << endl;
		setConsoleColor(CONCOL_NORMAL);
	}

	// Now, wait until the next image starts to appear, or a timeout, whatever first.
	CvImage compare_patch = sub_image(more_recent_screen,90,20,  64,32);
	bool waiting_end_animation=false;
	for (int timeout=30;timeout>=0;--timeout)
	{
		CvImage pantallaca = capture.captureNow();
		CvImage compare_patch2 = sub_image(pantallaca,90,20,  64,32);

		const double d = image_sum_abs_diff(compare_patch2 ,compare_patch);
		// For the next iter:
		compare_patch = compare_patch2;

		bool end = false;

		if (!waiting_end_animation )
		{
			cout << "Waiting for START of animation in landmark photo..." << endl;
			if (d>0.1)
				waiting_end_animation=true;
		}
		else 
		{
			cout << "Waiting for END of animation in landmark photo..." << endl;
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
				ai_state = stAfterLevel4Wait;
				Sleep(1000);
				return;
			}
		}


		Sleep(80);
		if (end) break;
	}
}



void doAfterLevel4Wait()
{
	cout << endl;
	cout << endl;
	cout << endl;
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ ALL 4 LEVELS ARE DONE!!! ============ " << endl;
	setConsoleColor(CONCOL_NORMAL);
	cout << endl;

	Sleep(1000);

	learndata.l4.save();

	cvShowImage("Geochallenge AI",lstImgs["end"]);

	// Go on:
	ai_state = stEnd;
}



bool TLearnData::TLevel4::save() const 
{
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Saving data of LEVEL 4...";
	setConsoleColor(CONCOL_NORMAL);

	_mkdir("learndata");
	_mkdir("learndata/level4");
#ifdef _WIN32
	system("del learndata\\level4\\*.* /Q");
#endif
	
	for (unsigned int i=0;i<places.size();i++)
	{
		cvSaveImage(format("learndata/level4/place%04u.png",i).c_str(),places[i].img );

		std::ofstream f;
		f.open(format("learndata/level4/place%04u.txt",i).c_str());
		if (!f.is_open()) return false;
			f << places[i].coords.x << " " << places[i].coords.y << endl;
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "Done!"<< endl;
	setConsoleColor(CONCOL_NORMAL);
	return true;
}


bool TLearnData::TLevel4::load() 
{
	places.clear();

	if (!directoryExists("learndata/level4"))
		return false;

	unsigned int n = 0;
	IplImage *newImg;

	while ( (newImg=cvLoadImage(format("learndata/level4/place%04u.png",n).c_str())) != NULL )
	{
		TPlaceInfo dat;

		dat.img = CvImage(newImg);
		dat.img_mean = cvAvg(newImg);

		ifstream f;
		f.open(format("learndata/level4/place%04u.txt",n).c_str());
		MY_ASSERT(f.is_open());
		
		f >> dat.coords.x >> dat.coords.y;

		places.push_back(dat);
		n++;
	}


	setConsoleColor(CONCOL_BLUE);
	cout << "[LEVEL4] ";
	setConsoleColor(CONCOL_NORMAL);
	cout << "Loaded " << places.size() << " places." << endl;
	return true;
}

