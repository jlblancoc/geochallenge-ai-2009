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

#include "utils.h"

#include <sys/stat.h>
#include <cctype>
#include <cstdarg>
#include <vector>
#include <algorithm>

using namespace std;

/** Looks for a pattern in a bigger image, and return its position and a goodness value for the found match */
double find_pattern(const IplImage* pattern,const IplImage* img, CvPoint &out_position )
{
	const int x_search_size=img->width-pattern->width+1;
	const int y_search_size=img->height-pattern->height+1;

	static CvImage result;
	
	// Only create the image as needed:
	if (!result || result.width()!=x_search_size || result.height()!=y_search_size)
	{
		result.attach( cvCreateImage(cvSize(x_search_size,y_search_size),IPL_DEPTH_32F, 1) );
	}

	// Compute cross correlation:
	cvMatchTemplate(img,pattern,result,CV_TM_CCOEFF_NORMED);

	// Find the max point:
	double mini,max_val;
	CvPoint min_point,max_point;
	cvMinMaxLoc(result,&mini,&max_val,&min_point,&max_point,NULL);

	out_position.x = max_point.x + int((pattern->width-1)/2);
	out_position.y = max_point.y + int((pattern->height-1)/2);

	return max_val;
}

CvImage sub_image(const CvImage &img,int org_x, int org_y,int width, int height)
{
	CvRect roi;
	roi.x = org_x;
	roi.y = org_y;
	roi.width = width;
	roi.height = height;

	// fix ROI:
	cvSetImageROI(const_cast<IplImage*>(img.operator const IplImage *()),roi);

	CvImage res(cvSize(roi.width,roi.height),img.depth(),img.channels());

	cvCopy(img,res);
	cvResetImageROI( const_cast<IplImage*>(img.operator const IplImage *()) );

	return res;
}


std::string str_trim(const std::string &s)
{
	std::string ret;
	for (size_t i=0;i<s.size();i++)
		if (isalnum(s[i]) && !isspace(s[i]))
			ret+=s[i];

	return ret;
}

std::string str_uppercase(const std::string &s)
{
	string outStr( s );
	transform(
		outStr.begin(), outStr.end(),		// In
		outStr.begin(),			// Out
		(int(*)(int)) tolower );
	return outStr;
}


void rotateImage( CvImage &img, double angle_radians, unsigned int center_x, unsigned int center_y, double scale )
{
	IplImage *srcImg = img;
	IplImage *outImg = cvCreateImage( cvGetSize( srcImg ), srcImg->depth, srcImg->nChannels );

	// Based on the blog entry:
	// http://blog.weisu.org/2007/12/opencv-image-rotate-and-zoom-rotation.html

	// Apply rotation & scale:
	float m[6];
	CvMat M = cvMat(2, 3, CV_32F, m);

	m[0] = (float)(scale*cos(angle_radians));
	m[1] = (float)(scale*sin(angle_radians));
	m[3] = -m[1];
	m[4] = m[0];
	m[2] = (float)center_x;
	m[5] = (float)center_y;

	cvGetQuadrangleSubPix( srcImg, outImg, &M );

	outImg->origin = srcImg->origin;

	// Assign the output image to the IPLImage pointer within the CImage
	img.attach(outImg);
}


void image_to_grayscale(CvImage &img)
{
	if (img.channels()==1) return;

	// Convert to a single luminance channel image
	IplImage *outImg = cvCreateImage( cvGetSize( img ), img.depth(), 1 );
	cvCvtColor( img, outImg, CV_BGR2GRAY );
	img.attach(outImg);
}


void image_binarize(CvImage &img, double thredhold)
{
	image_to_grayscale(img);
	CvImage img2(cvGetSize(img),img.depth(),img.channels());

	//cvAdaptiveThreshold(img,img2,255);
	cvThreshold(img,img2,thredhold,255,CV_THRESH_BINARY);
	img = img2;
}

void emulate_cursor_move(int x,int y)
{
	// Convert from pixels to [0,0x10000]:
	x =int( 0x10000* x / double(GetSystemMetrics(SM_CXSCREEN)));
	y =int( 0x10000* y / double(GetSystemMetrics(SM_CYSCREEN)));

	mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, x,y,0, NULL);
}

void emulate_cursor_click(int x,int y)
{
	// Convert from pixels to [0,0x10000]:
	x =int( 0x10000* x / double(GetSystemMetrics(SM_CXSCREEN)));
	y =int( 0x10000* y / double(GetSystemMetrics(SM_CYSCREEN)));

	mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_MOVE, x,y,0, NULL);
	mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTDOWN, x,y,0, NULL);
	mouse_event(MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_LEFTUP, x,y,0, NULL);
}


/**	Extracts just the directory of a filename from a
      complete path plus name plus extension */
std::string  extractFileDirectory(const std::string& filePath)
{
	if (filePath.size()<2) return filePath;

	// Search the first "/" or "\" from the right:
	int i;
	for (i=(int)filePath.size()-1;i>0;i--)
		if (filePath[i]=='\\' || filePath[i]=='/')
			break;

	if (!i) return string("");
	else	return filePath.substr(0,i+1);
}

bool directoryExists(const std::string& _path)
{
	std::string path = _path;

	// Remove the trailing "/" or "\\":
	if (!path.empty() && (*path.rbegin()=='/' || *path.rbegin()=='\\'))
		path = path.substr(0,path.size()-1);

	// Verify it's a directory:
	struct _stat buf;
	if (0!=_stat(path.c_str(),&buf)) return false;

#ifdef _WIN32
	return 0!=(buf.st_mode &_S_IFDIR);
#else
	return S_ISDIR(buf.st_mode);
#endif
}

/** A sprintf-like function for std::string  */
std::string format(const char *fmt, ...)
{
	if (!fmt) return string("");

	int   result = -1, length = 1024;
	vector<char> buffer;
	while (result == -1)
	{
		buffer.resize(length + 10);

		va_list args;  // This must be done WITHIN the loop
		va_start(args,fmt);
		result = vsnprintf(&buffer[0], length, fmt, args);
		va_end(args);

		// Truncated?
		if (result>=length) result=-1;
		length*=2;
	}
	string s(&buffer[0]);
	return s;
}


double image_sum_abs_diff(const CvImage &a, const CvImage& b)
{
	CvImage diffImage(cvGetSize(a),a.depth(),a.channels());
	cvAbsDiff(a,b,diffImage);
	CvScalar diff_sum = cvSum(diffImage);
	return (double(diff_sum.val[0]+diff_sum.val[1]+diff_sum.val[2]))/(a.width()*a.height()*255*a.channels());
}

double image_similarity(const CvImage &a, const CvImage& b, int MARGIN )
{
	CvImage a_cut = sub_image(a,MARGIN,MARGIN,a.width()-2*MARGIN,a.height()-2*MARGIN);
	CvPoint pos;
	return find_pattern(a_cut,b, pos);
}


CvImage image_scale( const CvImage& img, int width, int height)
{
	CvImage outImg(cvSize(width,height), img.depth(), img.channels());

	cvResize( img, outImg, CV_INTER_AREA); //CV_INTER_CUBIC );
	return outImg;

}


/** Changes the text color in the console for the text written from now on.
  * The parameter "color" can be:
  *  - 0 : Normal text color
  *  - 1 : Blue text color
  *  - 2 : Green text color
  *  - 4 : Red text color
  */
void setConsoleColor( TConsoleColor color,bool changeStdErr )
{
	static const int TS_NORMAL = 0;
	static const int TS_BLUE   = 1;
	static const int TS_GREEN  = 2;
	static const int TS_RED    = 4;

#ifdef _WIN32
    static int normal_attributes = -1;
    HANDLE hstdout = GetStdHandle( changeStdErr ? STD_ERROR_HANDLE : STD_OUTPUT_HANDLE );
	fflush(changeStdErr ? stderr: stdout);

    if( normal_attributes < 0 )
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo( hstdout, &info );
        normal_attributes = info.wAttributes;
    }

    SetConsoleTextAttribute( hstdout,
        (WORD)(color == TS_NORMAL ? normal_attributes :
        ((color & TS_BLUE ? FOREGROUND_BLUE : 0)|
        (color & TS_GREEN ? FOREGROUND_GREEN : 0)|
        (color & TS_RED ? FOREGROUND_RED : 0)|FOREGROUND_INTENSITY)) );
#else
	// *nix:
    static const uint8_t ansi_tab[] = { 30, 34, 32, 36, 31, 35, 33, 37 };
    int code = 0;
	fflush( changeStdErr ? stdout:stderr );
    if( color != TS_NORMAL )
        code = ansi_tab[color & (TS_BLUE|TS_GREEN|TS_RED)];
    fprintf(changeStdErr ? stdout:stderr, "\x1b[%dm", code );
#endif
}

void image_drawImage(CvImage &canvas, const CvImage &img, int x,int y)
{
	cvSetImageROI( canvas, cvRect( x, y, img.width(), img.height() ) );
	cvCopy( img, canvas );
	cvResetImageROI(canvas);
}


CvImage image_extract_color_channel(CvImage& img, int channel)
{
	img.set_coi(channel);
	CvImage out(cvGetSize(img),img.depth(),1);
	cvCopy(img,out);
	img.set_coi(0); // All
	return out;
}


void emulate_mouse_click_with_movement(const CvPoint &click,const CvPoint &ret_pnt, bool be_slow, int nSteps )
{
	Sleep(be_slow ? 500 : 1);
	const double click_x = click.x;
	const double click_y = click.y;

	double mouse_x = ret_pnt.x;
	double mouse_y = ret_pnt.y;

	const double Ax = (click_x-mouse_x)/nSteps;
	const double Ay = (click_y-mouse_y)/nSteps;

	for (int step=0;step<nSteps;step++)
	{
		mouse_x+=Ax;
		mouse_y+=Ay;
		emulate_cursor_move( int(mouse_x),int(mouse_y));
		Sleep(2);
	}

	Sleep(be_slow ? 250 : 1);

	// Click:
	emulate_cursor_click( int(click_x),int(click_y));

	for (int step=0;step<nSteps;step++)
	{
		mouse_x-=Ax;
		mouse_y-=Ay;
		emulate_cursor_move( int(mouse_x),int(mouse_y));
		Sleep(2);
	}
}

