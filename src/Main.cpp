/*
 *      Copyright (C) 2008-2016 Team Kodi
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


//--------------------------------------------------------------------------------------
#include <xbmc_vis_dll.h>
#include <stdio.h>
#ifdef _WIN32
#include <winsock2.h>
#endif
#include <curl/curl.h>
#include <string>
//------------------------------------------------------------------


//------------------------------------------------------------------
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
//----------------------------------------------------------------------


// Thread initialization -------------------------------------------------
std::mutex gMutex;
std::condition_variable gThreadConditionVariable;
std::atomic<bool> gRunThread;
bool gReady;
std::thread gWorkerThread;
std::queue<int> gQueue;
// End thread initialization ---------------------------------------------


void workerThread()
{
  bool isEmpty;
  std::string json;
  // This thread comes alive when AudioData() has put an item in the stack
  // It runs until Destroy() sets gRunThread to false and joins it
  while (gRunThread)
  {
    //check that an item is on the stack
    
    {
      std::lock_guard<std::mutex> lock(gMutex);
      isEmpty = gQueue.empty();
    }

    if (isEmpty)
    {
      //Wait until AudioData() sends data.
      std::unique_lock<std::mutex> lock(gMutex);
      gThreadConditionVariable.wait(lock, []{return gReady; });
    }
    else
    {
      std::lock_guard<std::mutex> lock(gMutex);
      int value = gQueue.front();
      gQueue.pop();
    }
    if (!isEmpty)
    {
      /*
      CURL *curl = curl_easy_init();
      CURLcode res;
      json = "{\"hue\":" + std::to_string(rand() % 60000) + "}";
      // Now specify we want to PUT data, but not using a file, so it has to be a CUSTOMREQUEST
      curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
      curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
      //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, noop_cb);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json.c_str());
      // Set the URL that is about to receive our POST. 
      curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.10.6/api/KodiVisWave/lights/3/state");
      // Perform the request, res will get the return code
      res = curl_easy_perform(curl);
      // always cleanup curl
      curl_easy_cleanup(curl);
      */
    }
  }
}




//-- Create -------------------------------------------------------------------
// Called on load. Addon should fully initalize or return error status
//-----------------------------------------------------------------------------
extern "C" ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  if (!props)
    return ADDON_STATUS_UNKNOWN;
  
  gRunThread = true;
  gReady = false;

  // Check if the thread is alive yet.
  if (!gWorkerThread.joinable())
  {
    gWorkerThread = std::thread(&workerThread);
  }

  // Must initialize libcurl before any threads are started.
  //curl_global_init(CURL_GLOBAL_ALL);

  return ADDON_STATUS_OK;
}

//-- Start --------------------------------------------------------------------
// Called when a new soundtrack is played
//-----------------------------------------------------------------------------
extern "C" void Start(int iChannels, int iSamplesPerSec, int iBitsPerSample, const char* szSongName)
{
  gRunThread = true;
  // Check if the thread is alive yet.
  if (!gWorkerThread.joinable())
  {
    gWorkerThread = std::thread(&workerThread);
  }
}

//-- Audiodata ----------------------------------------------------------------
// Called by XBMC to pass new audio data to the vis
//-----------------------------------------------------------------------------
extern "C" void AudioData(const float* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
  // Processing audio data
  if (rand() % 7 == 3)
  {
    std::lock_guard<std::mutex> lock(gMutex);
    gQueue.push(1);
  }

  gRunThread = true;
  // Check if the thread is alive yet.
  if (!gWorkerThread.joinable())
  {
    gWorkerThread = std::thread(&workerThread);
  }

  // Send the curl calls to the worker thread
  {
    std::lock_guard<std::mutex> lock(gMutex);
    gReady = true;
  }
  gThreadConditionVariable.notify_one();

}

//-- Stop ---------------------------------------------------------------------
// This dll must stop all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" void ADDON_Stop()
{
  gRunThread = false;
  while (gWorkerThread.joinable())
  {
    gWorkerThread.join();
  }
}

//-- Detroy -------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" void ADDON_Destroy()
{
  gRunThread = false;
  while (gWorkerThread.joinable())
  {
    gWorkerThread.join();
  }
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
extern "C" void Render()
{

}

//-- GetInfo ------------------------------------------------------------------
// Tell XBMC our requirements
//-----------------------------------------------------------------------------
extern "C" void GetInfo(VIS_INFO* pInfo)
{
  pInfo->bWantsFreq = false;
  pInfo->iSyncDelay = 0;
}

//-- OnAction -----------------------------------------------------------------
// Handle XBMC actions such as next preset, lock preset, album art changed etc
//-----------------------------------------------------------------------------
extern "C" bool OnAction(long flags, const void *param)
{
  bool ret = false;
  return ret;
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to XBMC for display
//-----------------------------------------------------------------------------
extern "C" unsigned int GetPresets(char ***presets)
{
  return 0;
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
extern "C" unsigned GetPreset()
{
  return 0;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
extern "C" bool IsLocked()
{
  return false;
}

//-- GetSubModules ------------------------------------------------------------
// Return any sub modules supported by this vis
//-----------------------------------------------------------------------------
extern "C" unsigned int GetSubModules(char ***names)
{
  return 0; // this vis supports 0 sub modules
}

//-- HasSettings --------------------------------------------------------------
// Returns true if this add-on use settings
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" bool ADDON_HasSettings()
{
  return false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" ADDON_STATUS ADDON_GetStatus()
{
  return ADDON_STATUS_OK;
}

//-- GetSettings --------------------------------------------------------------
// Return the settings for XBMC to display
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

//-- FreeSettings --------------------------------------------------------------
// Free the settings struct passed from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------

extern "C" void ADDON_FreeSettings()
{
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void* value)
{
  return ADDON_STATUS_OK;
}

//-- Announce -----------------------------------------------------------------
// Receive announcements from XBMC
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}
