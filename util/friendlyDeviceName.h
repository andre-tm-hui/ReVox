#ifndef FRIENDLYDEVICENAME_H
#define FRIENDLYDEVICENAME_H

#include <Windows.h>
#include <cfgmgr32.h>
#include <hidsdi.h>
#include <initguid.h>

#include <string>

#include "devpkey.h"

std::wstring FillDeviceInfo(const std::wstring& deviceInterfaceName);

std::string GetFriendlyDeviceName(std::string deviceName);

#endif  // FRIENDLYDEVICENAME_H
