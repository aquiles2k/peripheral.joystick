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

#include "Device.h"
#include "JoystickDefinitions.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cstdlib>

using namespace JOYSTICK;

CDevice::CDevice(void)
  : m_strName(),
    m_strProvider(),
    m_vid(0),
    m_pid(0),
    m_buttonCount(0),
    m_hatCount(0),
    m_axisCount(0)
{
}

CDevice::CDevice(const std::string& strName,
                 const std::string& strProvider,
                 uint16_t           vid,
                 uint16_t           pid,
                 unsigned int       buttonCount,
                 unsigned int       hatCount,
                 unsigned int       axisCount)
 : m_strName(strName),
   m_strProvider(strProvider),
   m_pid(pid),
   m_vid(vid),
   m_buttonCount(buttonCount),
   m_hatCount(hatCount),
   m_axisCount(axisCount)
{
}

bool CDevice::operator==(const CDevice& rhs) const
{
  return rhs.m_strName.empty()              ? true : m_strName     == rhs.m_strName   &&
         rhs.m_strProvider.empty()          ? true : m_strProvider == rhs.m_strProvider &&
         (rhs.m_pid == 0 && rhs.m_vid == 0) ? true : m_pid == rhs.m_pid && m_vid == rhs.m_vid &&
         rhs.m_buttonCount == 0 &&
         rhs.m_hatCount    == 0 &&
         rhs.m_axisCount   == 0             ? true : m_buttonCount == rhs.m_buttonCount &&
                                                     m_hatCount    == rhs.m_hatCount    &&
                                                     m_axisCount   == rhs.m_axisCount;

}

bool CDevice::GetFeatures(const std::string& strDeviceId, std::vector<ADDON::JoystickFeature*>& features) const
{
  ButtonMaps::const_iterator it = m_buttonMaps.find(strDeviceId);
  if (it != m_buttonMaps.end())
    return it->second.GetFeatures(features);

  return true;
}

bool CDevice::MapFeature(const std::string& strDeviceId, const ADDON::JoystickFeature* feature)
{
  return m_buttonMaps[strDeviceId].MapFeature(feature);
}

bool CDevice::IsValid(void) const
{
  return !m_strName.empty()     &&
         !m_strProvider.empty();
}

bool CDevice::Serialize(TiXmlElement* pElement) const
{
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_NAME,     m_strName);
  pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER, m_strProvider);
  if (m_vid != 0 &&
      m_pid != 0)
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_VID, m_vid);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_PID, m_pid);
  }
  if (m_buttonCount != 0 &&
      m_hatCount    != 0 &&
      m_axisCount   != 0)
  {
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT, m_buttonCount);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT,    m_hatCount);
    pElement->SetAttribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT,   m_axisCount);
  }

  for (ButtonMaps::const_iterator it = m_buttonMaps.begin(); it != m_buttonMaps.end(); ++it)
  {
    const std::string& deviceId = it->first;
    const CButtons& buttons = it->second;

    TiXmlElement profileElement(BUTTONMAP_XML_ELEM_PROFILE);
    TiXmlNode* profileNode = pElement->InsertEndChild(profileElement);
    if (profileNode == NULL)
      continue;

    TiXmlElement* profileElem = profileNode->ToElement();
    if (profileElem == NULL)
      continue;

    profileElem->SetAttribute(BUTTONMAP_XML_ATTR_PROFILE_ID, deviceId);

    buttons.Serialize(profileElem);
  }

  return true;
}

bool CDevice::Deserialize(const TiXmlElement* pElement)
{
  Reset();

  const char* name = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_NAME);
  if (!name)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_NAME);
    return false;
  }
  m_strName = name;

  const char* provider = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
  if (!provider)
  {
    esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_DEVICE, BUTTONMAP_XML_ATTR_DEVICE_PROVIDER);
    return false;
  }
  m_strProvider = provider;

  const char* vid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_VID);
  if (vid)
    m_vid = std::atoi(vid);

  const char* pid = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_PID);
  if (pid)
    m_pid = std::atoi(pid);

  const char* buttonCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_BUTTONCOUNT);
  if (buttonCount)
    m_buttonCount = std::atoi(buttonCount);

  const char* hatCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_HATCOUNT);
  if (hatCount)
    m_hatCount = std::atoi(hatCount);

  const char* axisCount = pElement->Attribute(BUTTONMAP_XML_ATTR_DEVICE_AXISCOUNT);
  if (axisCount)
    m_axisCount = std::atoi(axisCount);

  const TiXmlElement* pProfile = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_PROFILE);

  if (!pProfile)
  {
    esyslog("Device \"%s\": can't find <%s> tag", m_strName.c_str(), BUTTONMAP_XML_ELEM_PROFILE);
    return false;
  }

  while (pProfile)
  {
    const char* id = pProfile->Attribute(BUTTONMAP_XML_ATTR_PROFILE_ID);
    if (!id)
    {
      esyslog("Device \"%s\": <%s> tag has no attribute \"%s\"", m_strName.c_str(),
              BUTTONMAP_XML_ELEM_PROFILE, BUTTONMAP_XML_ATTR_PROFILE_ID);
      return false;
    }

    CButtons buttons;
    if (!buttons.Deserialize(pProfile))
      return false;

    m_buttonMaps[id] = buttons;

    pProfile = pProfile->NextSiblingElement(BUTTONMAP_XML_ELEM_PROFILE);
  }

  return true;
}
