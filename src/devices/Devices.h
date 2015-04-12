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
#pragma once

#include "Device.h"

#include "kodi/kodi_peripheral_types.h"
#include "kodi/kodi_peripheral_utils.hpp"

#include <string>
#include <vector>

class TiXmlElement;

namespace JOYSTICK
{
  class CDevices
  {
  private:
    CDevices(void) { }

  public:
    static CDevices& Get(void);

    virtual ~CDevices(void) { Deinitialize(); }

    bool Initialize(const PERIPHERAL_PROPERTIES& props);
    void Deinitialize(void) { }

    bool GetFeatures(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                     const std::string& strDeviceId, std::vector<ADDON::JoystickFeature*>& features) const;

    bool MapFeature(const ADDON::Peripheral& peripheral, const ADDON::Joystick& joystick,
                    const std::string& strDeviceId, const ADDON::JoystickFeature* feature);

    bool Deserialize(const TiXmlElement* pElement);
    bool Serialize(TiXmlElement* pElement) const;

  private:
    bool Load(const std::string& strPath);
    bool Save(const std::string& strPath) const;

    std::vector<CDevice> m_devices;
    std::string          m_strPath;
  };
}
