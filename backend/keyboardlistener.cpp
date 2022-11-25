#include "keyboardlistener.h"

std::wstring FillDeviceInfo(const std::wstring& deviceInterfaceName)
{
    // you need to provide deviceInterfaceName
    // example from my system: `\\?\HID#VID_203A&PID_FFFC&MI_01#7&2de99099&0&0000#{378de44c-56ef-11d1-bc8c-00a0c91405dd}`

    DEVPROPTYPE propertyType;
    ULONG propertySize = 0;
    CONFIGRET cr = ::CM_Get_Device_Interface_PropertyW(deviceInterfaceName.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, nullptr, &propertySize, 0);

    if (cr != CR_BUFFER_SMALL)
        return L"";

    std::wstring deviceId;
    deviceId.resize(propertySize);
    cr = ::CM_Get_Device_Interface_PropertyW(deviceInterfaceName.c_str(), &DEVPKEY_Device_InstanceId, &propertyType, (PBYTE)deviceId.data(), &propertySize, 0);

    if (cr != CR_SUCCESS)
        return L"";

    // here is deviceId will contain device instance id
    // example from my system: `HID\VID_203A&PID_FFFC&MI_01\7&2de99099&0&0000`

    DEVINST devInst;
    cr = ::CM_Locate_DevNodeW(&devInst, (DEVINSTID_W)deviceId.c_str(), CM_LOCATE_DEVNODE_NORMAL);

    if (cr != CR_SUCCESS)
        return L"";

    propertySize = 0;
    cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName, &propertyType, nullptr, &propertySize, 0);

    if (cr == CR_BUFFER_SMALL)
    {
        std::wstring friendlyString;
        friendlyString.resize(propertySize);
        cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_Device_FriendlyName, &propertyType, (PBYTE)friendlyString.data(), &propertySize, 0);
        return friendlyString;
    }

    propertySize = 0;
    cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, nullptr, &propertySize, 0);

    if (cr == CR_BUFFER_SMALL)
    {
        std::wstring friendlyString;
        friendlyString.resize(propertySize);
        cr = ::CM_Get_DevNode_PropertyW(devInst, &DEVPKEY_NAME, &propertyType, (PBYTE)friendlyString.data(), &propertySize, 0);
        return friendlyString;
    }

    return L"";
}

KeyboardListener::KeyboardListener() {}

LRESULT CALLBACK KeyboardListener::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INPUT:
        // Get the RawInputData
        UINT dwSize;
        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
        LPBYTE lpb = new BYTE[dwSize];
        if (lpb == NULL) return 0;
        if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
            OutputDebugString(TEXT("GetRawInputData does not return correct size!\n"));

        RAWINPUT *raw = (RAWINPUT*)lpb;

        // Get the identifying data - code, deviceName, and the action - event
        USHORT code = raw->data.keyboard.VKey;
        USHORT event = raw->data.keyboard.Flags;

        TCHAR ridDeviceName[256];
        UINT cbDataSize = 256;
        GetRawInputDeviceInfo(raw->header.hDevice, RIDI_DEVICENAME, ridDeviceName, &cbDataSize);
        if (cbDataSize == 0 || cbDataSize == UINT(-1))
            OutputDebugString(TEXT("Failed to get device name!\n"));


        std::wstring wDeviceName = FillDeviceInfo(ridDeviceName);
        std::string deviceName(wDeviceName.begin(), wDeviceName.end());
        deviceName.erase(std::remove_if(deviceName.begin(), deviceName.end(),
                                        [](char c){ return c == '\u0000' || !(c >= 0 && c < 128); }), deviceName.end());

        //audioManager->KeyEvent(code, deviceName, event); // Manage all keyboard events in AudioManager obj
        mi->KeyEvent(code, deviceName, event);
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int KeyboardListener::Start(HWND hWnd)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    WNDCLASSEXW wcex = {};
    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.lpszClassName = TEXT("KbdListener");
    wcex.hInstance = hInstance;
    if (RegisterClassEx(&wcex))
        hWnd = CreateWindowEx(0, wcex.lpszClassName, L"KbdListener", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInstance, NULL);

    rid.usUsagePage = 0x01;
    rid.usUsage = 0x06;
    rid.dwFlags = RIDEV_INPUTSINK;
    rid.hwndTarget = hWnd;
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) { std::cout<<"failed"<<std::endl;}
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)&WndProc);
    return 0;
}
