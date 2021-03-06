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
 */

#include "JoystickInterface.h"
#include "Joystick.h"
#include "utils/CommonMacros.h"

using namespace JOYSTICK;

CJoystickInterface::CJoystickInterface(const std::string& strName)
 : m_strName(strName)
{
}

bool CJoystickInterface::ScanForJoysticks(std::vector<CJoystick*>& results)
{
  bool bReturn(false);

  std::vector<CJoystick*> joysticks;
  if (PerformJoystickScan(joysticks))
  {
    bReturn = true;
    results.insert(results.end(), joysticks.begin(), joysticks.end());
  }

  return bReturn;
}

void CJoystickInterface::AddScanResult(CJoystick* joystick)
{
  m_scanResults.push_back(joystick);
}

void CJoystickInterface::GetScanResults(std::vector<CJoystick*>& joysticks) const
{
  joysticks.insert(joysticks.end(), m_scanResults.begin(), m_scanResults.end());
}

void CJoystickInterface::ClearScanResults(void)
{
  for (std::vector<CJoystick*>::iterator it = m_scanResults.begin(); it != m_scanResults.end(); ++it)
    delete *it;
  m_scanResults.clear();
}
