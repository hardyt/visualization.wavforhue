/*
*      Copyright (C) 2015-2016 Thomas M. Hardy
*      Copyright (C) 2003-2016 Team Kodi
*      Copyright (C) 1998-2000 Peter Alm, Mikael Alm, Olle Hallnas, 
*                              Thomas Nilsson and 4Front Technologies
*
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

#include "Main.h"

CVisualizationWaveForHue::CVisualizationWaveForHue()
{
  // -- Wavforhue function calls -------------------------------------
  // Initialize the lightIDs to something sane
  m_wt.wavforhue.activeHueData.lightIDs.push_back("1");
  m_wt.wavforhue.activeHueData.lightIDs.push_back("2");
  m_wt.wavforhue.activeHueData.lightIDs.push_back("3");
  m_wt.wavforhue.dimmedHueData.lightIDs.push_back("4");
  m_wt.wavforhue.dimmedHueData.lightIDs.push_back("5");
  m_wt.wavforhue.dimmedHueData.lightIDs.push_back("4");
  // -- Wavforhue function calls -------------------------------------


  m_wt.wavforhue.useWaveForm = kodi::GetSettingBoolean("useWaveForm");
  m_wt.wavforhue.strHueBridgeIPAddress = kodi::GetSettingString("hueBridgeIP");
  m_wt.wavforhue.hueBridgePort = kodi::GetSettingInt("hueBridgePort");
  m_wt.wavforhue.strHueBridgeUser = kodi::GetSettingString("hueBridgeUser");
  m_wt.wavforhue.priorState = kodi::GetSettingBoolean("priorState");

  fprintf(stderr, "m_wt.wavforhue.hueBridgePort %i\n", m_wt.wavforhue.hueBridgePort);
  fprintf(stderr, "m_wt.wavforhue.strHueBridgeIPAddress %s\n", m_wt.wavforhue.strHueBridgeIPAddress.c_str());
  fprintf(stderr, "m_wt.wavforhue.strHueBridgeUser %s\n", m_wt.wavforhue.strHueBridgeUser.c_str());

  //----------------------------------------------------------
  {
    std::string activeLightIDsUnsplit = kodi::GetSettingString("activeLights");
    m_wt.wavforhue.activeHueData.lightIDs.clear();
    std::string delimiter = ",";
    size_t last = 0;
    size_t next = 0;
    while ((next = activeLightIDsUnsplit.find(delimiter, last)) != std::string::npos)
    {
      m_wt.wavforhue.activeHueData.lightIDs.push_back(activeLightIDsUnsplit.substr(last, next - last));
      last = next + 1;
    }
    //do the last light token
    m_wt.wavforhue.activeHueData.lightIDs.push_back(activeLightIDsUnsplit.substr(last));
    m_wt.wavforhue.activeHueData.numberOfLights = m_wt.wavforhue.activeHueData.lightIDs.size();
  }
  //----------------------------------------------------------

  m_wt.wavforhue.beatThreshold = kodi::GetSettingFloat("beatThreshold");
  m_wt.wavforhue.maxBri = kodi::GetSettingInt("maxBri");
  m_wt.wavforhue.lastHue = kodi::GetSettingInt("hueRangeUpper");
  m_wt.wavforhue.initialHue = m_wt.wavforhue.lastHue;
  m_wt.wavforhue.targetHue = kodi::GetSettingInt("hueRangeLower");

  //----------------------------------------------------------
  {
    std::string dimmedLightIDsUnsplit = kodi::GetSettingString("dimmedLights");
    m_wt.wavforhue.dimmedHueData.lightIDs.clear();
    std::string delimiter = ",";
    size_t last = 0;
    size_t next = 0;
    while ((next = dimmedLightIDsUnsplit.find(delimiter, last)) != std::string::npos)
    {
      m_wt.wavforhue.dimmedHueData.lightIDs.push_back(dimmedLightIDsUnsplit.substr(last, next - last));
      last = next + 1;
    }
    //do the last light token
    m_wt.wavforhue.dimmedHueData.lightIDs.push_back(dimmedLightIDsUnsplit.substr(last));
    if (m_wt.wavforhue.dimmedHueData.lightIDs[0].size() == 0)
    {
      m_wt.wavforhue.dimmedHueData.numberOfLights = 0;
    }
    else
    {
      m_wt.wavforhue.dimmedHueData.numberOfLights = m_wt.wavforhue.dimmedHueData.lightIDs.size();
    }
  }

  m_wt.wavforhue.dimmedHueData.bri = kodi::GetSettingInt("dimmedBri");
  m_wt.wavforhue.dimmedHueData.sat = kodi::GetSettingInt("dimmedSat");
  m_wt.wavforhue.dimmedHueData.hue = kodi::GetSettingInt("dimmedHue");

  //----------------------------------------------------------
  {
    std::string afterLightIDsUnsplit = kodi::GetSettingString("afterLights");
    m_wt.wavforhue.afterHueData.lightIDs.clear();
    std::string delimiter = ",";
    size_t last = 0;
    size_t next = 0;
    while ((next = afterLightIDsUnsplit.find(delimiter, last)) != std::string::npos)
    {
      m_wt.wavforhue.afterHueData.lightIDs.push_back(afterLightIDsUnsplit.substr(last, next - last));
      last = next + 1;
    }
    //do the last light token
    m_wt.wavforhue.afterHueData.lightIDs.push_back(afterLightIDsUnsplit.substr(last));
    if (m_wt.wavforhue.afterHueData.lightIDs[0].size() == 0)
    {
      m_wt.wavforhue.afterHueData.numberOfLights = 0;
    }
    else
    {
      m_wt.wavforhue.afterHueData.numberOfLights = m_wt.wavforhue.afterHueData.lightIDs.size();
    }
  }

  m_wt.wavforhue.afterHueData.bri = kodi::GetSettingInt("afterBri");
  m_wt.wavforhue.afterHueData.sat = kodi::GetSettingInt("afterSat");
  m_wt.wavforhue.afterHueData.hue = kodi::GetSettingInt("afterHue");

  //----------------------------------------------------------

  m_wt.wavforhue.cuboxHDMIFix = kodi::GetSettingBoolean("cuboxHDMIFix");
  m_wt.wavforhue.debug = kodi::GetSettingBoolean("debug");
}

CVisualizationWaveForHue::~CVisualizationWaveForHue()
{

}

bool CVisualizationWaveForHue::Start(int channels, int samplesPerSec, int bitsPerSample, std::string songName)
{
  std::string fraqShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/frag.glsl");
  std::string vertShader = kodi::GetAddonPath("resources/shaders/" GL_TYPE_STRING "/vert.glsl");
  if (!LoadShaderFiles(vertShader, fraqShader) || !CompileAndLink())
  {
    kodi::Log(ADDON_LOG_ERROR, "Failed to create or compile shader");
    return false;
  }

#ifdef HAS_GL
  glGenBuffers(1, &m_vertexVBO);
#endif

  // -- Wavforhue function calls -------------------------------------
  // Prepare lights - dimming, turning on, etc.
  if (!m_wt.wavforhue.savedTheStates)
    m_wt.GetPriorState();
  m_wt.wavforhue.Start();
  // -- Wavforhue function calls -------------------------------------

  // -- Threading ---------------------------------------------------
  // Put this/these light request on the thread's queue.
  m_wt.TransferQueueToMain();
  // -- Threading ---------------------------------------------------

  m_startOK = true;
  return true;
}

void CVisualizationWaveForHue::Stop()
{
  if (!m_startOK)
    return;

  m_startOK = false;

#ifdef HAS_GL
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDeleteBuffers(1, &m_vertexVBO);
  m_vertexVBO = 0;
#endif

  /* This doesn't work with threading yet.
  // -- WavforHue function calls -------------------------------------
  // Change the lights to something acceptable.
  if (m_wt.wavforhue.priorState)
    m_wt.PutPriorState();
  else
    m_wt.wavforhue.Stop();
  // Put this/these light request on the main queue.
  m_wt.TransferQueueToMain();
  // -- WavforHue function calls -------------------------------------
  */

  // -- Threading ---------------------------------------------------
  //m_wt.transferQueueToThread(); // This doesn't work.
  // Clean up the thread.
  m_wt.gRunThread = false;
  while (m_wt.gWorkerThread.joinable())  // Kill 'em all \m/
  {
    m_wt.gWorkerThread.join();
  }
  // -- Threading ---------------------------------------------------

  // -- WavforHue function calls -------------------------------------
  // Change the lights to something acceptable.
  if (m_wt.wavforhue.priorState)
    m_wt.PutPriorState();
  else
    m_wt.wavforhue.Stop();
  // Put this/these light request on the main queue. This causes delay.
  m_wt.TransferQueueToMain();
  // -- WavforHue function calls -------------------------------------
}

void CVisualizationWaveForHue::Render()
{
  if (m_wt.wavforhue.useWaveForm)
  {
    // -- Waveform -----------------------------------------------------
#ifdef HAS_GL
    struct PackedVertex {
      float position[3]; // Position x, y, z
      float color[4]; // Color r, g, b, a
    } vertices[256];

    glBindBuffer(GL_ARRAY_BUFFER, m_vertexVBO);

    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, position)));
    glEnableVertexAttribArray(m_aPosition);

    glVertexAttribPointer(m_aColor, 4, GL_FLOAT, GL_FALSE, sizeof(PackedVertex), BUFFER_OFFSET(offsetof(PackedVertex, color)));
    glEnableVertexAttribArray(m_aColor);
#else
    float position[256][3]; // Position x, y, z
    float color[256][4]; // Color r, g, b, a

    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, 0, position);
    glEnableVertexAttribArray(m_aPosition);

    glVertexAttribPointer(m_aColor, 4, GL_FLOAT, GL_FALSE, 0, color);
    glEnableVertexAttribArray(m_aColor);
#endif

    glDisable(GL_BLEND);

    m_modelProjMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    m_modelProjMat = glm::rotate(m_modelProjMat, 0.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    m_modelProjMat = glm::rotate(m_modelProjMat, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    m_modelProjMat = glm::rotate(m_modelProjMat, 0.0f, glm::vec3(0.0f, 0.0f, 1.0f));

    EnableShader();

    // Left (upper) channel
#ifdef HAS_GL
    for (int i = 0; i < m_wt.wavforhue.iMaxAudioData_i; i++)
    {
      vertices[i].color[0] = m_wt.wavforhue.rgb[0];
      vertices[i].color[1] = m_wt.wavforhue.rgb[1];
      vertices[i].color[2] = m_wt.wavforhue.rgb[2];
      vertices[i].color[3] = 1.0f;
      //vertices[i].position[0] = g_viewport.X + ((i / 255.0f) * g_viewport.Width);
      //vertices[i].position[1] = g_viewport.Y + g_viewport.Height * 0.33f + (m_waveform[0][i] * g_viewport.Height * 0.15f);
      vertices[i].position[0] = -1.0f + ((i / m_wt.wavforhue.fMaxAudioData) * 2.0f);
      vertices[i].position[1] = 0.5f + m_waveform[0][i];
      vertices[i].position[2] = 1.0f;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
#else
    for (int i = 0; i < m_wt.wavforhue.iMaxAudioData_i; i++)
    {
      color[i][0] = m_wt.wavforhue.rgb[0];
      color[i][1] = m_wt.wavforhue.rgb[1];
      color[i][2] = m_wt.wavforhue.rgb[2];
      color[i][3] = 1.0f;
      //position[i][0] = g_viewport.X + ((i / 255.0f) * g_viewport.Width);
      //position[i][1] = g_viewport.Y + g_viewport.Height * 0.33f + (m_waveform[0][i] * g_viewport.Height * 0.15f);
      position[i][0] = -1.0f + ((i / m_wt.wavforhue.fMaxAudioData) * 2.0f);
      position[i][1] = 0.5f + m_waveform[0][i];
      position[i][2] = 1.0f;
    }
#endif

    glDrawArrays(GL_LINE_STRIP, 0, 256);

    // Right (lower) channel
#ifdef HAS_GL
    for (int i = 0; i < m_wt.wavforhue.iMaxAudioData_i; i++)
    {
      vertices[i].color[0] = m_wt.wavforhue.rgb[0];
      vertices[i].color[1] = m_wt.wavforhue.rgb[1];
      vertices[i].color[2] = m_wt.wavforhue.rgb[2];
      vertices[i].color[3] = 1.0f;
      //vertices[i].position[0] = g_viewport.X + ((i / 255.0f) * g_viewport.Width);
      //vertices[i].position[1] = g_viewport.Y + g_viewport.Height * 0.33f + (m_waveform[1][i] * g_viewport.Height * 0.15f);
      vertices[i].position[0] = -1.0f + ((i / m_wt.wavforhue.fMaxAudioData) * 2.0f);
      vertices[i].position[1] = -0.5f + m_waveform[1][i];
      vertices[i].position[2] = 1.0f;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
#else
    for (int i = 0; i < m_wt.wavforhue.iMaxAudioData_i; i++)
    {
      color[i][0] = m_wt.wavforhue.rgb[0];
      color[i][1] = m_wt.wavforhue.rgb[1];
      color[i][2] = m_wt.wavforhue.rgb[2];
      color[i][3] = 1.0f;
      //position[i][0] = g_viewport.X + ((i / 255.0f) * g_viewport.Width);
      //position[i][1] = g_viewport.Y + g_viewport.Height * 0.33f + (m_waveform[1][i] * g_viewport.Height * 0.15f);
      position[i][0] = -1.0f + ((i / m_wt.wavforhue.fMaxAudioData) * 2.0f);
      position[i][1] = -0.5f + m_waveform[1][i];
      position[i][2] = 1.0f;
    }
#endif

    glDrawArrays(GL_LINE_STRIP, 0, 256);

    DisableShader();

    glDisableVertexAttribArray(m_aPosition);
    glDisableVertexAttribArray(m_aColor);

    glEnable(GL_BLEND);
    // -- Waveform -----------------------------------------------------
  }

  // -- Wavforhue function calls -------------------------------------
  //get some interesting numbers to play with
  m_wt.wavforhue.UpdateTime();
  m_wt.wavforhue.timePass = m_wt.wavforhue.fElapsedAppTime;
  // -- Wavforhue function calls -------------------------------------
}

void CVisualizationWaveForHue::AudioData(const float* audioData, int audioDataLength, float *freqData, int freqDataLength)
{
  // Modified from Vortex
  float tempWave[2][576];
  int iPos = 0;
  int iOld = 0;

  while (iPos < 576)
  {
    for (int i = 0; i < audioDataLength; i += 2)
    {
      if(iPos < 512) // This is for Waveform.
      {
        m_waveform[0][iPos] = audioData[i  ]; // left channel
        m_waveform[1][iPos] = audioData[i+1]; // right channel
      }

      m_wt.wavforhue.sound.fWaveform[0][iPos] = float((audioData[i] / 32768.0f) * 255.0f);
      m_wt.wavforhue.sound.fWaveform[1][iPos] = float((audioData[i + 1] / 32768.0f) * 255.0f);

      // damp the input into the FFT a bit, to reduce high-frequency noise:
      tempWave[0][iPos] = 0.5f * (m_wt.wavforhue.sound.fWaveform[0][iPos] + m_wt.wavforhue.sound.fWaveform[0][iOld]);
      tempWave[1][iPos] = 0.5f * (m_wt.wavforhue.sound.fWaveform[1][iPos] + m_wt.wavforhue.sound.fWaveform[1][iOld]);
      iOld = iPos;
      iPos++;
      if (iPos >= 576)
        break;
    }
  }

  // -- Wavforhue function calls -------------------------------------
  m_wt.wavforhue.fftobj.time_to_frequency_domain(tempWave[0], m_wt.wavforhue.sound.fSpectrum[0]);
  m_wt.wavforhue.fftobj.time_to_frequency_domain(tempWave[1], m_wt.wavforhue.sound.fSpectrum[1]);
  m_wt.wavforhue.AnalyzeSound();
  // -- Wavforhue function calls -------------------------------------

  // -- Threading ---------------------------------------------------
  // Put this/these light request on the thread's queue.
  m_wt.TransferQueueToThread();
  // -- Threading ---------------------------------------------------
}

ADDON_STATUS CVisualizationWaveForHue::SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue)
{

  return ADDON_STATUS_OK;
}

void CVisualizationWaveForHue::OnCompiledAndLinked()
{
  m_uModelProjMatrix = glGetUniformLocation(ProgramHandle(), "u_modelViewProjectionMatrix");

  m_aPosition = glGetAttribLocation(ProgramHandle(), "a_position");
  m_aColor = glGetAttribLocation(ProgramHandle(), "a_color");
}

bool CVisualizationWaveForHue::OnEnabled()
{
  glUniformMatrix4fv(m_uModelProjMatrix, 1, GL_FALSE, glm::value_ptr(m_modelProjMat));
  return true;
}

ADDONCREATOR(CVisualizationWaveForHue) // Don't touch this!
