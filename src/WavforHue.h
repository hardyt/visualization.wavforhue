/*
*      Copyright (C) 2015-2016 Thomas M. Hardy
*      Copyright (C) 2003-2016 Team Kodi
*      http://kodi.tv
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#ifndef WAVFORHUE
#define WAVFORHUE

#include <string>
#include <sstream>
#include <vector>
#include <queue>

#ifndef _WIN32
#include <math.h>
#endif

// -- Fast Fourier Transforms --------------------------------------
#include "FFT.h"
// -- Fast Fourier Transforms --------------------------------------

// -- Time Analyzer ------------------------------------------------
#ifdef _WIN32
#include <Windows.h>
#endif
// -- Time Analyzer ------------------------------------------------

// -- json ---------------------------------------------------------
#include <cctype>
#include <functional>
#include <algorithm>
#include <iostream>
#include "json/json.h"
// -- json ---------------------------------------------------------

#define BUFFERSIZE 1024
#define NUM_FREQUENCIES (512)

// -- HTTP requests information ------------------------------------------
#pragma once
struct SocketData
{
  SocketData(): protocol("http"), port("80"), method("PUT"), json("") { }
  std::string protocol;
  std::string host;
  std::string port;
  std::string path;
  std::string method;
  std::string json;
};
// -- HTTP requests information ------------------------------------------


#pragma once
class WavforHue
{
public:
  // -- Sound Analyzer -----------------------------------------------------
  struct SoundData
  {
    float imm[2][3];                     // bass, mids, treble, no damping, for each channel (long-term average is 1)
    float avg[2][3];                     // bass, mids, treble, some damping, for each channel (long-term average is 1)
    float med_avg[2][3];                 // bass, mids, treble, more damping, for each channel (long-term average is 1)
    // float   long_avg[2][3];           // bass, mids, treble, heavy damping, for each channel (long-term average is 1)
    float fWaveform[2][576];             // Not all 576 are valid! - only NUM_WAVEFORM_SAMPLES samples are valid for each channel (note: NUM_WAVEFORM_SAMPLES is declared in shell_defines.h)
    float fSpectrum[2][NUM_FREQUENCIES]; // NUM_FREQUENCIES samples for each channel (note: NUM_FREQUENCIES is declared in shell_defines.h)
    float specImm[32];
    float specAvg[32];
    float specMedAvg[32];
    float bigSpecImm[512];
    float leftBigSpecAvg[512];
    float rightBigSpecAvg[512];
  };
  
  SoundData sound;
  float bass, bassLast;
  float treble, trebleLast;
  float middle, middleLast;
  float timePass;
  bool finished;
  float movingAvgMid[128];
  float movingAvgMidSum;
  // -- Sound Analyzer -----------------------------------------------------

  // -- Time Analyzer ------------------------------------------------------
  float fElapsedAppTime;
  // -- Time Analyzer ------------------------------------------------------

  // -- Hue Information ----------------------------------------------------
  struct HueData
  {
    int bri, sat, hue, transitionTime, numberOfLights;
    std::vector<std::string> lightIDs;
    bool on, off;
  };
  std::string strHueBridgeIPAddress, strHueBridgeUser;
  unsigned int hueBridgePort;
  HueData afterHueData, dimmedHueData, activeHueData;
  int lastHue, initialHue, targetHue, maxBri, targetBri, currentBri;
  float beatThreshold;
  std::queue<SocketData> queue;
  bool useWaveForm, priorState;
  std::vector<HueData> priorStates;
  bool savedTheStates;
  // -- Hue Information ----------------------------------------------------

  // -- Colors -------------------------------------------------------------
  float rgb[3];
  // -- Colors -------------------------------------------------------------

  // -- Fast Fourier Transforms --------------------------------------
  FFT fftobj;
  // -- Fast Fourier Transforms --------------------------------------

  // -- Workaround for OpenELEC imx6 ---------------------------------------
  // This is used if audiodata is not coming from Kodi nicely.
  // The problem is with Solidrun's Cubox (imx6) set to HDMI audio out. The 
  // Waveform visualisation has the right 1/4 of its waveforms flat because 
  // 0's are being reported by the visualisation API for that architecture.
  int iMaxAudioData_i;
  float fMaxAudioData;
  bool cuboxHDMIFix;
  // -- Workaround for OpenELEC imx6 ---------------------------------------

  // -- Debug --------------------------------------------------------------
  bool debug;
  // -- Debug --------------------------------------------------------------


  // -- Functions ----------------------------------------------------------
  void AnalyzeSound();
  void TurnLightsOn(HueData hueData);
  void TurnLightsOff(HueData hueData);
  void UpdateLights(HueData hueData);
  void UpdateTime();
  void Start();
  void Stop();
  void SaveState(std::string json);
  void RestoreState();
  void SendDebug(std::string);
  WavforHue();
  ~WavforHue();
  // -- Functions ----------------------------------------------------------

private:
  // -- Time Analyzer ------------------------------------------------------
#ifdef _WIN32
  FLOAT fSecsPerTick;
  LARGE_INTEGER qwTime, qwLastTime, qwLightTime, qwElapsedTime, qwAppTime, qwElapsedAppTime;
#else
  struct timespec systemClock;
#endif
  float fTime, fElapsedTime, fAppTime, fUpdateTime, fLastTime, fLightTime;
  int iFrames = 0;
  float fFPS = 0;
  // -- Time Analyzer ------------------------------------------------------

  // -- Hue Information ----------------------------------------------------
  std::vector<HueData> priorHueData;
  // -- Hue Information ----------------------------------------------------

  // -- Debug --------------------------------------------------------------
  
  // -- Debug --------------------------------------------------------------


  // -- Functions ----------------------------------------------------------
  void hsvToRgb(float h, float s, float v, float _rgb[]);
  void huePutRequest(HueData hueData);
  void AdjustBrightness();
  void FastBeatLights();
  void SlowBeatLights();
  void CycleHue(int huePoints);
  void CycleLights();
  float AdjustRateToFPS(float per_frame_decay_rate_at_fps1, float fps1, float actual_fps);
  void InitTime();  
  // -- Functions ----------------------------------------------------------


};

#endif
