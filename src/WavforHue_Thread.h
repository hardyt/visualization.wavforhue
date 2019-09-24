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

#ifndef WAVFORHUE_THREAD
#define WAVEFORHUE_THREAD

// -- cURL works in *nix, but is crap in Windows -------------------
#ifndef _WIN32
#include <curl/curl.h>
#else
// -- cURL works in *nix, but is crap in Windows -------------------
#define _WIN32_WINNT 0x0501
#include <Winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iostream>
#pragma comment (lib, "Ws2_32.lib")
#endif
/*
KS-  <afedchin>: hardyt_ why u want to use cURL you can just use  CHelper_libXBMC_addon *XBMC = new CHelper_libXBMC_addon; XBMC->OpenFile(url, flags)
7:20:23 AM	KS-  <paxxi>: hardyt_ sorry for the delay in response, using the method suggested by afedchin and then calling GetImplementation() on the returned file object should give you a curlfile object that you can test with  ``` auto f = dynamic_castXFILE::CCurlFile>(file-GetImplementation()); if (f) { do stuff here } ``` that gives you plenty of methods specific for curl and better control over headers and such. Not sure if it's a good way to handle it as it takes
7:20:24 AM	KS-  on kodi internals rather than the provided interface
7:21:10 AM	KS-  <paxxi>: trying to link against the provided libcurl might be tricky as it requires some tricks with delay loading to find it at runtime and that's not exposed to addons
7:22:33 AM	KS-  <afedchin>: Paxxi, is CCurlFile part of platform?
7:22:54 AM	KS-  <paxxi>: I don't think so, it's an ugly hack
7:23:11 AM	KS-  <paxxi>: it should probably be since it's common for addons to want http functionality
7:48:30 AM	davilla_  there’s a PR that add it to binary addon API
7:50:28 AM	davilla_  https://github.com/xbmc/xbmc/pull/9173
7:50:29 AM	davilla_  see
7:50:34 AM	davilla_  https://github.com/FernetMenta/xbmc/commit/e012beb27eadc9bfa434a151cb44a44ecbc54d9c

*/
// -- cURL works in *nix, but is crap in Windows -------------------


// -- Threading ----------------------------------------------------
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
// -- Threading ----------------------------------------------------

// -- Kodi stuff----------------------------------------------------
#include "WavforHue.h"

// -- Threading ----------------------------------------------------
class WavforHue_Thread
{
public:
  std::thread gWorkerThread;
  std::atomic<bool> gRunThread;

  WavforHue wavforhue;
  WavforHue_Thread();
  ~WavforHue_Thread();

  void GetPriorState();
  void PutPriorState();
  void TransferQueueToThread();
  void TransferQueueToMain();

private:
  std::string response;
  
  std::mutex gMutex;
  std::condition_variable gThreadConditionVariable;
  
  bool gReady;
  std::queue<SocketData> gQueue;

  static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp);
  void WorkerThread();
  void HTTPRequest(SocketData putData);
};



// -- Threading ----------------------------------------------------

#endif
