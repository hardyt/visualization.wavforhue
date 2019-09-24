/*
*      Copyright (C) 2015-2016 Thomas M. Hardy
*      Copyright (C) 2005-2016 Team Kodi
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

#include "WavforHue.h"

#include <kodi/General.h>

// -- trim ---------------------------------------------------------
// trim from start
static inline std::string &ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
  return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
  return ltrim(rtrim(s));
}
// -- trim ---------------------------------------------------------


// -- Constructor ---------------------------------------------------- 
WavforHue::WavforHue()
{
  // -- Hue Information ----------------------------------------------------
  useWaveForm = true;
  strHueBridgeIPAddress = "192.168.10.6";
  strHueBridgeIPAddress = "KodiVisWave";
  currentBri = 75;
  beatThreshold = 0.25f;
  activeHueData.numberOfLights = 3;
  dimmedHueData.numberOfLights = 2;
  dimmedHueData.bri = 10; 
  dimmedHueData.sat = 255; 
  dimmedHueData.hue = 65280;
  afterHueData.numberOfLights = 1;
  afterHueData.bri = 25; 
  afterHueData.sat = 255; 
  afterHueData.hue = 65280;
  priorState = false;
  savedTheStates = false;
  // -- Hue Information ----------------------------------------------------

  // -- Colors -------------------------------------------------------------
  rgb[0] = 1.0f; rgb[1] = 1.0f; rgb[2] = 1.0f;
  // -- Colors -------------------------------------------------------------

  // -- Workaround for OpenELEC imx6 ---------------------------------------
  cuboxHDMIFix = false;
  iMaxAudioData_i = 256;
  fMaxAudioData = 255.0f;
  // -- Workaround for OpenELEC imx6 ---------------------------------------

  // -- Time Analyzer ------------------------------------------------
  InitTime();
  // -- Time Analyzer ------------------------------------------------

  // -- Fast Fourier Transforms --------------------------------------
  fftobj.Init(576, NUM_FREQUENCIES);
  // -- Fast Fourier Transforms --------------------------------------

  // -- Sound Analyzer -----------------------------------------------------
  //initialize the moving average of mids
  for (int i = 0; i<15; i++)
  {
    movingAvgMid[i] = 0;
  }
  // -- Sound Analyzer -----------------------------------------------------

  // -- Debug --------------------------------------------------------------
  debug = false;
  // -- Debug --------------------------------------------------------------
}
// -- Constructor ---------------------------------------------------- 

// -- Destructor ----------------------------------------------------- 
WavforHue::~WavforHue()
{
}
// -- Destructor ----------------------------------------------------- 


// -- Hue and color functions -----------------------------------------------
// hsv to rgb conversion
void WavforHue::hsvToRgb(float h, float s, float v, float _rgb[]) {
  float r = 0.0f, g = 0.0f, b = 0.0f;

  int i = int(h * 6);
  float f = h * 6 - i;
  float p = v * (1 - s);
  float q = v * (1 - f * s);
  float t = v * (1 - (1 - f) * s);

  switch (i % 6){
  case 0: r = v, g = t, b = p; break;
  case 1: r = q, g = v, b = p; break;
  case 2: r = p, g = v, b = t; break;
  case 3: r = p, g = q, b = v; break;
  case 4: r = t, g = p, b = v; break;
  case 5: r = v, g = p, b = q; break;
  }

  _rgb[0] = r;
  _rgb[1] = g;
  _rgb[2] = b;
}

void WavforHue::SaveState(std::string json)
{
  // Let's parse it  
  Json::Value root;
  Json::Reader reader;
  bool parsedSuccess = reader.parse(trim(json),
    root,
    false);

  if (!parsedSuccess)
  {
    // Report failures and their locations in the document.
    // Oh I wish I could use debugging in here.
    SendDebug("Failed to parse JSON " + reader.getFormattedErrorMessages());
    return;
  }

  if (root.size() > 0) {
    for (Json::ValueIterator itr = root.begin(); itr != root.end(); itr++) {
      HueData hueData;
      hueData.lightIDs.push_back(itr.key().asString());
      hueData.numberOfLights = 1;
      if (root[itr.key().asString()]["state"]["on"].asString() == "false")
      {
        // The light was off.
        hueData.off = true;
        hueData.on = false;
      }
      else
      {
        // The light was on.
        hueData.off = false;
        hueData.on = true;
        priorStates.push_back(hueData);
        hueData.on = false;
        hueData.bri = root[itr.key().asString()]["state"]["bri"].asInt();
        hueData.hue = root[itr.key().asString()]["state"]["hue"].asInt();
        hueData.sat = root[itr.key().asString()]["state"]["sat"].asInt();
        hueData.transitionTime = 15;
      }
      
      priorStates.push_back(hueData);
      SendDebug("Saving hue data for light " + itr.key().asString());
    }
  }
  savedTheStates = true;
}

void WavforHue::RestoreState()
{
  // Put all the prior states on the queue.
  for (auto &i : priorStates)
  {
    huePutRequest(i);
    SendDebug("Restoring hue data for light " + i.lightIDs[0]);
  }
}

void WavforHue::huePutRequest(HueData hueData)
{
  std::string strJson, strURLLight;
  SocketData _putData;

  if (hueData.on) //turn lights on
    strJson = "{\"on\":true}";
  else if (hueData.off) //turn lights off
    strJson = "{\"on\":false}";
  else if (hueData.sat > 0) //change saturation
  {
    std::ostringstream oss;
    oss << "{\"bri\":" << hueData.bri << ",\"hue\":" << hueData.hue <<
      ",\"sat\":" << hueData.sat << ",\"transitiontime\":"
      << hueData.transitionTime << "}";
    strJson = oss.str();
  }
  else //change lights
  {
    std::ostringstream oss;
    oss << "{\"bri\":" << hueData.bri << ",\"hue\":" << hueData.hue <<
      ",\"transitiontime\":" << hueData.transitionTime << "}";
    strJson = oss.str();
  }

  for (int i = 0; i < hueData.numberOfLights; i++)
  {
    _putData.host = strHueBridgeIPAddress;
    _putData.port = std::to_string(hueBridgePort);
    _putData.path = "/api/" + strHueBridgeUser + "/lights/" + hueData.lightIDs[i] + "/state";
    _putData.method = "PUT";
    _putData.json = strJson;
    //if (queue.size() < 10)
    //{
      queue.push(_putData);
    //}
  }
}

void WavforHue::TurnLightsOn(HueData hueData)
{
  hueData.bri = 0;
  hueData.sat = 0;
  hueData.hue = 0;
  hueData.transitionTime = 0;
  hueData.on = true;
  hueData.off = false;
  huePutRequest(hueData);
}

void WavforHue::TurnLightsOff(HueData hueData)
{
  hueData.bri = 0;
  hueData.sat = 0;
  hueData.hue = 0;
  hueData.transitionTime = 0;
  hueData.on = false;
  hueData.off = true;
  huePutRequest(hueData);
}

void WavforHue::UpdateLights(HueData hueData)
{
  hueData.on = false;
  hueData.off = false;
  huePutRequest(hueData);
}

void WavforHue::AdjustBrightness() //nicely bring the brightness up or down
{
  int briDifference = currentBri - targetBri;
  if (briDifference > 7) currentBri = currentBri - 7;
  else if (briDifference < -7) currentBri = currentBri + 7;
  else currentBri = targetBri;
}

void WavforHue::FastBeatLights()
{
  HueData hueData;
  AdjustBrightness();
  //figure out a good brightness increase
  int beatBri = (int)(currentBri * 1.5f);
  if (beatBri > 255) beatBri = 255;
  //transition the color immediately
  hueData.bri = beatBri;
  hueData.sat = 0;
  hueData.hue = lastHue;
  hueData.transitionTime = 0;
  hueData.lightIDs = activeHueData.lightIDs;
  hueData.numberOfLights = activeHueData.numberOfLights;
  UpdateLights(hueData);
  //fade brightness
  hueData.bri = 5;
  hueData.sat = 0;
  hueData.hue = lastHue;
  hueData.transitionTime = 10;
  UpdateLights(hueData);
}

void WavforHue::SlowBeatLights()
{
  HueData hueData;
  AdjustBrightness();
  //figure out a good brightness increase
  int beatBri = (int)(currentBri * 1.25f);
  if (beatBri > 255) beatBri = 255;
  //transition the color immediately
  hueData.bri = beatBri;
  hueData.sat = 0;
  hueData.hue = lastHue;
  hueData.transitionTime = 2;
  hueData.lightIDs = activeHueData.lightIDs;
  hueData.numberOfLights = activeHueData.numberOfLights;
  UpdateLights(hueData);
  //fade brightness
  hueData.bri = 5;
  hueData.sat = 0;
  hueData.hue = lastHue;
  hueData.transitionTime = 8;
  UpdateLights(hueData); 
}

void WavforHue::CycleHue(int huePoints)
{
  int hueGap;
  if ((lastHue - targetHue) > 0) hueGap = lastHue - targetHue;
  else hueGap = (lastHue - targetHue) * -1;
  if (hueGap > huePoints)
  {
    if (lastHue > targetHue) lastHue = lastHue - huePoints;
    else lastHue = lastHue + huePoints;
  }
  else
  {
    lastHue = targetHue;
    targetHue = initialHue;
    initialHue = lastHue;
  }
  //for the waveform to match the lights
  hsvToRgb(((float)lastHue / 65535.0f), 1.0f, 1.0f, rgb);
}

void WavforHue::CycleLights()
{
  HueData hueData;
  //this is called once per second if no beats are detected
  CycleHue(3000);
  AdjustBrightness();
  hueData.bri = currentBri;
  hueData.sat = 0;
  hueData.hue = lastHue;
  hueData.transitionTime = 10;
  hueData.lightIDs = activeHueData.lightIDs;
  hueData.numberOfLights = activeHueData.numberOfLights;
  UpdateLights(hueData);
}

void WavforHue::Start()
{
  // Turn active lights on
  TurnLightsOn(activeHueData);
  activeHueData.bri = currentBri;
  activeHueData.sat = 255;
  activeHueData.hue = lastHue;
  activeHueData.transitionTime = 30;
  UpdateLights(activeHueData);
  // Dim lights
  if (dimmedHueData.numberOfLights > 0)
  {
    TurnLightsOn(dimmedHueData);
    dimmedHueData.transitionTime = 30;
    UpdateLights(dimmedHueData);
  }
  if (cuboxHDMIFix)
  {
    iMaxAudioData_i = 180;
    fMaxAudioData = 179.0f;
  }
}

void WavforHue::Stop()
{
  if (afterHueData.numberOfLights>0)
  {
    TurnLightsOff(activeHueData);
    if (dimmedHueData.numberOfLights>0)
    {
      TurnLightsOff(dimmedHueData);
    }
    TurnLightsOn(afterHueData);
    afterHueData.transitionTime = 30;
    UpdateLights(afterHueData);
  }
  else
  {
    TurnLightsOff(activeHueData);
    if (dimmedHueData.numberOfLights>0)
    {
      TurnLightsOff(dimmedHueData);
    }
  }
}
// -- Hue and color functions -----------------------------------------------


// -- Sound Analyzer -----------------------------------------------------
// Modified from Vortex
float WavforHue::AdjustRateToFPS(float per_frame_decay_rate_at_fps1, float fps1, float actual_fps)
{
  // returns the equivalent per-frame decay rate at actual_fps

  // basically, do all your testing at fps1 and get a good decay rate;
  // then, in the real application, adjust that rate by the actual fps each time you use it.

  float per_second_decay_rate_at_fps1 = powf(per_frame_decay_rate_at_fps1, fps1);
  float per_frame_decay_rate_at_fps2 = powf(per_second_decay_rate_at_fps1, 1.0f / actual_fps);

  return per_frame_decay_rate_at_fps2;
}

// Modified from Vortex
void WavforHue::AnalyzeSound()
{
  int m_fps = 60;

  // sum (left channel) spectrum up into 3 bands
  // [note: the new ranges do it so that the 3 bands are equally spaced, pitch-wise]
  float min_freq = 200.0f;
  float max_freq = 11025.0f;
  float net_octaves = (logf(max_freq / min_freq) / logf(2.0f));     // 5.7846348455575205777914165223593
  float octaves_per_band = net_octaves / 3.0f;                    // 1.9282116151858401925971388407864
  float mult = powf(2.0f, octaves_per_band); // each band's highest freq. divided by its lowest freq.; 3.805831305510122517035102576162
  // [to verify: min_freq * mult * mult * mult should equal max_freq.]
  //    for (int ch=0; ch<2; ch++)
  {
    for (int i = 0; i<3; i++)
    {
      // old guesswork code for this:
      //   float exp = 2.1f;
      //   int start = (int)(NUM_FREQUENCIES*0.5f*powf(i/3.0f, exp));
      //   int end   = (int)(NUM_FREQUENCIES*0.5f*powf((i+1)/3.0f, exp));
      // results:
      //          old range:      new range (ideal):
      //   bass:  0-1097          200-761
      //   mids:  1097-4705       761-2897
      //   treb:  4705-11025      2897-11025
      int start = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)i) / 11025.0f);
      int end = (int)(NUM_FREQUENCIES * min_freq*powf(mult, (float)i + 1) / 11025.0f);
      if (start < 0) start = 0;
      if (end > NUM_FREQUENCIES) end = NUM_FREQUENCIES;

      sound.imm[0][i] = 0;
      for (int j = start; j<end; j++)
      {
        sound.imm[0][i] += sound.fSpectrum[0][j];
        sound.imm[0][i] += sound.fSpectrum[1][j];
      }
      sound.imm[0][i] /= (float)(end - start) * 2;
    }
  }

  // multiply by long-term, empirically-determined inverse averages:
  // (for a trial of 244 songs, 10 seconds each, somewhere in the 2nd or 3rd minute,
  //  the average levels were: 0.326781557	0.38087377	0.199888934
  for (int ch = 0; ch<2; ch++)
  {
    sound.imm[ch][0] /= 0.326781557f;//0.270f;   
    sound.imm[ch][1] /= 0.380873770f;//0.343f;   
    sound.imm[ch][2] /= 0.199888934f;//0.295f;   
  }

  // do temporal blending to create attenuated and super-attenuated versions
  for (int ch = 0; ch<2; ch++)
  {
    for (int i = 0; i<3; i++)
    {
      // sound.avg[i]
      {
        float avg_mix;
        if (sound.imm[ch][i] > sound.avg[ch][i])
          avg_mix = AdjustRateToFPS(0.2f, 14.0f, (float)m_fps);
        else
          avg_mix = AdjustRateToFPS(0.5f, 14.0f, (float)m_fps);
        //                if (sound.imm[ch][i] > sound.avg[ch][i])
        //                  avg_mix = 0.5f;
        //                else 
        //                  avg_mix = 0.8f;
        sound.avg[ch][i] = sound.avg[ch][i] * avg_mix + sound.imm[ch][i] * (1 - avg_mix);
      }

      {
        float med_mix = 0.91f;//0.800f + 0.11f*powf(t, 0.4f);    // primarily used for velocity_damping
        float long_mix = 0.96f;//0.800f + 0.16f*powf(t, 0.2f);    // primarily used for smoke plumes
        med_mix = AdjustRateToFPS(med_mix, 14.0f, (float)m_fps);
        long_mix = AdjustRateToFPS(long_mix, 14.0f, (float)m_fps);
        sound.med_avg[ch][i] = sound.med_avg[ch][i] * (med_mix)+sound.imm[ch][i] * (1 - med_mix);
        //                sound.long_avg[ch][i] = sound.long_avg[ch][i]*(long_mix) + sound.imm[ch][i]*(1-long_mix);
      }
    }
  }

  float newBass = ((sound.avg[0][0] - sound.med_avg[0][0]) / sound.med_avg[0][0]) * 2;
  float newMiddle = ((sound.avg[0][1] - sound.med_avg[0][1]) / sound.med_avg[0][1]) * 2;
  float newTreble = ((sound.avg[0][2] - sound.med_avg[0][2]) / sound.med_avg[0][2]) * 2;

  newBass = std::max(std::min(newBass, 1.0f), -1.0f);
  newMiddle = std::max(std::min(newMiddle, 1.0f), -1.0f);
  newTreble = std::max(std::min(newTreble, 1.0f), -1.0f);

  bassLast = bass;
  middleLast = middle;

  float avg_mix;
  if (newTreble > treble)
    avg_mix = 0.5f;
  else
    avg_mix = 0.5f;

  //dealing with NaN's in linux
  if (bass != bass) bass = 0;
  if (middle != middle) middle = 0;
  if (treble != treble) treble = 0;

  bass = bass*avg_mix + newBass*(1 - avg_mix);
  middle = middle*avg_mix + newMiddle*(1 - avg_mix);
  //treble = treble*avg_mix + newTreble*(1 - avg_mix);

  bass = std::max(std::min(bass, 1.0f), -1.0f);
  middle = std::max(std::min(middle, 1.0f), -1.0f);
  //treble = std::max(std::min(treble, 1.0f), -1.0f);

  if (middle < 0) middle = middle * -1.0f;
  if (bass < 0) bass = bass * -1.0f;

  if (((middle - middleLast) > beatThreshold ||
    (bass - bassLast > beatThreshold))
    && ((fAppTime - fLightTime) > 0.3f))
  {
    //beat
    FastBeatLights();
    CycleHue(1500);
    //changed lights
    fLightTime = fAppTime;
  }
}
// -- Sound Analyzer -----------------------------------------------------

// -- Time Analyzer ------------------------------------------------------
void WavforHue::InitTime()
{
#ifdef _WIN32
  // Get the frequency of the timer
  LARGE_INTEGER qwTicksPerSec;
  QueryPerformanceFrequency(&qwTicksPerSec);
  fSecsPerTick = 1.0f / (FLOAT)qwTicksPerSec.QuadPart;

  // Save the start time
  QueryPerformanceCounter(&qwTime);
  qwLastTime.QuadPart = qwTime.QuadPart;
  qwLightTime.QuadPart = qwTime.QuadPart;

  qwAppTime.QuadPart = 0;
  qwElapsedTime.QuadPart = 0;
  qwElapsedAppTime.QuadPart = 0;
  srand((unsigned int)qwTime.QuadPart);
#else
  // Save the start time
  clock_gettime(CLOCK_MONOTONIC, &systemClock);
  fTime = ((float)systemClock.tv_nsec / 1000000000.0) + (float)systemClock.tv_sec;
#endif

  fAppTime = 0;
  fElapsedTime = 0;
  fElapsedAppTime = 0;
  fLastTime = 0;
  fLightTime = 0;
  fUpdateTime = 0;

}

void WavforHue::UpdateTime()
{
#ifdef _WIN32
  QueryPerformanceCounter(&qwTime);
  qwElapsedTime.QuadPart = qwTime.QuadPart - qwLastTime.QuadPart;
  qwLastTime.QuadPart = qwTime.QuadPart;
  qwLightTime.QuadPart = qwTime.QuadPart;
  qwElapsedAppTime.QuadPart = qwElapsedTime.QuadPart;
  qwAppTime.QuadPart += qwElapsedAppTime.QuadPart;

  // Store the current time values as floating point
  fTime = fSecsPerTick * ((FLOAT)(qwTime.QuadPart));
  fElapsedTime = fSecsPerTick * ((FLOAT)(qwElapsedTime.QuadPart));
  fAppTime = fSecsPerTick * ((FLOAT)(qwAppTime.QuadPart));
  fElapsedAppTime = fSecsPerTick * ((FLOAT)(qwElapsedAppTime.QuadPart));
#else
  clock_gettime(CLOCK_MONOTONIC, &systemClock);
  fTime = ((float)systemClock.tv_nsec / 1000000000.0) + (float)systemClock.tv_sec;
  fElapsedTime = fTime - fLastTime;
  fLastTime = fTime;
  fAppTime += fElapsedTime;
#endif

  // Keep track of the frame count
  iFrames++;

  //fBeatTime = 60.0f / (float)(bpm); //skip every other beat

  // If beats aren't doing anything then cycle colors nicely
  if (fAppTime - fLightTime > 1.5f)
  {
    CycleLights();
    fLightTime = fAppTime;
  }

  movingAvgMidSum = 0.0f;
  //update the max brightness based on the moving avg of the mid levels
  for (int i = 0; i<128; i++)
  {
    movingAvgMidSum += movingAvgMid[i];
    if (i != 127)
      movingAvgMid[i] = movingAvgMid[i + 1];
    else
      movingAvgMid[i] = (sound.avg[0][1] + sound.avg[1][1]) / 2.0f;
  }

  if ((movingAvgMidSum*1000.0f / 15.0f) < 0.5f &&
    (movingAvgMidSum*1000.0f / 15.0f) > 0.1f)
    targetBri = (int)(maxBri * 2 * movingAvgMidSum*1000.0f / 15.0f);
  else if (movingAvgMidSum*1000.0f / 15.0f > 0.5f)
    targetBri = maxBri;
  else if (movingAvgMidSum*1000.0f / 15.0f < 0.1f)
    targetBri = (int)(maxBri * 0.1f);

  // Update the scene stats once per second
  if (fAppTime - fUpdateTime > 1.0f)
  {
    fFPS = (float)(iFrames / (fAppTime - fLastTime));
    fUpdateTime = fAppTime;
    iFrames = 0;
  }
}
// -- Time Analyzer ------------------------------------------------------

// -- Debug --------------------------------------------------------------
void WavforHue::SendDebug(std::string mStrDebug)
{
#ifndef ANDROID
  kodi::Log(ADDON_LOG_DEBUG, mStrDebug.c_str());
#endif
}
