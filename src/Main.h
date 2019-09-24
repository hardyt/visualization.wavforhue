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

/*
 *  Wed May 24 10:49:37 CDT 2000
 *  Fixes to threading/context creation for the nVidia X4 drivers by
 *  Christian Zander <phoenix@minion.de>
 */

#pragma once

#include "WavforHue_Thread.h"

#include <kodi/addon-instance/Visualization.h>
#include <kodi/gui/gl/Shader.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtc/type_ptr.hpp>

class ATTRIBUTE_HIDDEN CVisualizationWaveForHue
  : public kodi::addon::CAddonBase,
    public kodi::addon::CInstanceVisualization,
    public kodi::gui::gl::CShaderProgram
{
public:
  CVisualizationWaveForHue();
  ~CVisualizationWaveForHue() override;

  bool Start(int channels, int samplesPerSec, int bitsPerSample, std::string songName) override;
  void Stop() override;
  void Render() override;
  void AudioData(const float* audioData, int audioDataLength, float *freqData, int freqDataLength) override;
  ADDON_STATUS SetSetting(const std::string& settingName, const kodi::CSettingValue& settingValue) override;

  void OnCompiledAndLinked() override;
  bool OnEnabled() override;

private:
  WavforHue_Thread m_wt;
  float m_waveform[2][512];

  glm::mat4 m_modelProjMat;

#ifdef HAS_GL
  GLuint m_vertexVBO = 0;
#endif
  GLint m_uModelProjMatrix = -1;
  GLint m_aPosition = -1;
  GLint m_aColor = -1;

  bool m_startOK = false;
};
