# --------------------------------------------------------------------
#  Facebook Geo-Challenge AI automatic player software
#
#  Written by: 
#   Jose-Luis Blanco-Claraco-Claraco  <joseluisblancoc at gmail dot com>
#   AUG 2009 - Spain
#
#  This file is part of Geo-Challenge AI.
# 
#     Geo-Challenge AI is free software: you can redistribute it and/or 
#     modify it under the terms of the GNU General Public License as 
#     published by the Free Software Foundation, either version 3 of 
#     the License, or (at your option) any later version.
# 
#     Geo-Challenge AI is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
# 
#     You should have received a copy of the GNU General Public License
#     along with Geo-Challenge AI.  If not, see <http://www.gnu.org/licenses/>.
# --------------------------------------------------------------------

PROJECT(geochallenge_ai)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

FIND_PACKAGE( OpenCV REQUIRED )

# Add all the sources of tesseract:
FILE (GLOB_RECURSE tesseract_ocr_files  ${CMAKE_SOURCE_DIR}/tesseract_ocr/*.cpp ${CMAKE_SOURCE_DIR}/tesseract_ocr/*.h)

set(src_files
	screen_capture.cpp screen_capture.h
	CTicTac.cpp	CTicTac.h
	utils.cpp utils.h
	COCREngine.cpp COCREngine.h
	level1.cpp 
	level2.cpp
	level3.cpp
	level4.cpp
	waitmainscreen.cpp
	main.cpp
	data.h
	)

SOURCE_GROUP("tesseract (OCR engine)" FILES ${tesseract_ocr_files})


INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/ccmain")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/ccstruct")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/ccutil")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/classify")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/cutil")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/dict")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/image")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/pageseg")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/textord")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/viewer")
INCLUDE_DIRECTORIES("${CMAKE_SOURCE_DIR}/tesseract_ocr/wordrec")

IF(WIN32)
	ADD_DEFINITIONS(/D__MSW32__)
	ADD_DEFINITIONS(/D_CRT_SECURE_NO_WARNINGS)
ENDIF(WIN32)

# Declare the target (an executable)
ADD_EXECUTABLE(geochallenge_ai
	${src_files}
	${tesseract_ocr_files}
	)
	
IF(WIN32)
	TARGET_LINK_LIBRARIES(geochallenge_ai ws2_32)
ENDIF(WIN32)

TARGET_LINK_LIBRARIES(geochallenge_ai ${OpenCV_LIBS})

