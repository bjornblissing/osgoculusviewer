/*
 * oculusconfig.h
 *
 *  Created on: Dec 29, 2020
 *      Author: Riccardo Corsi
 */

#pragma once


// define the dll export directives based on CMake option
#ifdef OSGOCULUSLIB_BUILD_SHARED
#  if defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined( __BCPLUSPLUS__)  || defined( __MWERKS__)
#     ifdef OsgOculus_EXPORTS
#        define OSGOCULUS_EXPORT __declspec(dllexport)
#     else
#        define OSGOCULUS_EXPORT __declspec(dllimport)
#     endif
#  endif
#endif

#ifndef OSGOCULUS_EXPORT
#define OSGOCULUS_EXPORT
#endif
