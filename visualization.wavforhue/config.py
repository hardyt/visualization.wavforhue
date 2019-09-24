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
# along with XBMC; see the file COPYING.  If not, see
# <http://www.gnu.org/licenses/>.

import xbmc
import xbmcgui
import xbmcaddon
import json
import time
import sys
import colorsys
import os
import datetime
import math
#import collections # Android Kodi uses 2.6, so I am including
# the library directly with from ordereddict import * below.
from threading import Timer

__addon__      = xbmcaddon.Addon('visualization.wavforhue')
__addondir__   = xbmc.translatePath( __addon__.getAddonInfo('profile') ) 
__cwd__        = __addon__.getAddonInfo('path')
__resource__   = xbmc.translatePath( os.path.join( __cwd__, 'resources', 'lib' ) )
__icon__       = os.path.join(__cwd__,"icon.png")

sys.path.append (__resource__)

from settings import *
from tools import *
from ordereddict import *

try:
  import requests
except ImportError:
  xbmc.log("ERROR: Could not locate required library Requests.")
  notify("WavforHue", "ERROR: Could not import Python Requests")

 
class Hue:
  params = None
  connected = None
  last_state = None
  light = None
  ambilight_dim_light = None
  pauseafterrefreshchange = 0

  def __init__(self, settings, args):
    self.logger = Logger()
    if settings.debug:
      self.logger.debug()

    #get settings
    self.settings = settings
    self._parse_argv(args)
        
    if self.params['action'] == "discover":
      self.logger.debuglog("Starting discovery")
      notify("Bridge Discovery", "starting")
      hue_ip, hue_port = self.start_autodiscover()
      self.logger.debuglog("Discovery finished")
      if hue_ip != None:
        notify("Bridge Discovery", "Found bridge at: %s" % hue_ip)
        username = self.register_user(hue_ip, hue_port)
        self.logger.debuglog("Updating settings")
        self.settings.update(hueBridgeIP = hue_ip)
        self.settings.update(hueBridgePort = hue_port)
        self.settings.update(hueBridgeUser = username)
        notify("Bridge Discovery", "Finished")
        self.test_connection()
      else:
        notify("Bridge Discovery", "Failed. Could not find bridge.")
      self.configure_lights()
    elif self.params['action'] == "config_lights":
      self.configure_lights()
    elif self.params['action'] == "reset_settings":
      self.logger.debuglog("Reset Settings to default.")
      #self.logger.debuglog(__addondir__)
      #os.unlink(os.path.join(__addondir__,"settings.xml"))
      self.settings.update(hueBridgeIP = "")
      self.settings.update(hueBridgeUser = "")
      self.settings.update(activeLights = "")
      self.settings.update(dimmedLights = "")
      self.settings.update(afterLights = "")
      self.logger.debuglog("Cleared bridge settings")
      time.sleep(1)
      self.settings.readxml()
    else:
      # not yet implemented
      self.logger.debuglog("unimplemented action call: %s" % self.params['action'])    
    
        
  def _parse_argv(self, args):
    try:
        self.params = dict(arg.split("=") for arg in args.split("&"))
    except:
        self.params = {}
        
  def start_autodiscover(self):
    port = 1900
    ip = "239.255.255.250"

    self.logger.debuglog("In autodiscovery")
    address = (ip, port)
    data = """M-SEARCH * HTTP/1.1
    HOST: %s:%s
    MAN: ssdp:discover
    MX: 3
    ST: upnp:rootdevice
    """ % (ip, port)
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP) #force udp
    self.logger.debuglog("Created socket")
    client_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    self.logger.debuglog("Updated socket 1")
    client_socket.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)
    self.logger.debuglog("Updated socket 2")
    client_socket.settimeout(1)
    self.logger.debuglog("Set timeout")

    hue_ip = None
    hue_port = None
    num_retransmits = 0
    while(num_retransmits < 10) and hue_ip == None and hue_port == None:
      self.logger.debuglog("Transmitting on protocol 1")
      num_retransmits += 1
      try:
        self.logger.debuglog("Sendto")
        client_socket.sendto(data, address)
        self.logger.debuglog("recvfrom")
        recv_data, addr = client_socket.recvfrom(2048)
        self.logger.debuglog("received data during autodiscovery: "+recv_data)
        if "IpBridge" in recv_data and "description.xml" in recv_data:
          self.logger.debuglog("Found IpBridge")
          hue_ip = recv_data.split("LOCATION: http://")[1].split(":")[0]
          hue_port = recv_data.split("LOCATION: http://")[1].split(":")[1]
        time.sleep(1)
      except socket.timeout:
        self.logger.debuglog("Socket timed out")
        break #if the socket times out once, its probably not going to complete at all. fallback to nupnp.
    if hue_ip == None:
      #still nothing found, try alternate api
      self.logger.debuglog("Trying alt api")
      r=requests.get("https://www.meethue.com/api/nupnp", verify=False) #verify false hack until meethue fixes their ssl cert.
      j=r.json()
      if len(j) > 0:
        hue_ip=j[0]["internalipaddress"]
        hue_port=80
        self.logger.debuglog("meethue nupnp api returned: "+hue_ip)
      else:
        self.logger.debuglog("meethue nupnp api did not find bridge")
        
    return [hue_ip, hue_port]

  def register_user(self, hue_ip, hue_port):
    #username = hashlib.md5(str(random.random())).hexdigest() #not needed with new strategy
    device = "kodi-wavforhue-addon"
    data = '{"devicetype": "%s#%s"}' % (device, xbmc.getInfoLabel('System.FriendlyName')[0:19])
    self.logger.debuglog("sending data: %s" % data)

    r = requests.post('http://%s:%s/api' % (hue_ip, hue_port), data=data)
    response = r.text
    while "link button not pressed" in response:
      self.logger.debuglog("register user response: %s" % r)
      notify("Bridge Discovery", "Press link button on bridge")
      r = requests.post('http://%s:%s/api' % (hue_ip, hue_port),, data=data)
      response = r.text 
      time.sleep(3)

    j = r.json()
    self.logger.debuglog("got a username response: %s" % j)
    username = j[0]["success"]["username"]

    return username

  def test_connection(self):
    self.logger.debuglog("testing connection")
    r = requests.get('http://%s:%s/api/%s/config' % \
      (self.settings.hueBridgeIP, self.settings.hueBridgePort, self.settings.hueBridgeUser))
    test_connection = r.text.find("name")
    if not test_connection:
      notify("Failed", "Could not connect to bridge")
      self.connected = False
    else:
      notify("WavforHue", "Connected")
      self.connected = True
    return self.connected
    
  def configure_lights(self):
    self.lactiveLights = []
    self.ldimmedLights = []
    self.lafterLights = []
    # Find all the light IDs.
    self.get_all_lights();
    # Update the settings.
    self.settings.update(activeLights = ','.join(self.lactiveLights))
    self.settings.update(dimmedLights = ','.join(self.ldimmedLights))
    self.settings.update(afterLights = ','.join(self.lafterLights))
      
  def get_all_lights(self):
    self.logger.debuglog("get_all_ligts. requesting from: http://%s:%s/api/%s/lights" % \
      (self.settings.hueBridgeIP, self.settings.hueBridgePort, self.settings.hueBridgeUser))
    r = requests.get("http://%s:%s/api/%s/lights" % \
      (self.settings.hueBridgeIP, self.settings.hueBridgePort, self.settings.hueBridgeUser))
    j = r.json()

    if isinstance(j, list) and "error" in j[0]:
      # something went wrong.
      err = j[0]["error"]
      if err["type"] == 3:
        notify("Lights not found", "Could not find lights in bridge. Check log.")
      else:
        notify("Bridge Error", "Error %s while talking to the bridge" % err["type"])
      raise ValueError("Bridge Error", err["type"], err)
      return

    #no error, keep going
    # Sort the lights.
    # collections is not available in 2.6, which messes up Android
    #od = collections.OrderedDict(sorted(j.items(), key=lambda x:int(x[0])))
    od = OrderedDict(sorted(j.items(), key=lambda x:int(x[0])))
    # Loop through all the lights.
    i = 0
    self.light = [None] * len(j);
    for key, value in od.iteritems():
      # Flash current light.
      self.light[i] = Light(key, self.settings)
      self.light[i].flash_light();
      i += 1
      dialog = xbmcgui.Dialog()
      lightName = value['name']
      # Ask user if they want the light to be active, dimmed, or off
      if dialog.yesno(" Light Configuration", "Use light ID #%s? " 
                      "This light's name is '%s.'" % (key, lightName)):
        if dialog.yesno(" Light Configuration", 
                        "Should light %s be active? 'No' implies dimmed on play." % key):
          self.lactiveLights.append(key)
        else:
          self.ldimmedLights.append(key)
        if dialog.yesno(" Light Configuration", 
                        "Should light %s come on after playback is stopped?" % key):
          self.lafterLights.append(key)
    dialog.ok(" Light Configuration", "Setup is complete.")

if ( __name__ == "__main__" ):
  settings = settings()
  logger = Logger()
  if settings.debug == True:
    logger.debug()
  
  args = None
  if len(sys.argv) == 2:
    args = sys.argv[1]
  hue = Hue(settings, args)
