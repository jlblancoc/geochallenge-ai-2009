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
#include <limits>

using namespace std;


/* -----------------------------------------------
				doLevel1
  ----------------------------------------------- */
void doLevel1()
{
	static CTicTac tictac;
	
	// 3 or 6 flags, select by its name
	// -------------------------------------	
	CvImage scr = capture.captureNow();

	// Did we run out of time??
	{
		CvImage part = sub_image(scr, 472,16, 57,43);
		CvPoint pos;
		double quality = find_pattern( lstImgs["l1_time_over"],part, pos);
		if (quality>0.90)
		{
			cout << "     ** TIME IS OVER ** " << endl;
			
			ai_state = stAfterLevel1Wait;
			Sleep(1000);
			return;
		}
	}

	// Before reading the country name, make sure we are in a NEW question, 
	//  by looking for the shaded suitcase at the bottom:
	{
		CvImage part = sub_image(scr, 105,380, 470,50);
		
		CvPoint pos;
		double quality = find_pattern( lstImgs["shaded_bag"],part, pos);
		if (quality<0.75)
		{
			cout << "I can't play if there's not a shaded suitcase on the bottom!" << endl;
			Sleep(100);
			return;
		}
	}


	const int country_img_w = 180;
	const int country_img_h = 35;
	CvImage country_name_img_org = sub_image(scr,185,25,country_img_w,country_img_h);
	
	// A binarization of an enlarged image must be better:
	CvImage country_name_img2 = image_scale( country_name_img_org , country_name_img_org.width()*2,country_name_img_org.height()*2);

#ifdef _DEBUG
	cvSaveImage("countryname.png",country_name_img2);
#endif

	const double thres = 127;
	image_binarize(country_name_img2,thres );

	// Erode it to make OCR easier:
	CvImage country_name_img (cvGetSize(country_name_img2),country_name_img2.depth(),country_name_img2.channels());
	
	cvErode( country_name_img2,country_name_img );	
	CvScalar country_name_img_mean = cvAvg(country_name_img);

	string country_name_str;
	ocr.ocr_on_image(country_name_img2, country_name_str, true);


#ifdef _DEBUG
	cvSaveImage("countryname_bin.png",country_name_img);
#endif

#if 0
	// OCR:
	std::string country_idx;
	ocr.ocr_on_image(country_name_img, country_idx,true);
#else
	// Non-OCR version: Directly associate with the name IMAGES (more robust!)

	// ----------------------------------------------------------------------------
	// 3 or 6 flags? 
	// ----------------------------------------------------------------------------
	bool twoRowsOfFlags=false; 

	// Trick: look for a case handle around (245,204). 
	//  If found, we have 6 flags:
	{
		CvPoint pos_handle;

		double goodness[6];
		
		CvImage sub_scr;

		sub_scr=sub_image(scr,70,203,   130,13+8);	goodness[0] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,225,203,  130,13+8);	goodness[1] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,375,203,  130,13+8);	goodness[2] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,70,97,   130,13+8);	goodness[3] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,225,97,  130,13+8);	goodness[4] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,375,97,  130,13+8);	goodness[5] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);

		twoRowsOfFlags=true;
		for (int i=0;i<6;i++)
			twoRowsOfFlags = twoRowsOfFlags && (goodness[i]>0.85);
	}

	// And just to make sure, if no handle is found at the second row, 
	// look for one at the first row:
	if (!twoRowsOfFlags)
	{
		CvPoint pos_handle;
		double goodness[3];
		
		CvImage sub_scr;

		sub_scr=sub_image(scr,210,154,   130,13+8);	goodness[0] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,225,154,  130,13+8);	goodness[1] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);
		sub_scr=sub_image(scr,375,154,   130,13+8);	goodness[2] = find_pattern(lstImgs["handle"],sub_scr,pos_handle);

		if (goodness[0]<0.85 || goodness[1]<0.85 || goodness[2]<0.85)
		{
			cout << "I can't see ALL the flags!... trying again in a sec." << endl;
			Sleep(100);
			return;
		}
	}

	// Determine the country ID, or assign a new one if it's a new country:
	// -------------------------------------------------------------------------
	TCountryID   country_idx = -1; //-1 -> New one, other number, an index in "learndata.l1.country_names".

	tictac.Tic();
	{
		// Compare to all existing ones:
		int		best_idx = -1;
		double  best_simil = 0;
		for (size_t i=0;i<learndata.l1.country_names.size();i++)
		{
			const double brute_diff = (1.0/3)*(
				std::abs(country_name_img_mean.val[0]-learndata.l1.country_names[i].img_mean.val[0])+
				std::abs(country_name_img_mean.val[1]-learndata.l1.country_names[i].img_mean.val[1])+
				std::abs(country_name_img_mean.val[2]-learndata.l1.country_names[i].img_mean.val[2]));

			if (brute_diff>0.4)
				continue; // Ey, these images don't look very alike...
			 
			const double d = image_similarity(country_name_img,learndata.l1.country_names[i].img);
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
			country_idx = best_idx;
		}
		else
		{
			country_idx = learndata.l1.country_names.size();  // New index

			// It's a new name: Add to the list
			TLearnData::TLevel1::TCountryInfo dat;
			dat.img = country_name_img;
			dat.img_mean = country_name_img_mean;

			learndata.l1.country_names.push_back( dat );
		}
	} // end for n

	cout << "Country identified in " << 1e3*tictac.Tac() << " ms." << endl;
#endif


#if 0
	if (country_idx.size()<3)
	{
		cout << "I can't read any country name... trying again in a sec. (" << country_idx <<")" << endl;
		Sleep(100);
		return;
	}
#endif

	cout << "===================> COUNTRY IDX: " << country_idx << ". OCR read: " << country_name_str << endl;

	const size_t nFlags = twoRowsOfFlags ? 6:3;
	
	// Extract all the flags:
	// -----------------------------
	static const int Ax=-25;
	static const int Ay= 18;
	static const int flags3_corner_xs[]={130+Ax,279+Ax,430+Ax};
	static const int flags3_corner_ys[]={160+Ay,160+Ay,160+Ay};
	static const int flags6_corner_xs[]={130+Ax,280+Ax,430+Ax,130+Ax,280+Ax,430+Ax};
	static const int flags6_corner_ys[]={100+Ay,100+Ay,100+Ay,206+Ay,206+Ay,206+Ay};
	static const int flag_width  = 64;
	static const int flag_height = 50;

	cout << "Extracting " << nFlags  << " flags..." << endl;
	std::vector<CvImage>  lstFlagImgs(nFlags);
	std::vector<CvScalar> lstFlagImgMeans(nFlags);


	for (size_t i=0;i<nFlags;i++)
	{
		lstFlagImgs[i] = sub_image(
			scr, 
			twoRowsOfFlags ? flags6_corner_xs[i]:flags3_corner_xs[i],
			twoRowsOfFlags ? flags6_corner_ys[i]:flags3_corner_ys[i],
			flag_width,
			flag_height);

		// Compute the image means, as a brute descriptor:
		lstFlagImgMeans[i] = cvAvg(lstFlagImgs[i]);

		// Half-size for efficient cross-correlation:
		//lstFlagImgs[i] = image_scale(lstFlagImgs[i],lstFlagImgs[i].width()>>1,lstFlagImgs[i].height()>>1);
	}

	// Classify each flag among the already known ones:
	// --------------------------------------------------
	std::vector<int> lstFlagsIdx(nFlags,-1); //-1 -> New one, other number, an index in "learndata.l1.flags".
	int flag_to_press = -1;  // 0-5
	bool Iam100sureIknowThis=false; // For detecting inconsistent data if I'm sure but I fail...
	int screen_index_of_correct_flag_if_I_fail_when_I_pick_on_two = -1; // Isn't the name enough?? ;-)

	tictac.Tic();
	
	// For each seen flag:
	for (size_t n=0;n<nFlags;n++)
	{
		// Compare to all existing ones:
		int		best_idx = -1;
		double  best_simil = 0;
		for (size_t i=0;i<learndata.l1.flags.size();i++)
		{
			// First pass: brute mean colors:
			const double brute_diff = (1.0/3)*(
				std::abs(lstFlagImgMeans[n].val[0]-learndata.l1.flags[i].img_mean.val[0])+
				std::abs(lstFlagImgMeans[n].val[1]-learndata.l1.flags[i].img_mean.val[1])+
				std::abs(lstFlagImgMeans[n].val[2]-learndata.l1.flags[i].img_mean.val[2]));

			if (brute_diff>5)
				continue; // Ey, these images don't look very alike...

			// Second pass: cross-correlation:
			const double d = image_similarity(lstFlagImgs[n],learndata.l1.flags[i].img, 2);

			//const double diff = image_sum_abs_diff(lstFlagImgs[n],learndata.l1.flags[i].img);
			if (d>best_simil)
			{
				best_simil = d;
				best_idx = i;
			}
		}

		// good enough?
		if (best_simil>0.98)
		{
			lstFlagsIdx[n] = best_idx;
			//cout << " S: " << best_simil << " of #"<<n << " with flag " << best_idx << endl;

			// If this is THE flag, skip the others!
			if (!learndata.l1.flags[best_idx].yes.empty())
			{
				if ((*learndata.l1.flags[best_idx].yes.begin()) == country_idx )
				{	
					flag_to_press = n;
					Iam100sureIknowThis = true;
					cout << "===> It's so easy!! ";
					setConsoleColor(CONCOL_GREEN);
					cout << "It MUST BE flag #" << flag_to_press+1 << endl;
					setConsoleColor(CONCOL_NORMAL);
					break;
				}
			}
		}
		else
		{
			lstFlagsIdx[n] = learndata.l1.flags.size();  // New index

			// It's a new flag: Add to the list
			learndata.l1.flags.push_back( TLearnData::TLevel1::TFlagInfo() );
			learndata.l1.flags[lstFlagsIdx[n]].img = lstFlagImgs[n];
			learndata.l1.flags[lstFlagsIdx[n]].img_mean = lstFlagImgMeans[n];

		}
	} // end for n
	
#if 0
	cout << "Recognized flags: ";
	for (size_t n=0;n<nFlags;n++)
		cout << lstFlagsIdx[n] << " ";
	cout << endl;
#endif
	cout << "Recognition took: " << tictac.Tac()*1e3 << " ms." << endl;



	// And finally: Do my call! -> Detect the flag by its country name, if I can... 
	// -----------------------------------------------------------------------------

	// 1st: Look for a flag known to correspond to this country:
	// (Already done above)


	// 2nd: If still not decided:
	if (flag_to_press<0)
	{
		// Make a list with those non-discardable, either for:
		//  1) Having a "yes" list
		//  2) Havng THIS map in the "no" list.

		std::set<int> flagIndInScreenToConsider;
		for (size_t n=0;n<nFlags;n++)
		{
			const size_t flag_number_in_list = lstFlagsIdx[n];
			if (learndata.l1.flags[flag_number_in_list].yes.empty() &&
				learndata.l1.flags[flag_number_in_list].no.count(country_idx)==0)
			{
				flagIndInScreenToConsider.insert(n);
			}
		}

		if(flagIndInScreenToConsider.empty())
		{
			setConsoleColor(CONCOL_RED);
			cout << "WARNING: All the flags were discarded!!" << endl 
				 << "This means there's something wrong: FORGETTING all about them." << endl;
			setConsoleColor(CONCOL_NORMAL);

			// Forgot inconsistent data:
			for (size_t n=0;n<nFlags;n++)
			{
				if (lstFlagsIdx[n]<0) continue;
				const size_t flag_number_in_list = lstFlagsIdx[n];
				learndata.l1.flags[flag_number_in_list].yes.clear();
				learndata.l1.flags[flag_number_in_list].no.clear();
			}
			
			// Set all as options:
			flagIndInScreenToConsider.clear();
			for (size_t n=0;n<nFlags;n++)
				flagIndInScreenToConsider.insert(n);
		}

		if (flagIndInScreenToConsider.size()==1)
		{
			Iam100sureIknowThis=true;
			flag_to_press = *flagIndInScreenToConsider.begin();
			cout << "===> ";
			setConsoleColor(CONCOL_GREEN);
			cout << "By elimination, IT MUST BE flag #" << flag_to_press+1 << endl;
			setConsoleColor(CONCOL_NORMAL);
		}
		else
		{
			cout << "===> Mmmmm, I'm not sure!! I have ";
			setConsoleColor(CONCOL_GREEN);
			cout << flagIndInScreenToConsider.size() << " candidates -> random pick!" << endl;
			setConsoleColor(CONCOL_NORMAL);

			std::set<int>::iterator it = flagIndInScreenToConsider.begin();
			std::advance(it, rand() % flagIndInScreenToConsider.size());

			flag_to_press = *it;

			// Smart move: If we are btwn two flags, keep the other as backup so
			//  if I fail, I'll know FOR SURE that the other one was the correct:
			if (flagIndInScreenToConsider.size()==2)
			{
				if (it==flagIndInScreenToConsider.begin())
				{
					std::set<int>::iterator picha = flagIndInScreenToConsider.begin();
					picha++;
					screen_index_of_correct_flag_if_I_fail_when_I_pick_on_two = *picha;
				}
				else
				{
					screen_index_of_correct_flag_if_I_fail_when_I_pick_on_two  = *flagIndInScreenToConsider.begin();
				}
			}
		}
	}


	// Press the selected flag:
	static const int flags3_xs[]={140,280,440};
	static const int flags3_ys[]={200,200,200};
	static const int flags6_xs[]={140,280,440,140,280,440};
	static const int flags6_ys[]={140,140,140,240,240,240};

	MY_ASSERT(flag_to_press>=0);
	cout << "SELECTING FLAG #" << flag_to_press+1 << endl;

	// ---------------------------------
	// Create the display image:
	// ---------------------------------
	CvImage display_img(cvSize(400,300),8,3);
	cvRectangle(display_img,cvPoint(0,0),cvPoint(display_img.width(),display_img.height()),cvScalar(120,120,120,120),CV_FILLED);

	// Draw detected country:
	{
		CvImage country_bw = learndata.l1.country_names[country_idx].img;
		CvImage country_col( cvGetSize( country_bw ), country_bw.depth(), 3 );
		cvCvtColor( country_bw, country_col, CV_GRAY2BGR );

		image_drawImage(display_img,country_col,10,10);
	}

	// Draw detected flags:
	for (size_t n=0;n<nFlags;n++)
	{
		const int idxFlag = lstFlagsIdx[n];
		if (idxFlag<0) continue;
		CvImage img = learndata.l1.flags[idxFlag].img;

		int x = 30+ (20+img.width())* (n%3);
		int y = 90+ (20+img.height())*(n/3);
		image_drawImage(display_img,img,x,y);

		if (n==flag_to_press)
			cvRectangle(display_img,cvPoint(x-3,y-3),cvPoint(x+3+img.width(),y+3+img.height()),cvScalar(0,0,255),2);
	}

	// Show image of my decision
	cvShowImage("Geochallenge AI",display_img);
	cvWaitKey(1);

	// It seems that the game is smart against bots?? 
	// Emulate mouse move...
	// ---------------------------------------------------------
	CvPoint click = cvPoint( 
		capture.getCaptureX()+ (twoRowsOfFlags ? flags6_xs[flag_to_press] : flags3_xs[flag_to_press]),
		capture.getCaptureY()+ (twoRowsOfFlags ? flags6_ys[flag_to_press] : flags3_ys[flag_to_press]) );
	CvPoint ret_pnt = cvPoint(  capture.getCaptureX()+600+(rand()%30), capture.getCaptureY()+2+(rand()%30) );

	emulate_mouse_click_with_movement(click,ret_pnt,be_slow,3);

	// Look for the green / red colors of the signs:
	bool did_score=false;
	CvImage src2;
	{
		int timeout=30;
		while( --timeout>0 )
		{
			src2= capture.captureNow();
			//cvSaveImage(format("mark_%05i.png",timeout).c_str(),src2);

			// Decide green / red?

			// BAD: 
			CvScalar p1=cvGet2D(src2,139,333); // IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
			//cout << p1.val[2] << " " << p1.val[1] << " " << p1.val[0] << endl;
			if ( std::abs(p1.val[2]-225)<20 && std::abs(p1.val[1]-5)<20 && std::abs(p1.val[0]-15)<20 )
			{
				did_score = false;
				break;
			}

			// GOOD: 
			CvScalar p2=cvGet2D(src2,204,217); // Scored . IMPORTANT: cvGet2D seems to get coordinates as (y,x), not (x,y)!! 
			CvScalar p3=cvGet2D(src2,225,210); // Scored, in a row
			// [2]: R
			// [1]: G
			// [0]: B
			if ( (std::abs(p2.val[2]-145)<15 && std::abs(p2.val[1]-194)<15) ||
				 (std::abs(p3.val[2]-108)<10 && std::abs(p3.val[1]-174)<20) )
			{
				did_score = true;
				break;
			}

			Sleep(20);
		} 
		
		if (timeout<=0)
		{
			cout << endl << "It looks like a MISS?" << endl;
			Sleep(200);
			//system("pause");
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
	


	// Double check everything:
	//  Check if the grat suitcase is still at the bottom: 
	//   then it's not a mistake, but seems a MISS!
	{
		CvImage part = sub_image(src2, 105,380, 450,50);
		CvPoint pos;
		double quality = find_pattern( lstImgs["shaded_bag"],part, pos);
		if (quality>0.83)
		{
			cout << endl << "It looks like a MISS?" << endl;
			Sleep(200);
			return;
		}
	}

#if 0
	// Keep a history of choices:
	{
		static int iter=0;
		string name = format("learndata/screen%05i",++iter);

		name+=format("_COUNTRY=%i",(int)country_idx);
		name+="_FLAGS=";
		for (size_t n=0;n<nFlags;n++)
			name+=format("%i,",lstFlagsIdx[n]);

		name+=format("_PICK=#%i",flag_to_press+1);
		
		name+= (did_score? "_OK":"_ERR");

		name+=".png";

		cvSaveImage(name.c_str(),scr);
	}
#endif

	// Watch for inconsistencies:
	if (Iam100sureIknowThis && !did_score)
	{
		setConsoleColor(CONCOL_RED);
		cout << "WARNING: I was sure about this one but failed!!" << endl 
			 << "  --> FORGETTING all about these flags!!" << endl;
		setConsoleColor(CONCOL_NORMAL);

		// Forgot inconsistent data:
		for (size_t n=0;n<nFlags;n++)
		{
			if (lstFlagsIdx[n]<0) continue;
			const size_t flag_number_in_list = lstFlagsIdx[n];
			learndata.l1.flags[flag_number_in_list].yes.clear();
			learndata.l1.flags[flag_number_in_list].no.clear();
		}
#if 0
		// Something's wrong!!
		cvSaveImage("screen.png",scr);
		learndata.l1.save();
		cout << "Ey cowboy!" << endl;
		system("pause");
#endif
	}


	// -------------------------------------------------
	// LEARNING:
	//  Use the score or not score to learn something
	// -------------------------------------------------
	if (did_score)
	{
		// It was a good choice: reward it:
		// In fact, we can remove all the negative rewards and leave only the correct country:
		learndata.l1.flags[ lstFlagsIdx[flag_to_press] ].no.clear();
		learndata.l1.flags[ lstFlagsIdx[flag_to_press] ].yes.clear();
		learndata.l1.flags[ lstFlagsIdx[flag_to_press] ].yes.insert( country_idx );

		// And mark all the other flags as NOT being this country:
		for (size_t i=0;i<nFlags;i++)
		{
			if (i==flag_to_press) continue;
			if (lstFlagsIdx[i]<0) continue;
			// If this flag is already known without doubt, don't touch it:
			if (learndata.l1.flags[ lstFlagsIdx[i] ].yes.empty())
			{	// Negative reward:
				learndata.l1.flags[ lstFlagsIdx[i] ].no.insert(country_idx);
			}
		}
	}
	else
	{
		// Mark the choice as wrong:
		learndata.l1.flags[ lstFlagsIdx[flag_to_press] ].no.insert(country_idx);

		// If there was only one remaining possibility, I DO KNOW it must be correct now:
		if (screen_index_of_correct_flag_if_I_fail_when_I_pick_on_two>=0)
		{
			const int i=screen_index_of_correct_flag_if_I_fail_when_I_pick_on_two; // it's shorter!!
			learndata.l1.flags[ lstFlagsIdx[i] ].no.clear();
			learndata.l1.flags[ lstFlagsIdx[i] ].yes.clear();
			learndata.l1.flags[ lstFlagsIdx[i] ].yes.insert(country_idx);
			
			setConsoleColor(CONCOL_GREEN);
			cout << "Ey! By elimination, flag #" << i+1 <<" must be correct! ;-)" << endl;
			setConsoleColor(CONCOL_NORMAL);
		}
	}

	// Wait until the game reacts and show me a new country:
	Sleep(300);
}


void doAfterLevel1Wait()
{
	cout << endl;
	cout << endl;
	cout << endl;
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Waiting for Level 2 ============ " << endl;
	cout << "  I will press the green button until Level 2 begins... " << endl;
	setConsoleColor(CONCOL_NORMAL);
	cout << endl;

	Sleep(3000);

	doPressGreenButton();


	learndata.l1.save();
	ai_state = stLevel2;
}





bool TLearnData::TLevel1::load() 
{
	flags.clear();
	country_names.clear();

	if (!directoryExists("learndata/level1"))
		return false;

	unsigned int n = 0;
	IplImage *newImg;

	unsigned int nPerfectFlags = 0;

	while ( (newImg=cvLoadImage(format("learndata/level1/flag%04u.png",n).c_str())) != NULL )
	{
		TFlagInfo newData;
		newData.img.attach( newImg );
		newData.img_mean = cvAvg(newImg);

		ifstream f_yes;
		f_yes.open(format("learndata/level1/flag%04u_yes.txt",n).c_str());
		MY_ASSERT(f_yes.is_open());
		while (!f_yes.eof())
		{
			TCountryID country;
			f_yes >> country;
			if (!f_yes.eof() && !f_yes.fail())
			{
				newData.yes.insert(country);
				nPerfectFlags++;
			}
		}

		ifstream f_no;
		f_no.open(format("learndata/level1/flag%04u_no.txt",n).c_str());
		MY_ASSERT(f_no.is_open());
		while (!f_no.eof())
		{
			TCountryID country;
			f_no >> country;
			if (!f_no.eof() && !f_no.fail())
				newData.no.insert(country);
		}
		
		flags.push_back(newData);
		n++;
	}

	// Country names:
	n = 0;
	while ( (newImg=cvLoadImage(format("learndata/level1/name%04u.png",n).c_str(),false)) != NULL )
	{
		TLearnData::TLevel1::TCountryInfo dat;

		dat.img = CvImage(newImg);
		dat.img_mean= cvAvg(newImg);

		country_names.push_back(dat);

		n++;
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "[LEVEL1] ";
	setConsoleColor(CONCOL_NORMAL);
	cout << "Loaded " << country_names.size() << " countries and " << flags.size() << " flags, "<<nPerfectFlags<< " perfectly known." << endl;

	return true;
}

bool TLearnData::TLevel1::save() const 
{
	setConsoleColor(CONCOL_BLUE);
	cout << " ============ Saving data of LEVEL 1...";
	setConsoleColor(CONCOL_NORMAL);

	_mkdir("learndata");
	_mkdir("learndata/level1");
#ifdef _WIN32
	system("del learndata\\level1\\*.* /Q");
#endif
	
	for (unsigned i=0;i<flags.size();i++)
	{
		cvSaveImage( format("learndata/level1/flag%04u.png",i).c_str(),flags[i].img);
		
		std::ofstream f_yes,f_no;
		f_yes.open(format("learndata/level1/flag%04u_yes.txt",i).c_str());
		f_no.open(format("learndata/level1/flag%04u_no.txt",i).c_str());
		if (!f_yes.is_open()) return false;
		if (!f_no.is_open()) return false;

		for (std::set<TCountryID>::const_iterator it=flags[i].yes.begin();it!=flags[i].yes.end();it++)
			f_yes << *it << endl;
		
		for (std::set<TCountryID>::const_iterator it=flags[i].no.begin();it!=flags[i].no.end();it++)
			f_no << *it << endl;
	}

	for (unsigned i=0;i<country_names.size();i++)
	{
		cvSaveImage( format("learndata/level1/name%04u.png",i).c_str(),country_names[i].img);
	}

	setConsoleColor(CONCOL_BLUE);
	cout << "Done!"<< endl;
	setConsoleColor(CONCOL_NORMAL);
	return true;
}

