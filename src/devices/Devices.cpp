/*
 *      Copyright (C) 2014 Garrett Brown
 *      Copyright (C) 2014 Team XBMC
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

#include "Devices.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "kodi/libXBMC_addon.h"
#include "tinyxml.h"

#include <algorithm>

using namespace JOYSTICK;

#define ADDON_BUTTONMAP  "/resources/buttonmap.xml"
#define USER_BUTTONMAP   "/buttonmap.xml"

namespace JOYSTICK
{
  void RemoveSlashAtEnd(std::string& strPath)
  {
    if (strPath.empty())
    {
      char end = strPath[strPath.size() - 1];
      if (end == '/' || end == '\\')
        strPath.erase(strPath.end() - 1);
    }
  }
}

CDevices& CDevices::Get(void)
{
  static CDevices _instance;
  return _instance;
}

bool CDevices::Initialize(const PERIPHERAL_PROPERTIES& props)
{
  std::string strAddonPath = props.addon_path ? props.addon_path : "";
  std::string strUserPath = props.user_path ? props.user_path : "";

  RemoveSlashAtEnd(strAddonPath);
  RemoveSlashAtEnd(strUserPath);

  strAddonPath += ADDON_BUTTONMAP;
  strUserPath += USER_BUTTONMAP;

  bool bSuccess = false;

  /* TODO
  // Require add-on buttonmap, attempt user buttonmap
  bSuccess = Load(strAddonPath);
  */
  bSuccess = true;
  Load(strAddonPath);
  Load(strUserPath);

  m_strPath = strUserPath;

  return bSuccess;
}

bool CDevices::Load(const std::string& strPath)
{
  dsyslog("Loading buttonmap: %s", strPath.c_str());

  TiXmlDocument xmlFile;
  if (xmlFile.LoadFile(strPath))
  {
    TiXmlElement* pRootElement = xmlFile.RootElement();
    if (!pRootElement || pRootElement->NoChildren() || pRootElement->ValueStr() != BUTTONMAP_XML_ROOT)
    {
      esyslog("Can't find root <%s> tag", BUTTONMAP_XML_ROOT);
      return false;
    }
    else
    {
      return Deserialize(pRootElement);
    }
  }

  return false;
}

bool CDevices::Save(const std::string& strPath) const
{
  if (m_strPath.empty())
    return false;

  TiXmlDocument xmlFile;

  TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "", "");
  xmlFile.LinkEndChild(decl);

  TiXmlElement rootElement(BUTTONMAP_XML_ROOT);
  TiXmlNode* root = xmlFile.InsertEndChild(rootElement);
  if (root == NULL)
    return false;

  TiXmlElement devices(BUTTONMAP_XML_ELEM_DEVICES);
  TiXmlNode* pDevices = root->InsertEndChild(devices);
  if (pDevices == NULL)
    return false;

  TiXmlElement* devicesElem = pDevices->ToElement();
  if (!devicesElem)
    return false;

  for (std::vector<CDevice>::const_iterator it = m_devices.begin(); it != m_devices.end(); ++it)
  {
    TiXmlElement device(BUTTONMAP_XML_ELEM_DEVICE);
    TiXmlNode* pDevice = devicesElem->InsertEndChild(device);
    if (pDevice == NULL)
      continue;

    TiXmlElement* deviceElem = pDevice->ToElement();
    if (!deviceElem)
      continue;

    it->Serialize(deviceElem);
  }

  return xmlFile.SaveFile(m_strPath);
}

bool CDevices::GetFeatures(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                           const std::string& strControllerId, std::vector<ADDON::JoystickFeature*>& features) const
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  CDevice needle(strName,
                 strProvider,
                 vid,
                 pid,
                 buttonCount,
                 hatCount,
                 axisCount);

  std::vector<CDevice>::const_iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice != m_devices.end())
    return itDevice->GetFeatures(strControllerId, features);

  return true;
}

bool CDevices::MapFeature(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                          const std::string& strControllerId, const ADDON::JoystickFeature* feature)
{
  const std::string& strName     = peripheral.Name();
  uint16_t           vid         = peripheral.VendorID();
  uint16_t           pid         = peripheral.ProductID();
  const std::string& strProvider = joystick.Provider();
  unsigned int       buttonCount = joystick.ButtonCount();
  unsigned int       hatCount    = joystick.HatCount();
  unsigned int       axisCount   = joystick.AxisCount();

  CDevice needle(strName,
                 strProvider,
                 vid,
                 pid,
                 buttonCount,
                 hatCount,
                 axisCount);

  std::vector<CDevice>::iterator itDevice = std::find(m_devices.begin(), m_devices.end(), needle);
  if (itDevice == m_devices.end())
  {
    m_devices.push_back(needle);
    itDevice = m_devices.end() - 1;
  }

  if (itDevice->MapFeature(strControllerId, feature))
  {
    Save(m_strPath);
    return true;
  }

  return false;
}

bool CDevices::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  for (std::vector<CDevice>::const_iterator it = m_devices.begin(); it != m_devices.end(); ++it)
  {
    const CDevice& device = *it;

    TiXmlElement deviceElement(BUTTONMAP_XML_ELEM_DEVICE);
    TiXmlNode* deviceNode = pElement->InsertEndChild(deviceElement);
    if (deviceNode == NULL)
      return false;

    TiXmlElement* deviceElem = deviceNode->ToElement();
    if (deviceElem == NULL)
      return false;

    device.Serialize(deviceElem);
  }

  return true;
}

bool CDevices::Deserialize(const TiXmlElement* pElement)
{
  if (!pElement)
    return false;

  const TiXmlElement* pDevices = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_DEVICES);

  if (!pDevices)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_DEVICES);
    return false;
  }

  const TiXmlNode* pDevicesNode = pDevices->ToElement();
  if (!pDevicesNode)
    return false;

  const TiXmlElement* pDevice = pDevicesNode->FirstChildElement(BUTTONMAP_XML_ELEM_DEVICE);

  if (!pDevice)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_DEVICE);
    return false;
  }

  while (pDevice)
  {
    CDevice device;
    if (!device.Deserialize(pDevice))
      return false;

    if (!device.IsValid())
    {
      esyslog("<%s> tag with name=\"%s\" is invalid", BUTTONMAP_XML_ELEM_DEVICE, device.Name().c_str());
      return false;
    }

    m_devices.push_back(device);

    pDevice = pDevice->NextSiblingElement(BUTTONMAP_XML_ELEM_DEVICE);
  }

  return true;
}
