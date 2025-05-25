#include "VivoController/VivoController.h"


QString VivoController::btAddressToString(const BLUETOOTH_ADDRESS& addr) {
    return QString("%1:%2:%3:%4:%5:%6")
        .arg((addr.rgBytes[5]), 2, 16, QLatin1Char('0'))
        .arg((addr.rgBytes[4]), 2, 16, QLatin1Char('0'))
        .arg((addr.rgBytes[3]), 2, 16, QLatin1Char('0'))
        .arg((addr.rgBytes[2]), 2, 16, QLatin1Char('0'))
        .arg((addr.rgBytes[1]), 2, 16, QLatin1Char('0'))
        .arg((addr.rgBytes[0]), 2, 16, QLatin1Char('0'))
        .toUpper();
}

std::string VivoController::wideToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";

    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
        nullptr, 0, nullptr, nullptr);
    std::string result(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(),
        &result[0], sizeNeeded, nullptr, nullptr);
    return result;
}

std::vector<VivoController::BtDevice> VivoController::getPairedBluetoothDevices() {
    std::vector<BtDevice> devices;

    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
    searchParams.fReturnAuthenticated = TRUE;
    searchParams.fReturnConnected = TRUE;
    searchParams.fReturnRemembered = TRUE;
    searchParams.fReturnUnknown = FALSE;
    searchParams.hRadio = NULL;
    searchParams.cTimeoutMultiplier = 5;

    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };

    HANDLE hFind = BluetoothFindFirstDevice(&searchParams, &deviceInfo);
    if (hFind == NULL) {
        return devices;
    }

    do {
        BtDevice dev;
        dev.name = wideToUtf8(deviceInfo.szName);
        dev.address = deviceInfo.Address;
        devices.push_back(dev);
    } while (BluetoothFindNextDevice(hFind, &deviceInfo));

    BluetoothFindDeviceClose(hFind);
    return devices;
}
