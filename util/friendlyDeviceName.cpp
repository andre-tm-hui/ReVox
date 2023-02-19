#include "friendlyDeviceName.h"

std::wstring FillDeviceInfo(const std::wstring& deviceInterfaceName) {
  // you need to provide deviceInterfaceName
  // example from my system:
  // `\\?\HID#VID_203A&PID_FFFC&MI_01#7&2de99099&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}`

  DEVPROPTYPE propertyType;
  ULONG propertySize = 0;
  CONFIGRET cr = ::CM_Get_Device_Interface_PropertyW(
      deviceInterfaceName.c_str(), &DEVPKEY_Device_InstanceId, &propertyType,
      nullptr, &propertySize, 0);

  if (cr != CR_BUFFER_SMALL) return L"";

  std::wstring deviceId;
  deviceId.resize(propertySize);
  cr = ::CM_Get_Device_Interface_PropertyW(
      deviceInterfaceName.c_str(), &DEVPKEY_Device_InstanceId, &propertyType,
      (PBYTE)deviceId.data(), &propertySize, 0);

  if (cr != CR_SUCCESS) return L"";

  // here is deviceId will contain device instance id
  // example from my system: `HID\VID_203A&PID_FFFC&MI_01\7&2de99099&0&0000`

  DEVINST devInst;
  cr = ::CM_Locate_DevNodeW(&devInst, (DEVINSTID_W)deviceId.c_str(),
                            CM_LOCATE_DEVNODE_NORMAL);

  if (cr != CR_SUCCESS) return L"";

  propertySize = 0;
  cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName,
                                  &propertyType, nullptr, &propertySize, 0);

  if (cr == CR_BUFFER_SMALL) {
    std::wstring friendlyString;
    friendlyString.resize(propertySize);
    cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName,
                                    &propertyType, (PBYTE)friendlyString.data(),
                                    &propertySize, 0);
    return friendlyString;
  }

  propertySize = 0;
  cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType,
                                  nullptr, &propertySize, 0);

  if (cr == CR_BUFFER_SMALL) {
    std::wstring friendlyString;
    friendlyString.resize(propertySize);
    cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType,
                                    (PBYTE)friendlyString.data(), &propertySize,
                                    0);
    return friendlyString;
  }

  return L"";
}

std::string GetFriendlyDeviceName(std::string deviceName) {
  std::wstring wDeviceName(deviceName.begin(), deviceName.end());
  wDeviceName = FillDeviceInfo(wDeviceName);
  return std::string(wDeviceName.begin(), wDeviceName.end());
}
