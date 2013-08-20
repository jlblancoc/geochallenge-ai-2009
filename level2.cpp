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
				doLevel2
  ----------------------------------------------- */
void doLevel2()
{
	static CTicTac tictac;

	// 1 map shape, 4 possibilities
	// ------------------------------	
	CvImage scr = capture.captureNow();

	// Did we run out of time??
	{
		CvImage part = sub_image(scr, 472,16, 57,43);
		CvPoint pos;
		double quality = find_pattern( lstImgs["l1_time_over"],part, pos);
		if (quality>0.90)
		{
			cout << "     ** TIME IS OVER ** " << endl;
			
			ai_state = stAfterLevel2Wait;
			Sleep(1000);
			return;
		}
	}
	

	// Make sure we have all the 4 names presented and fade in animation is over:
	{
		CvImage part = sub_image(scr, 490,373, 75,32);
		CvPoint pos;
		double quality = find_pattern( lstImgs["ticket_corner"],part, pos);
		//cout << quality << endl;
		if (quality<0.90)
		{
			cout << "The 4 names are not still on the screen (no pattern)...waiting a sec." << endl;
			Sleep(100);
			return;
		}
	}

	// Double check: one pixel in ALL country name must be in white (otherwise it's still the last panel):
	static const int tickets_x[]={530,530,530,540};
	static const int tickets_y[]={104,202,293,390};
	CvScalar tickets_color[4];

	//cvSaveImage("screen.png",scr);
	for (size_t i=0;i<4;i++)
		tickets_color[i] = cvGet2D(scr,tickets_y[i],tickets_x[i]); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 

	bool all_white = true;
	for (size_t i=0;i<4;i++)
	{
		all_white = all_white && (tickets_color[i].val[0]>230 && tickets_color[i].val[1]>230 && tickets_color[i].val[2]>230);
	}

	if (!all_white)
	{
		cout << "Four new names are not still on the screen (no white)...waiting a sec." << endl;
		Sleep(100);
		return;
	}

	// OCR on the 4 country names:
	// --------------------------------
	std::string map_str[4];

	// Maps choices:
	CvImage map_choice_1 = sub_image(scr,391,123,143,30);
	CvImage map_choice_2 = sub_image(scr,391,206,145,35);
	CvImage map_choice_3 = sub_image(scr,398,300,150,30);
	CvImage map_choice_4 = sub_image(scr,402,390,150,50);

	rotateImage(map_choice_1,DEG2RAD(2.0),map_choice_1.width()/2,map_choice_1.height()/2,1);
	//rotateImage(map_choice_2,DEG2RAD(0),map_choice_2.width()/2,map_choice_2.height()/2,1);
	//rotateImage(map_choice_3,DEG2RAD(0),map_choice_3.width()/2,map_choice_3.height()/2,1);
	rotateImage(map_choice_4,DEG2RAD(-7.5),map_choice_4.width()/2,map_choice_4.height()/2,1);

	const double thres = 90;
	image_binarize(map_choice_1,thres );
	image_binarize(map_choice_2,thres );
	image_binarize(map_choice_3,thres );
	image_binarize(map_choice_4,thres );

	//cvSaveImage("map1.png",map_choice_1);
	//cvSaveImage("map2.png",map_choice_2);
	//cvSaveImage("map3.png",map_choice_3);
	//cvSaveImage("map4.png",map_choice_4);

	ocr.ocr_on_image(map_choice_1 , map_str[0], true);
	ocr.ocr_on_image(map_choice_2 , map_str[1], true);
	ocr.ocr_on_image(map_choice_3 , map_str[2], true);
	ocr.ocr_on_image(map_choice_4 , map_str[3], true);
	
	setConsoleColor(CONCOL_GREEN);
	cout << "MAP CHOICES: " << 
		map_str[0] << ", " <<
		map_str[1] << ", " <<
		map_str[2] << ", " <<
		map_str[3]<< endl;
	setConsoleColor(CONCOL_NORMAL);


	// Add map names to my DB:
//	for (int i=0;i<4;i++)
//		learndata.l2.country_names.insert(map_str[i]);


	// Get the map shape:
	// ------------------------
	CvImage map_shape_img = sub_image(scr, 77,126, 300, 260);

	// Segmentation by color:
	// Blue channel is <180 for the area of interest, >= otherwise.
	CvImage img_shape_bin = image_extract_color_channel(map_shape_img, 1);
	image_binarize(img_shape_bin, 180);

	CvScalar img_shape_avr = cvAvg(img_shape_bin); // As a fast descriptor


	// Identify the map shape among known maps:
	// -------------------------------------------------
	int shape_idx = -1;
	tictac.Tic();
	{
		// Compare to all existing ones:
		int		best_idx = -1;
		double  best_simil = 0;
		for (size_t i=0;i<learndata.l2.shapes.size();i++)
		{
			const double brute_diff = std::abs(img_shape_avr.val[0]-learndata.l2.shapes[i].img_mean.val[0]);
			//cout << brute_diff << endl;

			if (brute_diff>1.0)
				continue; // Ey, these images don't look very alike...
			 
			const double d = image_similarity(img_shape_bin,learndata.l2.shapes[i].img);
			//cout << brute_diff << " -> " << d << endl;
			if (d>best_simil)
			{
				best_simil = d;
				best_idx = i;
			}
		}

		// good enough?
		if (best_simil>0.90)
		{
			shape_idx = best_idx;
		}
		else
		{
			shape_idx = learndata.l2.shapes.size(); // New index

			// It's a new name: Add to the list
			TLearnData::TLevel2::TShapeInfo dat;
			dat.img = img_shape_bin;
			dat.img_mean = img_shape_avr;

			learndata.l2.shapes.push_back( dat );
		}
	} // end for n

	cout << "Map shape identified in " << 1e3*tictac.Tac() << " ms." << endl;
	MY_ASSERT(shape_idx>=0)

	// Decide selection:
	// ---------------------------------------------------------
	int map_to_press = -1;
	int index_in_screen_of_countryname_that_is_correct_if_I_fail = -1;

	if (!learndata.l2.shapes[shape_idx].known_map.empty())
	{
		// I DO know this shape:
		const string known_match = learndata.l2.shapes[shape_idx].known_map;

		// See if any of the 4 map names coincide (it should!)
		int matching_mapname = -1;
		for (int i=0;i<4;i++)
		{
			if (map_str[i]==known_match)
			{
				// Yeah
				matching_mapname=i;

				cout << "===> It's so easy!! ";
				setConsoleColor(CONCOL_GREEN);
				cout << "It MUST BE: " << map_str[matching_mapname] << endl;
				setConsoleColor(CONCOL_NORMAL);
				break;
			}
		}

		if (matching_mapname>=0)
		{
			map_to_press=matching_mapname;
		}
		else
		{
			// Shouldnt'...
			setConsoleColor(CONCOL_GREEN);
			cout << "===> Mmmmm, I though I know this one but I can't read its name... :-S!! -> random pick!"<<endl;
			setConsoleColor(CONCOL_NORMAL);

			map_to_press = rand() % 4;		
		}
	}
	else
	{
		// I do NOT know this shape:

		// Make a list of potential choices:
		set<int>  lstPotentialsMapIndx;
		for (int i=0;i<4;i++)
		{
			// Only add those WITHOUT a shape correctly asociated:
			bool isKnown = false;
			for (unsigned int k=0;k<learndata.l2.shapes.size();k++)
			{
				if (learndata.l2.shapes[k].known_map==map_str[i])
				{
					isKnown=true;
					break;
				}
			}

			if (!isKnown)
				lstPotentialsMapIndx.insert(i);
		}

		// Pick among the potentials:
		const size_t nPotentials = lstPotentialsMapIndx.size();

		if (nPotentials)
		{
			setConsoleColor(CONCOL_GREEN);
			cout << "===> Mmmmm, I have " << nPotentials << " posibilities... -> random pick!" << endl;
			setConsoleColor(CONCOL_NORMAL);

			set<int>::iterator it=lstPotentialsMapIndx.begin();
			std::advance(it, rand() % nPotentials );

			map_to_press = *it;

			// A smart point:
			if (nPotentials==2)
			{
				if (it==lstPotentialsMapIndx.begin())
				{
					set<int>::iterator it= lstPotentialsMapIndx.begin();
					it++;
					index_in_screen_of_countryname_that_is_correct_if_I_fail = *it;
				}
				else
				{
					index_in_screen_of_countryname_that_is_correct_if_I_fail = *lstPotentialsMapIndx.begin();
				}
			}

		}
		else
		{
			setConsoleColor(CONCOL_GREEN);
			cout << "===> Mmmmm, I have NO posibilities!! :-S... random pick!"<< endl;
			setConsoleColor(CONCOL_NORMAL);

			map_to_press = rand() % 4;
		}
	}



	// Emulate mouse move...
	// ---------------------------------------------------------
	static const int clicks_x[]={430,460,460,430};
	static const int clicks_y[]={140,222,316,395};

	MY_ASSERT(map_to_press>=0 && map_to_press<4);

	CvPoint click = cvPoint( capture.getCaptureX()+ clicks_x[map_to_press], capture.getCaptureY()+ clicks_y[map_to_press] );
	CvPoint ret_pnt = cvPoint(  capture.getCaptureX()+2+(rand()%30), capture.getCaptureY()+200+(rand()%30) );

	emulate_mouse_click_with_movement(click,ret_pnt,be_slow,3);

	// ---------------------------------
	// Create the display image:
	// ---------------------------------
	CvImage display_img(cvSize(400,350),8,3);
	cvRectangle(display_img,cvPoint(0,0),cvPoint(display_img.width(),display_img.height()),cvScalar(120,120,120,120),CV_FILLED);

	// Draw detected shape:
	{
		CvImage country_bw = learndata.l2.shapes[shape_idx].img;
		CvImage country_col( cvGetSize( country_bw ), country_bw.depth(), 3 );
		cvCvtColor( country_bw, country_col, CV_GRAY2BGR );

		image_drawImage(display_img,country_col,10,10);
	}

	// Draw the 4 detected names:
	for (int i=0;i<4;i++)
	{
		cvPutText(
			display_img,
			format("Choice %i: %s",i,map_str[i].c_str()).c_str(),
			cvPoint(20,250+i*20),
			&my_font1,
			CV_RGB(0,0,255));
	}

	// Show image of my decision
	cvShowImage("Geochallenge AI",display_img);
	cvWaitKey(1);


	// Look for the green / red colors of the tickets:
	// -------------------------------
	bool did_score = false;
	int it_was_btn_number = -1;  // if I fail...
	{
		int timeout=40;

		do
		{
			// Check colors now:
			CvImage screen2 = capture.captureNow();

			for (size_t i=0;i<4;i++)
			{
				tickets_color[i] = cvGet2D(screen2,tickets_y[i],tickets_x[i]); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
				//cout << tickets_color[i].val[0] << " " <<tickets_color[i].val[1] << " "<< tickets_color[i].val[2] << endl;
			}

			// [2]: R
			// [1]: G
			// [0]: B
			
			// Look for someone red?
			bool btn_red[4];
			bool btn_green[4];
			for (int i=0;i<4;i++)
			{
				btn_red[i]   = (tickets_color[i].val[2]>200) && (tickets_color[i].val[1]<50) && (tickets_color[i].val[0]<50);
				btn_green[i] = (tickets_color[i].val[2]<200) && (tickets_color[i].val[1]>180) && (tickets_color[i].val[0]<50);
			}

			if (btn_red[0] || btn_red[1] || btn_red[2] || btn_red[3])
			{
				for (int k=0;k<4;k++)
					if (btn_green[k]) 
					{
						it_was_btn_number=k;
						break;
					}

				did_score=false;
				break;
			}

			// Or look for a green tick in the middle...
			const CvScalar p1=cvGet2D(screen2,250,230); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
			const CvScalar p2=cvGet2D(screen2,210,145); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 

			if ( (std::abs(p1.val[2]-0)<20 && std::abs(p1.val[1]-122)<20 && std::abs(p1.val[0]-45)<20 ) ||
				 (std::abs(p1.val[2]-147)<20 && std::abs(p1.val[1]-195)<20 && std::abs(p1.val[0]-27)<20 ) )
			{
				did_score=true;
				break;
			}

			Sleep(30);
		} while (--timeout>0);

		if (timeout<=0)
		{
			cout << endl << "It looks like a MISS?" << endl;
			Sleep(200);
			return;
		}
	}

	if (did_score)
	{
		setConsoleColor(CONCOL_GREEN);
		cout << "Scored! ;-)" << endl;
	}
	else
	{
		setConsoleColor(CONCOL_RED);
		cout << "Didn't score :-(" << endl;

	}
	setConsoleColor(CONCOL_NORMAL);


	// Learning =============================
	if (!did_score && it_was_btn_number>=0)
	{
		cout << "Learning it was: " << map_str[it_was_btn_number] << endl;
		learndata.l2.shapes[shape_idx].known_map = map_str[it_was_btn_number];
	}

	Sleep(300);
}



void doAfterLevel2Wait()
{
	cout << endl;
	cout << endl;
	cout << endl;
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Waiting for Level 3 ============ " << endl;
	cout << "  I will press the green button until Level 3 begins... " << endl;
	setConsoleColor(CONCOL_NORMAL);
	cout << endl;

	Sleep(3000);

	// Push the green button with the mouse:
	doPressGreenButton();

	learndata.l2.save();
	// Go on:
	ai_state = stLevel3;
}



bool TLearnData::TLevel2::save() const 
{
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Saving data of LEVEL 2...";
	setConsoleColor(CONCOL_NORMAL);

	_mkdir("learndata");
	_mkdir("learndata/level2");
#ifdef _WIN32
	system("del learndata\\level2\\*.* /Q");
#endif
	
	for (unsigned i=0;i<shapes.size();i++)
	{
		cvSaveImage( format("learndata/level2/shape%04u.png",i).c_str(),shapes[i].img);
		
		std::ofstream f;
		f.open(format("learndata/level2/shape%04u.txt",i).c_str());
		if (!f.is_open()) return false;
		f << shapes[i].known_map;
		f.close();
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "Done!"<< endl;
	setConsoleColor(CONCOL_NORMAL);
	return true;
}


bool TLearnData::TLevel2::load() 
{
	shapes.clear();

	if (!directoryExists("learndata/level2"))
		return false;

	unsigned int n = 0;
	IplImage *newImg;

	unsigned int nPerfectMaps = 0;

	while ( (newImg=cvLoadImage(format("learndata/level2/shape%04u.png",n).c_str(),0)) != NULL )
	{
		TShapeInfo	newData;
		newData.img.attach( newImg );
		newData.img_mean = cvAvg(newImg);

		ifstream f;
		f.open(format("learndata/level2/shape%04u.txt",n).c_str());
		MY_ASSERT(f.is_open());

		
		std::string read_map;
		std::getline(f,read_map);

		if (!read_map.empty())
		{
			newData.known_map = read_map;
			nPerfectMaps++;
		}
		else
			newData.known_map = "";

		shapes.push_back(newData);
		n++;
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "[LEVEL2] ";
	setConsoleColor(CONCOL_NORMAL);
	cout << "Loaded " << shapes.size() << " shapes, " <<nPerfectMaps<< " perfectly known." << endl;
	return true;
}

