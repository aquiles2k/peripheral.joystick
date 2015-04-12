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

#define PERIPHERAL_ADDON_JOYSTICKS

#include "api/Joystick.h"
#include "api/JoystickManager.h"
#include "devices/Devices.h"
#include "log/Log.h"
#include "log/LogAddon.h"
#include "utils/CommonMacros.h"

#include "kodi/libXBMC_addon.h"
#include "kodi/libKODI_peripheral.h"
#include "kodi/xbmc_addon_dll.h"
#include "kodi/kodi_peripheral_dll.h"
#include "kodi/kodi_peripheral_utils.hpp"

#include <vector>

using namespace JOYSTICK;

extern "C"
{

ADDON::CHelper_libXBMC_addon*      FRONTEND;
ADDON::CHelper_libKODI_peripheral* PERIPHERAL;

ADDON_STATUS ADDON_Create(void* callbacks, void* props)
{
  PERIPHERAL_PROPERTIES* peripheralProps = static_cast<PERIPHERAL_PROPERTIES*>(props);

  try
  {
    if (!callbacks || !peripheralProps)
      throw ADDON_STATUS_UNKNOWN;

    FRONTEND = new ADDON::CHelper_libXBMC_addon;
    if (!FRONTEND || !FRONTEND->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;

    PERIPHERAL = new ADDON::CHelper_libKODI_peripheral;
    if (!PERIPHERAL || !PERIPHERAL->RegisterMe(callbacks))
      throw ADDON_STATUS_PERMANENT_FAILURE;
  }
  catch (const ADDON_STATUS& status)
  {
    SAFE_DELETE(PERIPHERAL);
    SAFE_DELETE(FRONTEND);
    return status;
  }

  CLog::Get().SetPipe(new CLogAddon(FRONTEND));

  if (!CJoystickManager::Get().Initialize())
    return ADDON_STATUS_PERMANENT_FAILURE;

  if (!CDevices::Get().Initialize(*peripheralProps))
    return ADDON_STATUS_PERMANENT_FAILURE;

  return ADDON_STATUS_OK;
}

void ADDON_Stop()
{
}

void ADDON_Destroy()
{
  CJoystickManager::Get().Deinitialize();
  CDevices::Get().Deinitialize();

  CLog::Get().SetType(SYS_LOG_TYPE_CONSOLE);

  SAFE_DELETE(PERIPHERAL);
  SAFE_DELETE(FRONTEND);
}

ADDON_STATUS ADDON_GetStatus()
{
  return FRONTEND && PERIPHERAL ? ADDON_STATUS_OK : ADDON_STATUS_UNKNOWN;
}

bool ADDON_HasSettings()
{
  return false;
}

unsigned int ADDON_GetSettings(ADDON_StructSetting ***sSet)
{
  return 0;
}

ADDON_STATUS ADDON_SetSetting(const char *settingName, const void *settingValue)
{
  return ADDON_STATUS_OK;
}

void ADDON_FreeSettings()
{
}

void ADDON_Announce(const char *flag, const char *sender, const char *message, const void *data)
{
}

const char* GetPeripheralAPIVersion(void)
{
  return PERIPHERAL_API_VERSION;
}

const char* GetMinimumPeripheralAPIVersion(void)
{
  return PERIPHERAL_MIN_API_VERSION;
}

PERIPHERAL_ERROR GetAddonCapabilities(PERIPHERAL_CAPABILITIES* pCapabilities)
{
  if (!pCapabilities)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  pCapabilities->provides_joysticks = true;

  return PERIPHERAL_NO_ERROR;
}

PERIPHERAL_ERROR PerformDeviceScan(unsigned int* peripheral_count, PERIPHERAL_INFO** scan_results)
{
  if (!peripheral_count || !scan_results)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  std::vector<CJoystick*> joysticks;
  if (!CJoystickManager::Get().PerformJoystickScan(joysticks))
    return PERIPHERAL_ERROR_FAILED;

  // Upcast array pointers
  std::vector<ADDON::Peripheral*> peripherals;
  peripherals.assign(joysticks.begin(), joysticks.end());

  *peripheral_count = peripherals.size();
  ADDON::Peripherals::ToStructs(peripherals, scan_results);

  return PERIPHERAL_NO_ERROR;
}

void FreeScanResults(unsigned int peripheral_count, PERIPHERAL_INFO* scan_results)
{
  ADDON::Peripherals::FreeStructs(peripheral_count, scan_results);
}

PERIPHERAL_ERROR GetEvents(unsigned int* event_count, PERIPHERAL_EVENT** events)
{
  if (!event_count || !events)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  std::vector<ADDON::PeripheralEvent> peripheralEvents;
  if (CJoystickManager::Get().GetEvents(peripheralEvents))
  {
    *event_count = peripheralEvents.size();
    ADDON::PeripheralEvents::ToStructs(peripheralEvents, events);
    return PERIPHERAL_NO_ERROR;
  }

  return PERIPHERAL_ERROR_FAILED;
}

void FreeEvents(unsigned int event_count, PERIPHERAL_EVENT* events)
{
  ADDON::PeripheralEvents::FreeStructs(event_count, events);
}

PERIPHERAL_ERROR GetJoystickInfo(unsigned int index, JOYSTICK_INFO* info)
{
  if (!info)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  const CJoystick* joystick = CJoystickManager::Get().GetJoystick(index);
  if (!joystick)
    return PERIPHERAL_ERROR_NOT_CONNECTED;

  // Need to be explicit because we're using instead typedef struct { ... }T of struct T{ ... }
  joystick->ADDON::Joystick::ToStruct(*info);

  return PERIPHERAL_NO_ERROR;
}

void FreeJoystickInfo(JOYSTICK_INFO* info)
{
  if (!info)
    return;

  ADDON::Joystick::FreeStruct(*info);
}

PERIPHERAL_ERROR GetButtonMap(const PERIPHERAL_INFO* peripheral, const JOYSTICK_INFO* joystick,
                              const char* device_id,
                              unsigned int* feature_count, JOYSTICK_FEATURE** features)
{
  if (!peripheral || !joystick || !device_id || !feature_count || !features)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  std::vector<ADDON::JoystickFeature*> joystickFeatures;
  if (CDevices::Get().GetFeatures(ADDON::Peripheral(*peripheral), ADDON::Joystick(*joystick),
                                  device_id,  joystickFeatures))
  {
    *feature_count = joystickFeatures.size();
    ADDON::JoystickFeatures::ToStructs(joystickFeatures, features);
    return PERIPHERAL_NO_ERROR;
  }

  return PERIPHERAL_ERROR_FAILED;
}

void FreeButtonMap(unsigned int feature_count, JOYSTICK_FEATURE* features)
{
  ADDON::JoystickFeatures::FreeStructs(feature_count, features);
}

PERIPHERAL_ERROR MapJoystickFeature(const PERIPHERAL_INFO* peripheral, const JOYSTICK_INFO* joystick,
                                    const char* device,
                                    JOYSTICK_FEATURE* feature)
{
  if (!peripheral || !joystick || !device || !feature)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  ADDON::JoystickFeature* joystickFeature = ADDON::JoystickFeatureFactory::Create(*feature);
  if (!joystickFeature)
    return PERIPHERAL_ERROR_INVALID_PARAMETERS;

  bool bSuccess = CDevices::Get().MapFeature(ADDON::Peripheral(*peripheral), ADDON::Joystick(*joystick),
                                                  device, joystickFeature);

  delete joystickFeature;

  return bSuccess ? PERIPHERAL_NO_ERROR : PERIPHERAL_ERROR_FAILED;
}

} // extern "C"
