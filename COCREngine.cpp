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

#include "COCREngine.h"

#undef EXIT  // Yes, I know...

#include "mfcpch.h"
#include "applybox.h"
#include "control.h"
#include "tessvars.h"
#include "tessedit.h"
#include "baseapi.h"
#include "pageres.h"
#include "imgs.h"
#include "varabled.h"
#include "tprintf.h"
#include "tesseractmain.h"
#include "stderr.h"
#include "notdll.h"
#include "mainblk.h"
#include "output.h"
#include "globals.h"
#include "blread.h"
#include "tfacep.h"
#include "callnet.h"

using namespace std;

extern void TesseractImage(const char* input_file, IMAGE* image, STRING* text_out);

int  COCREngine::m_nInstances = 0;
bool COCREngine::m_init = false;


COCREngine::COCREngine()
{
	m_nInstances++;
}

COCREngine::~COCREngine()
{
	if (!--m_nInstances)
	{
		m_init = false;
		TessBaseAPI::End();
	}
}

/** Runs OCR on an image and returns the detected characters 
  * \return false on error
  */
bool COCREngine::ocr_on_image(const CvImage &img, std::string &out_text, bool removeWhitespaces)
{
	// init (MUST NOT be done at constructor since it will crash if an object is created in "global scope"):
	if (!m_init)
	{
		m_init=true;
		const char* lang = "eng";
		TessBaseAPI::InitWithLanguage(
			".",	// Data path
			NULL,	// Output base
			lang,
			NULL,	// cfg file
			false,
			0,NULL); // Extra args
		tprintf ("[COCREngine] Tesseract Open Source OCR Engine is up\n");
	}


	// IPL-> IMAGE structure:
	// ----------------------------
	IMAGE image;
#if 1
	const char *tmp_fil=".tmpfil.tif";
	cvSaveImage(tmp_fil,img);

	if (image.read_header(tmp_fil) < 0) {
		return false;
	}
	if (image.read(image.get_ysize()) < 0) {
		return false;
    }
	remove(tmp_fil);
#else
	const size_t row_bytes = img.width()*img.channels()*(img.depth()/8);
	char *new_buf= new char[  row_bytes * img.height() ];
	for (int r=0;r<img.height();r++)
		memcpy(new_buf+r*row_bytes, img.data()+img.step()*r, row_bytes);

	image.capture( (uinT8*)new_buf,img.width(),img.height(),img.depth(),true);
#endif

	// Do OCR:
	STRING text_out;
	TesseractImage("dummy", &image, &text_out);

	// Save output:
	out_text=text_out.string();

	if (removeWhitespaces) 
		out_text= str_trim(str_uppercase( out_text ));

	return true;
}

