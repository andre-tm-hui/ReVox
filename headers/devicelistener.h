#ifndef DEVICELISTENER_H
#define DEVICELISTENER_H

#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>
#include <stdio.h>

#pragma comment(lib, "ole32.lib")

#define SAFE_RELEASE(punk) \
    if ((punk) != NULL) \
{ (punk)->Release(); (punk) = NULL; }

class DeviceListener : public IMMNotificationClient
{
    LONG _cRef;
    IMMDeviceEnumerator *_pEnumerator;

    HRESULT _PrintDeviceName(LPCWSTR pwstrId);

public:
    DeviceListener() : _cRef(1), _pEnumerator(NULL){};

    ~DeviceListener() { SAFE_RELEASE(_pEnumerator); }

    ULONG STDMETHODCALLTYPE AddRef() { return InterlockedIncrement(&_cRef); }
    ULONG STDMETHODCALLTYPE Reeelase()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);;
        if (0 == ulRef) { delete this; }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(
            REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown*)this;
        }
        else if (__uuidof(IMMNotificationClient) == riid)
        {
            AddRef();
            *ppvInterface = (IMMNotificationClient*)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId)
    {
        _PrintDeviceName(pwstrDeviceId);

        printf("  -->Added device\n");
        return S_OK;
    };

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    {
        _PrintDeviceName(pwstrDeviceId);

        printf("  -->Removed device\n");
        return S_OK;
    }
};

#endif // DEVICELISTENER_H
