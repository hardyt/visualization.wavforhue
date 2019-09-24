#
# Copyright (C) 2016 Thomas M. Hardy
#
# http://kodi.tv
#
# This Program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This Program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Kodi; see the file COPYING.  If not, see
# <http://www.gnu.org/licenses/>.

import sys
import xbmcaddon

__addon__      = sys.modules[ "__main__" ].__addon__

class settings():
  def __init__( self, *args, **kwargs ):
    self.readxml()
    self.addon = xbmcaddon.Addon('visualization.wavforhue')

  def readxml(self):
    self.hueBridgeIP           = __addon__.getSetting("hueBridgeIP")
    self.hueBridgePort         = __addon__.getSetting("hueBridgePort")
    self.hueBridgeUser         = __addon__.getSetting("hueBridgeUser")

    self.activeLights          = __addon__.getSetting("activeLights")
    self.dimmedLights          = __addon__.getSetting("dimmedLights")
    self.afterLights           = __addon__.getSetting("afterLights")
    self.priorState            = __addon__.getSetting("priorState") == "false"

    self.debug                 = __addon__.getSetting("debug") == "true"
    
    #dummy settings to keep tools.py happy    
    self.mode                  = 0
    self.dim_time              = 10
    self.proportional_dim_time = True
    self.override_hue          = False
    self.dimmed_bri            = 5
    self.dimmed_hue            = 65100
    self.override_sat          = False
    self.dimmed_sat            = 254
    self.undim_sat             = 40
    self.override_paused       = False
    self.paused_bri            = 75
    self.undim_bri             = 100
    self.undim_hue             = 17500
    self.override_undim_bri    = False
    self.force_light_on        = False
    self.force_light_group_start_override = False
    self.misc_initialflash     = True

  def update(self, **kwargs):
    self.__dict__.update(**kwargs)
    for k, v in kwargs.iteritems():
      self.addon.setSetting(k, v)

  def __repr__(self):
    return 'hueBridgeIP: %s\n' % self.hueBridgeIP + \
    'hueBridgeUser: %s\n' % self.hueBridgeUser + \
    'activeLights: %s\n' % str(self.activeLights) + \
    'dimmedLights: %s\n' % str(self.dimmedLights) + \
    'afterLights: %s\n' % str(self.afterLights) + \
    'priorState: %s\n' % self.priorState + \
    'debug: %s\n' % self.debug