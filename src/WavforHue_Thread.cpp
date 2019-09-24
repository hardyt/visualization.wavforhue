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


#ifndef WAVFORHUE_THREAD
#include "WavforHue_Thread.h"
#endif


// -- Constructor ----------------------------------------------------
WavforHue_Thread::WavforHue_Thread()
{
}
// -- Constructor ----------------------------------------------------

// -- Destructor ----------------------------------------------------- 
WavforHue_Thread::~WavforHue_Thread()
{
}
// -- Destructor ----------------------------------------------------- 

// -- Threading ----------------------------------------------------------
// This thread keeps cURL from puking all over the waveform, suprising it and
// making it jerk away.
void WavforHue_Thread::WorkerThread()
{
  bool isEmpty;
  SocketData putData;
  std::queue<SocketData> mQueue;
  // This thread comes alive when AudioData(), Create() or Start() has put an 
  // item in the stack. It runs until Destroy() or Stop() sets gRunThread to 
  // false and joins it. Or something like that. It's actually magic.
  while (gRunThread || !mQueue.empty())
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
      gThreadConditionVariable.wait(lock, [&]{return gReady; });
    }
    else
    {
      // Get everything off the global queue for local processing
      std::lock_guard<std::mutex> lock(gMutex);
      while (!gQueue.empty())
      {
        mQueue.push(gQueue.front());
        gQueue.pop();
      }
    }
    while (!mQueue.empty())
    {
      putData = mQueue.front(); mQueue.pop();
      HTTPRequest(putData);
    }
  }
}

void WavforHue_Thread::TransferQueueToMain()
{
	SocketData putData;
	while (!wavforhue.queue.empty())
	{
    putData = wavforhue.queue.front(); wavforhue.queue.pop();
    HTTPRequest(putData);
  }
}

void WavforHue_Thread::TransferQueueToThread()
{
  SocketData putData;
  gRunThread = true;
  // Check if the thread is alive yet.
  if (!gWorkerThread.joinable())
  {
    gWorkerThread = std::thread(&WavforHue_Thread::WorkerThread, this);
  }
  while (!wavforhue.queue.empty())
  {
    putData = wavforhue.queue.front();
    wavforhue.queue.pop();
    //if (gQueue.size() < 10)
    //{
      std::lock_guard<std::mutex> lock(gMutex);
      gQueue.push(putData);
    //}
  }
  // Let the thread know to start processing.
  {
    std::lock_guard<std::mutex> lock(gMutex);
    gReady = true;
  }
  gThreadConditionVariable.notify_one();
}
//-- Threading -----------------------------------------------------



// -- HTTP functions -----------------------------------------------
// This helps cURL store the HTTP response in a string
size_t WavforHue_Thread::WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  ((std::string*)userp)->append((char*)contents, size * nmemb);
  return size * nmemb;
}

void WavforHue_Thread::HTTPRequest(SocketData socketData)
{
  response="";
#ifndef _WIN32
  wavforhue.SendDebug("Initializing cURL.");
  CURL *curl;
  CURLcode res;
  std::string url = "http://" + socketData.host + ":" + socketData.port + socketData.path;
  wavforhue.SendDebug(url.c_str());
  wavforhue.SendDebug(socketData.json.c_str());
  curl = curl_easy_init();
  // Now specify we want to PUT data, but not using a file, so it has o be a CUSTOMREQUEST
  curl_easy_setopt(curl, CURLOPT_TCP_NODELAY, 1);
  curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);
  curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, socketData.method.c_str());
  // This eliminates all kinds of HTTP responses from showing up in stdin.
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &WavforHue_Thread::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, socketData.json.c_str());
  // Set the URL that is about to receive our POST. 
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  // Perform the request, res will get the return code
  res = curl_easy_perform(curl);
  // always cleanup curl
  curl_easy_cleanup(curl);
  wavforhue.SendDebug("cURL cleaned up.");

#else
  std::string request, error;
  char buffer[BUFFERSIZE];

  WSADATA wsaData;
  SOCKET ConnectSocket = INVALID_SOCKET;
  struct addrinfo *result = NULL,
    *ptr = NULL,
    hints;
  int iResult;

  // Initialize Winsock
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (iResult != 0) {
    wavforhue.SendDebug("WSAStartup failed with error: " + iResult);
    WSACleanup();
    abort();
  }

  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  // Resolve the server address and port
  iResult = getaddrinfo(socketData.host.c_str(), socketData.port.c_str(), &hints, &result);
  if (iResult != 0) {
    wavforhue.SendDebug("getaddrinfo failed with error: " + iResult);
    WSACleanup();
    abort();
  }

  // Attempt to connect to an address until one succeeds
  for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

    // Create a SOCKET for connecting to server
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
      ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
      wavforhue.SendDebug("socket failed with error : " + WSAGetLastError());
      WSACleanup();
      abort();
    }

    // Connect to server.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
      closesocket(ConnectSocket);
      ConnectSocket = INVALID_SOCKET;
      continue;
    }
    break;
  }

  freeaddrinfo(result);

  if (ConnectSocket == INVALID_SOCKET) {
    wavforhue.SendDebug("Unable to connect to server!");
    WSACleanup();
    abort();
  }

  // Send an initial buffer
  std::stringstream ss;
  ss << socketData.json.length();

  std::stringstream request2;

  request2 << socketData.method << " " << socketData.path << " HTTP/1.1" << std::endl;
  request2 << "User-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; .NET CLR 1.1.4322; .NET CLR 2.0.50727)" << std::endl;
  //request2 << "" << endl;
  request2 << "Host: " << socketData.host << std::endl;
  request2 << "Content-Length: " << socketData.json.length() << std::endl;

  request2 << "Content-Type: application/x-www-form-urlencoded" << std::endl;
  request2 << "Accept-Language: en" << std::endl;
  request2 << std::endl;
  request2 << socketData.json;
  request = request2.str();

  iResult = send(ConnectSocket, request.c_str(), request.length(), 0);
  if (iResult == SOCKET_ERROR) {
    wavforhue.SendDebug("send failed with error: " + WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    abort();
  }

  if (wavforhue.debug)
  {
    wavforhue.SendDebug("Connected");
  }

  // shutdown the connection since no more data will be sent
  iResult = shutdown(ConnectSocket, SD_SEND);
  if (iResult == SOCKET_ERROR) {
    wavforhue.SendDebug("shutdown failed with error: " + WSAGetLastError());
    closesocket(ConnectSocket);
    WSACleanup();
    abort();
  }

  // Receive until the peer closes the connection
  response = "";
  do {
    iResult = recv(ConnectSocket, (char*)&buffer, BUFFERSIZE, 0);
    if (iResult > 0)
      response += std::string(buffer).substr(0, iResult);
    else if (iResult == 0)
    {
      if (wavforhue.debug)
      {
        wavforhue.SendDebug("Connection closed.");
      }
    }
    else
    {
      wavforhue.SendDebug("recv failed with error: " + WSAGetLastError());
    }
  } while (iResult > 0);

  // cleanup
  closesocket(ConnectSocket);
  WSACleanup();
  // response is holding the json response from the Hue bridge;
  response = response.substr(response.find("\r\n\r\n"));
#endif
  wavforhue.SendDebug(response.c_str());
}

void WavforHue_Thread::GetPriorState()
{
  // Get the json data for the current state.
  SocketData getData;
  wavforhue.SendDebug("Setting up GET packet.");
  getData.host = wavforhue.strHueBridgeIPAddress;
  getData.method = "GET";
  getData.path = "/api/" + wavforhue.strHueBridgeUser + "/lights";
  getData.json = "";

  wavforhue.SendDebug("Sending HTTPRequest.");
  HTTPRequest(getData);

  // Now response should have the json for all the light states
  wavforhue.SendDebug("Saving the response.");
  wavforhue.SaveState(response);

}

void WavforHue_Thread::PutPriorState()
{
  wavforhue.RestoreState();
}


// -- HTTP functions -----------------------------------------------
