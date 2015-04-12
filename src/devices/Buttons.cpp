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

#include "Buttons.h"
#include "JoystickDefinitions.h"
#include "JoystickTranslator.h"
#include "log/Log.h"

#include "tinyxml.h"

#include <cmath>
#include <cstdlib>
#include <sstream>

using namespace JOYSTICK;

bool CButtons::GetFeatures(std::vector<ADDON::JoystickFeature*>& features) const
{
  for (Buttons::const_iterator itButton = m_buttons.begin(); itButton != m_buttons.end(); ++itButton)
    features.push_back(itButton->second);

  return true;
}

bool CButtons::MapFeature(const ADDON::JoystickFeature* feature)
{
  if (feature && !feature->Name().empty())
  {
    UnMap(feature);

    const std::string& strFeatureName = feature->Name();

    Buttons::iterator itFeature = m_buttons.find(strFeatureName);
    if (itFeature != m_buttons.end())
      delete itFeature->second;

    m_buttons[strFeatureName] = feature->Clone();

    return true;
  }

  return false;
}

void CButtons::UnMap(const ADDON::JoystickFeature* feature)
{
  switch (feature->Type())
  {
    case JOYSTICK_DRIVER_TYPE_BUTTON:
      UnMapButton(static_cast<const ADDON::DriverButton*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      UnMapHat(static_cast<const ADDON::DriverHat*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      UnMapSemiAxis(static_cast<const ADDON::DriverSemiAxis*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      UnMapAnalogStick(static_cast<const ADDON::DriverAnalogStick*>(feature));
      break;
    case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      UnMapAccelerometer(static_cast<const ADDON::DriverAccelerometer*>(feature));
      break;
    default:
      break;
  }
}

void CButtons::UnMapButton(const ADDON::DriverButton* button)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (ButtonConflicts(button, it->second))
    {
      m_buttons.erase(it);
      break;
    }
  }
}

void CButtons::UnMapHat(const ADDON::DriverHat* hat)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (HatConflicts(hat, it->second))
    {
      m_buttons.erase(it);
      break;
    }
  }
}

void CButtons::UnMapSemiAxis(const ADDON::DriverSemiAxis* semiAxis)
{
  for (Buttons::iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    if (SemiAxisConflicts(semiAxis, it->second))
    {
      switch (it->second->Type())
      {
        case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
        {
          m_buttons.erase(it);
          break;
        }
        case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
        {
          ADDON::DriverAnalogStick* analogStick = static_cast<ADDON::DriverAnalogStick*>(it->second);

          if (semiAxis->Index() == analogStick->XIndex())
          {
            analogStick->SetXIndex(-1);
            if (analogStick->YIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == analogStick->YIndex())
          {
            analogStick->SetYIndex(-1);
            if (analogStick->XIndex() < 0)
              m_buttons.erase(it);
          }
          break;
        }
        case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
        {
          ADDON::DriverAccelerometer* accelerometer = static_cast<ADDON::DriverAccelerometer*>(it->second);

          if (semiAxis->Index() == accelerometer->XIndex())
          {
            accelerometer->SetXIndex(-1);
            if (accelerometer->YIndex() < 0 && accelerometer->ZIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->YIndex())
          {
            accelerometer->SetYIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->ZIndex() < 0)
              m_buttons.erase(it);
          }
          else if (semiAxis->Index() == accelerometer->ZIndex())
          {
            accelerometer->SetZIndex(-1);
            if (accelerometer->XIndex() < 0 && accelerometer->YIndex() < 0)
              m_buttons.erase(it);
          }
          break;
        }
        default:
          break;
      }
    }

    break;
  }
}

void CButtons::UnMapAnalogStick(const ADDON::DriverAnalogStick* analogStick)
{
  ADDON::DriverSemiAxis semiAxis;

  if (analogStick->XIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (analogStick->YIndex() >= 0)
  {
    semiAxis.SetIndex(analogStick->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }
}

void CButtons::UnMapAccelerometer(const ADDON::DriverAccelerometer* accelerometer)
{
  ADDON::DriverSemiAxis semiAxis;

  if (accelerometer->XIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->XIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->YIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->YIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }

  if (accelerometer->ZIndex() >= 0)
  {
    semiAxis.SetIndex(accelerometer->ZIndex());
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE);
    UnMapSemiAxis(&semiAxis);
    semiAxis.SetDirection(JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE);
    UnMapSemiAxis(&semiAxis);
  }
}

bool CButtons::ButtonConflicts(const ADDON::DriverButton* button, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_BUTTON)
  {
    const ADDON::DriverButton* button2 = static_cast<const ADDON::DriverButton*>(feature);
    return button->Index() == button2->Index();
  }
  return false;
}

bool CButtons::HatConflicts(const ADDON::DriverHat* hat, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_HAT_DIRECTION)
  {
    const ADDON::DriverHat* hat2 = static_cast<const ADDON::DriverHat*>(feature);
    return hat->Index()     == hat2->Index()  &&
           hat->Direction() == hat2->Direction();
  }
  return false;
}

bool CButtons::SemiAxisConflicts(const ADDON::DriverSemiAxis* semiAxis, const ADDON::JoystickFeature* feature)
{
  if (feature->Type() == JOYSTICK_DRIVER_TYPE_SEMIAXIS)
  {
    const ADDON::DriverSemiAxis* semiAxis2 = static_cast<const ADDON::DriverSemiAxis*>(feature);
    return semiAxis->Index()     == semiAxis2->Index()  &&
           semiAxis->Direction() == semiAxis2->Direction();
  }
  else if (feature->Type() == JOYSTICK_DRIVER_TYPE_ANALOG_STICK)
  {
    const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);
    return semiAxis->Index() == analogStick->XIndex() ||
           semiAxis->Index() == analogStick->YIndex();
  }
  else if (feature->Type() == JOYSTICK_DRIVER_TYPE_ACCELEROMETER)
  {
    const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);
    return semiAxis->Index() == accelerometer->XIndex() ||
           semiAxis->Index() == accelerometer->YIndex() ||
           semiAxis->Index() == accelerometer->ZIndex();
  }
  return false;
}

bool CButtons::Serialize(TiXmlElement* pElement) const
{
  if (pElement == NULL)
    return false;

  for (Buttons::const_iterator it = m_buttons.begin(); it != m_buttons.end(); ++it)
  {
    const std::string& strFeatureName = it->first;
    const ADDON::JoystickFeature* feature = it->second;

    TiXmlElement featureElement(BUTTONMAP_XML_ELEM_FEATURE);
    TiXmlNode* featureNode = pElement->InsertEndChild(featureElement);
    if (featureNode == NULL)
      return false;

    TiXmlElement* featureElem = featureNode->ToElement();
    if (featureElem == NULL)
      return false;

    featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_NAME, strFeatureName);

    switch (feature->Type())
    {
      case JOYSTICK_DRIVER_TYPE_BUTTON:
      {
        const ADDON::DriverButton* button = static_cast<const ADDON::DriverButton*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON, button->Index());

        break;
      }
      case JOYSTICK_DRIVER_TYPE_HAT_DIRECTION:
      {
        const ADDON::DriverHat* hat = static_cast<const ADDON::DriverHat*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_HAT, JoystickTranslator::TranslateHatDir(hat->Direction()));

        break;
      }
      case JOYSTICK_DRIVER_TYPE_SEMIAXIS:
      {
        const ADDON::DriverSemiAxis* semiAxis = static_cast<const ADDON::DriverSemiAxis*>(feature);

        std::ostringstream strAxis;
        if (semiAxis->Direction() >= 0)
          strAxis << "+";
        strAxis << semiAxis->Index();

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS, strAxis.str());

        break;
      }
      case JOYSTICK_DRIVER_TYPE_ANALOG_STICK:
      {
        const ADDON::DriverAnalogStick* analogStick = static_cast<const ADDON::DriverAnalogStick*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS, analogStick->XIndex());
        if (analogStick->XInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS, analogStick->YIndex());
        if (analogStick->YInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED, "true");

        break;
      }
      case JOYSTICK_DRIVER_TYPE_ACCELEROMETER:
      {
        const ADDON::DriverAccelerometer* accelerometer = static_cast<const ADDON::DriverAccelerometer*>(feature);

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS, accelerometer->XIndex());
        if (accelerometer->XInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS, accelerometer->YIndex());
        if (accelerometer->YInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED, "true");

        featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_ZAXIS, accelerometer->ZIndex());
        if (accelerometer->ZInverted())
          featureElem->SetAttribute(BUTTONMAP_XML_ATTR_FEATURE_ZINVERTED, "true");

        break;
      }
      default:
        break;
    }
  }

  return true;
}

bool CButtons::Deserialize(const TiXmlElement* pElement)
{

  Reset();

  const TiXmlElement* pFeature = pElement->FirstChildElement(BUTTONMAP_XML_ELEM_FEATURE);

  if (!pFeature)
  {
    esyslog("Can't find <%s> tag", BUTTONMAP_XML_ELEM_FEATURE);
    return false;
  }

  while (pFeature)
  {
    const char* name = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_NAME);
    if (!name)
    {
      esyslog("<%s> tag has no \"%s\" attribute", BUTTONMAP_XML_ELEM_FEATURE, BUTTONMAP_XML_ATTR_FEATURE_NAME);
      return false;
    }

    ADDON::JoystickFeature* feature = NULL;

    const char* button = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_BUTTON);
    const char* hat = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_HAT);
    const char* axis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_AXIS);
    const char* xaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_XAXIS);
    const char* yaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_YAXIS);
    const char* zaxis = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_ZAXIS);
    const char* xinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_XINVERTED);
    const char* yinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_YINVERTED);
    const char* zinverted = pFeature->Attribute(BUTTONMAP_XML_ATTR_FEATURE_ZINVERTED);

    if (button)
    {
      int buttonIndex = std::atoi(button);
      feature = new ADDON::DriverButton(0, name, buttonIndex);
    }
    else if (hat)
    {
      const int hatIndex = 0;

      JOYSTICK_DRIVER_HAT_DIRECTION dir = JoystickTranslator::TranslateHatDir(hat);
      if (dir == JOYSTICK_DRIVER_HAT_UNKNOWN)
      {
        esyslog("<%s> tag name=\"%s\" attribute \"%s\" is invalid: \"%s\"",
                BUTTONMAP_XML_ELEM_FEATURE, name, BUTTONMAP_XML_ATTR_FEATURE_HAT, hat);
        return false;
      }

      feature = new ADDON::DriverHat(0, name, hatIndex, dir);
    }
    else if (axis)
    {
      int axisIndex = std::abs(std::atoi(axis));

      JOYSTICK_DRIVER_SEMIAXIS_DIRECTION dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN;

      if (axis[0] == '+')
        dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_POSITIVE;
      else if (axis[0] == '-')
        dir = JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_NEGATIVE;

      if (dir == JOYSTICK_DRIVER_SEMIAXIS_DIRECTION_UNKNOWN)
      {
        esyslog("<%s> tag name=\"%s\" attribute \"%s\" is invalid: \"%s\"",
                BUTTONMAP_XML_ELEM_FEATURE, name, BUTTONMAP_XML_ATTR_FEATURE_AXIS, axis);
        return false;
      }

      feature = new ADDON::DriverSemiAxis(0, name, axisIndex, dir);
    }
    else if (xaxis && yaxis && !zaxis)
    {
      int axisIndexX = std::atoi(xaxis);
      int axisIndexY = std::atoi(yaxis);

      bool bInvertedX = std::string(xinverted ? xinverted : "") == "true";
      bool bInvertedY = std::string(yinverted ? yinverted : "") == "true";

      feature = new ADDON::DriverAnalogStick(0, name,
                                             axisIndexX, bInvertedX,
                                             axisIndexY, bInvertedY);
    }
    else if (xaxis && yaxis && zaxis)
    {
      int axisIndexX = std::atoi(xaxis);
      int axisIndexY = std::atoi(yaxis);
      int axisIndexZ = std::atoi(zaxis);

      bool bInvertedX = std::string(xinverted ? xinverted : "") == "true";
      bool bInvertedY = std::string(yinverted ? yinverted : "") == "true";
      bool bInvertedZ = std::string(zinverted ? zinverted : "") == "true";

      feature = new ADDON::DriverAccelerometer(0, name,
                                               axisIndexX, bInvertedX,
                                               axisIndexY, bInvertedY,
                                               axisIndexZ, bInvertedZ);
    }
    else
    {
      esyslog("Invalid <%s> tag: %s", BUTTONMAP_XML_ELEM_FEATURE, name);
      return false;
    }

    m_buttons[name] = feature;

    pFeature = pFeature->NextSiblingElement(BUTTONMAP_XML_ELEM_FEATURE);
  }

  return true;
}
