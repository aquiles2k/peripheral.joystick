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

#include "kodi/kodi_peripheral_utils.hpp"

#include <map>
#include <string>

class TiXmlElement;

namespace JOYSTICK
{
  class CButtons
  {
  public:
    CButtons(void) { }

    void Reset(void) { *this = CButtons(); }

    bool GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const;

    bool MapFeature(const ADDON::JoystickFeature* feature);

    bool Serialize(TiXmlElement* pElement) const;
    bool Deserialize(const TiXmlElement* pElement);

  private:
    void UnMap(const ADDON::JoystickFeature* feature);
    void UnMapButton(const ADDON::DriverButton* button);
    void UnMapHat(const ADDON::DriverHat* hat);
    void UnMapSemiAxis(const ADDON::DriverSemiAxis* semiAxis);
    void UnMapAnalogStick(const ADDON::DriverAnalogStick* analogStick);
    void UnMapAccelerometer(const ADDON::DriverAccelerometer* accelerometer);

    static bool ButtonConflicts(const ADDON::DriverButton* button, const ADDON::JoystickFeature* feature);
    static bool HatConflicts(const ADDON::DriverHat* hat, const ADDON::JoystickFeature* feature);
    static bool SemiAxisConflicts(const ADDON::DriverSemiAxis* semiAxis, const ADDON::JoystickFeature* feature);

    typedef std::string                                    FeatureName;
    typedef std::map<FeatureName, ADDON::JoystickFeature*> Buttons;

    Buttons m_buttons;
  };
}
