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


#include "screen_capture.h"
#include "CTicTac.h"

using namespace std;

void WriteBMPFile(HBITMAP bitmap, const char* filename, HDC hDC);


CScreenCapture::CScreenCapture() : 
	hCaptureBitmap(NULL)
{
    hDesktopWnd = GetDesktopWindow();
    hDesktopDC = GetDC(hDesktopWnd);
    hCaptureDC = CreateCompatibleDC(hDesktopDC);
    
	openCapture();
}

/** Start capture for the whole screen. */
void CScreenCapture::openCapture()
{
	CvPoint new_p;
	CvSize  new_size;
	
	new_p.x=0;
	new_p.y=0;
	new_size.width = GetSystemMetrics(SM_CXSCREEN);
	new_size.height = GetSystemMetrics(SM_CYSCREEN);

	if (new_size.width==m_captureSize.width && new_size.height==m_captureSize.height &&
		new_p.x==m_capturePos.x && new_p.y==m_capturePos.y)
		return; // already done!

	m_capturePos	= new_p;
	m_captureSize	= new_size;

	if (hCaptureBitmap)
	{
		DeleteObject(hCaptureBitmap);
		hCaptureBitmap=NULL;
	}
	hCaptureBitmap =CreateCompatibleBitmap(hDesktopDC, m_captureSize.width, m_captureSize.height);
    SelectObject(hCaptureDC,hCaptureBitmap); 
}

/** Start capture for a given area only. */
void CScreenCapture::openCapture(int x, int y, int width, int height)
{
	m_capturePos.x = x;
	m_capturePos.y = y;

	m_captureSize.width = width;
	m_captureSize.height = height;

	if (hCaptureBitmap)
	{
		DeleteObject(hCaptureBitmap);
		hCaptureBitmap=NULL;
	}
	hCaptureBitmap =CreateCompatibleBitmap(hDesktopDC, m_captureSize.width, m_captureSize.height);
    SelectObject(hCaptureDC,hCaptureBitmap); 
}


CvImage CScreenCapture::captureNow()
{
	// Capture the bitmap:
    BitBlt(
		hCaptureDC,
		0,0,
		m_captureSize.width,
		m_captureSize.height,
		hDesktopDC,
		m_capturePos.x,
		m_capturePos.y, 
		SRCCOPY|CAPTUREBLT);

	// Convert BMP -> IPL:
	BITMAP bmp;
	if (!GetObject( hCaptureBitmap, sizeof(BITMAP), &bmp)) {
		throw std::logic_error("Could not retrieve bitmap info");
	}
	MY_ASSERT(bmp.bmBitsPixel==32);


	BITMAPINFOHEADER   bi;

	bi.biSize = sizeof(BITMAPINFOHEADER);    
	bi.biWidth = bmp.bmWidth;    
	bi.biHeight = bmp.bmHeight;  
	bi.biPlanes = 1;    
	bi.biBitCount = 32;    
	bi.biCompression = BI_RGB;    
	bi.biSizeImage = 0;  
	bi.biXPelsPerMeter = 0;    
	bi.biYPelsPerMeter = 0;    
	bi.biClrUsed = 0;    
	bi.biClrImportant = 0;

	DWORD dwBmpSize = ((bmp.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmp.bmHeight;
	void *bmpdata = malloc( dwBmpSize+ 1000  );

	// Gets the "bits" from the bitmap and copies them into a buffer 
	// which is pointed to by lpbi
	if (!GetDIBits( hCaptureDC,hCaptureBitmap,0 ,bmp.bmHeight, bmpdata,(BITMAPINFO *)&bi, DIB_RGB_COLORS) ) {
		throw std::logic_error("Could not retrieve bitmap bits");
	}

	// Reserve opencv image:
	IplImage *img = cvCreateImage(cvSize(bmp.bmWidth,bmp.bmHeight),IPL_DEPTH_8U,3);
	CvImage ret(img);

	// Copy memory to IPL buffer:
	for (int r=0;r<bmp.bmHeight;r++)
	{
		char *dst = ((char *)ret.data())+ ret.step()*r;
		char *src = ((char *)bmpdata)+ bmp.bmWidthBytes*(bmp.bmHeight-1-r);
		for (int c=0;c<bmp.bmWidth;c++)
		{
			*dst++=*src++;
			*dst++=*src++;
			*dst++=*src++;
			src++;
		}
	}
	free(bmpdata);
	return ret;
}

CScreenCapture::~CScreenCapture()
{
    ReleaseDC(hDesktopWnd,hDesktopDC);
    DeleteDC(hCaptureDC);
    DeleteObject(hCaptureBitmap);
}

void WriteBMPFile(HBITMAP bitmap, const char *filename, HDC hDC)
{
BITMAP bmp;
PBITMAPINFO pbmi;
WORD cClrBits;
HANDLE hf; // file handle
BITMAPFILEHEADER hdr; // bitmap file-header
PBITMAPINFOHEADER pbih; // bitmap info-header
LPBYTE lpBits; // memory pointer
DWORD dwTotal; // total count of bytes
DWORD cb; // incremental count of bytes
BYTE *hp; // byte pointer
DWORD dwTmp;

// create the bitmapinfo header information

if (!GetObject( bitmap, sizeof(BITMAP), (LPSTR)&bmp)){
cout << ("Could not retrieve bitmap info");
return;
}

// Convert the color format to a count of bits.
cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);
if (cClrBits == 1)
cClrBits = 1;
else if (cClrBits <= 4)
cClrBits = 4;
else if (cClrBits <= 8)
cClrBits = 8;
else if (cClrBits <= 16)
cClrBits = 16;
else if (cClrBits <= 24)
cClrBits = 24;
else cClrBits = 32;
 

 
// Allocate memory for the BITMAPINFO structure.
if (cClrBits != 24)
pbmi = (PBITMAPINFO) LocalAlloc(LPTR,
sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<< cClrBits));
else
pbmi = (PBITMAPINFO) LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

// Initialize the fields in the BITMAPINFO structure.

pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
pbmi->bmiHeader.biWidth = bmp.bmWidth;
pbmi->bmiHeader.biHeight = bmp.bmHeight;
pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
if (cClrBits < 24)
pbmi->bmiHeader.biClrUsed = (1<<cClrBits);

// If the bitmap is not compressed, set the BI_RGB flag.
pbmi->bmiHeader.biCompression = BI_RGB;

// Compute the number of bytes in the array of color
// indices and store the result in biSizeImage.
pbmi->bmiHeader.biSizeImage = (pbmi->bmiHeader.biWidth + 7) /8 * pbmi->bmiHeader.biHeight * cClrBits;
// Set biClrImportant to 0, indicating that all of the
// device colors are important.
pbmi->bmiHeader.biClrImportant = 0;

// now open file and save the data
pbih = (PBITMAPINFOHEADER) pbmi;
lpBits = (LPBYTE) GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

if (!lpBits) {
cout << ("writeBMP::Could not allocate memory");
return;
}

// Retrieve the color table (RGBQUAD array) and the bits
if (!GetDIBits(hDC, HBITMAP(bitmap), 0, (WORD) pbih->biHeight, lpBits, pbmi,
DIB_RGB_COLORS)) {
cout << ("writeBMP::GetDIB error");
return;
}

// Create the .BMP file.
hf = CreateFile(filename, GENERIC_READ | GENERIC_WRITE, (DWORD) 0,
NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
(HANDLE) NULL);
if (hf == INVALID_HANDLE_VALUE){
cout << ("Could not create file for writing");
return;
}
hdr.bfType = 0x4d42; // 0x42 = "B" 0x4d = "M"
// Compute the size of the entire file.
hdr.bfSize = (DWORD) (sizeof(BITMAPFILEHEADER) +
pbih->biSize + pbih->biClrUsed
* sizeof(RGBQUAD) + pbih->biSizeImage);
hdr.bfReserved1 = 0;
hdr.bfReserved2 = 0;

// Compute the offset to the array of color indices.
hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER) +
pbih->biSize + pbih->biClrUsed
* sizeof (RGBQUAD);

// Copy the BITMAPFILEHEADER into the .BMP file.
if (!WriteFile(hf, (LPVOID) &hdr, sizeof(BITMAPFILEHEADER),
(LPDWORD) &dwTmp, NULL)) {
cout << ("Could not write in to file");
return;
}

// Copy the BITMAPINFOHEADER and RGBQUAD array into the file.
if (!WriteFile(hf, (LPVOID) pbih, sizeof(BITMAPINFOHEADER)
+ pbih->biClrUsed * sizeof (RGBQUAD),
(LPDWORD) &dwTmp, ( NULL))){
cout << ("Could not write in to file");
return;
}


// Copy the array of color indices into the .BMP file.
dwTotal = cb = pbih->biSizeImage;
hp = lpBits;
if (!WriteFile(hf, (LPSTR) hp, (int) cb, (LPDWORD) &dwTmp,NULL)){
cout << ("Could not write in to file");
return;
}

// Close the .BMP file.
if (!CloseHandle(hf)){
cout << ("Could not close file");
return;
}

// Free memory.
GlobalFree((HGLOBAL)lpBits);
}






