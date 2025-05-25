#pragma once
#include <vector>
#include <string>
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceInfo>
#include <QBluetoothSocket>
#include <QBluetoothAddress>
#include <stdio.h>
#include <windows.h>
#include <bluetoothapis.h>
#include <QThread>
#include <string_view>
#include <string>
#pragma comment(lib, "Bthprops.lib")

class VivoDeviceCommand {
public:
    
    struct NoiseMaker {
        std::vector<uint8_t> data;

        enum class NoiseMode {
            ReductionMode = 0,
            CloseMode,
            TransparentMode,
            NoiseMode,
        };
        void setMode(NoiseMode mode)
        {
            const auto& map = getBaseCommandMap();
            auto it = map.find("setNoise");
            if (it != map.end()) {
                data = it->second;
                if(data.size() >= 2)
                    data[data.size() - 1 - 1] = static_cast<uint8_t>(mode);
            }
                
        }

        /*NoiseMode getMode()
        {
            if (data.size() >= 2) {
                return static_cast<NoiseMode>(data[data.size() - 2]);
            }
            return NoiseMode::ReductionMode;
        }*/

    };
    
    struct EarDoubleClickMaker {
        /* 0x00 唤醒 0x01 播放 0x02上一首 0x03下一首 0x04 无, 同时只能设置一边, 高位右耳低位左耳*/
        std::vector<uint8_t> data;

        enum class EarDoubleClickMode {
            WakeupVoiceAssistantMode = 0,
            PlayOrPauseMode,
            PrevMode,
            NextMode,
            NoneMode = 0x06,
        };

        void setLeftMode(EarDoubleClickMode mode){
            const auto& map = getBaseCommandMap();
            auto it = map.find("earDoubleClick");
            if (it != map.end()) {
                data = it->second;
                if (data.size()) {
                    data[data.size() - 1] = static_cast<uint8_t>(mode);
                }
            }
            
        }

        void setRightMode(EarDoubleClickMode mode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("earDoubleClick");
            if (it != map.end()) {
                data = it->second;
                if (data.size()) {
                    data[data.size() - 1] = static_cast<uint8_t>(mode) | 0x10;
                }
            }

        }
        /*void setRightDoubleClick(EarDoubleClickMode mode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("setNoise");
            if (it != map.end()) {
                data = it->second;
                if(data.size()) {
                    data[data.size() - 1] = (static_cast<uint8_t>(mode) << 4) & 0xf8;
                }
            }
        }*/

    };


    struct AcceptCallMaker {
        std::vector<uint8_t> data;

        enum class DoubleClickMode {
            DoubleClickNoneMode = 0,
            DoubleClickAcceprOrRefuseMode,
            
        };

        enum class LongPressMode {
            LongPressNoneMode = 0,
            LongPressRefuseMode,
            
        };

        void setMode(DoubleClickMode doubleClickMode, LongPressMode longPressMode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("doubleOrLongPress");
            if (it != map.end()) {
                data = it->second;
                if (data.size()) {
                    data[data.size() - 1] = (static_cast<uint8_t>(doubleClickMode) & 0b10) | (static_cast<uint8_t>(longPressMode) & 0b01);
                }
            }
        }
    };

    struct EarLongPressMaker {
        /* 11 降噪关闭通透切换 8 开启关闭降噪 9开启关闭通透 10降噪通透切换 0xff 无, 高位优先设置,两个字节 低位右耳 高位左耳*/
        std::vector<uint8_t> data;

        enum class EarLongPressMode {
            OpenCloseNoiseMode = 8,
            OpenCloseTransparentMode,
            SwitchBetweenNoiseAndTransparentMode,
            SwitchBetweenNoiseAndCloseAndTransparentMode,
            NoneMode = 0xff,
        };

        void setMode(EarLongPressMode leftMode, EarLongPressMode rightMode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("earLongPress");
            if (it != map.end()) {
                data = it->second;
                if (data.size() >= 2) {
                    data[data.size() - 2] = static_cast<uint8_t>(leftMode) & 0xff;
                    data[data.size() - 1] = static_cast<uint8_t>(rightMode) & 0xff;
                }
            }

        }


    };

    struct DeepxEffectMaker {
        /* 0 默认 1 清澈人声 2 超重低音 3 清亮高音 5 悠扬听书 */
        std::vector<uint8_t> data;

        enum class DeepxEffectMode {
            DefaultMode = 0,
            HumanSoundMode,
            DeepBassMode,
            ClearTrebleMode,
            SoothingMode = 5,
        };

        void setMode(DeepxEffectMode mode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("deepXEffect");
            if (it != map.end()) {
                data = it->second;
                if (data.size()) {
                    data[data.size() - 1] = static_cast<uint8_t>(mode);
                }
            }

        }


    };

    struct WearDetectionMaker {
        
        std::vector<uint8_t> data;

        enum class WearDetectionMode {
            OffMode = 0,
            OnMode,
        };

        void setMode(WearDetectionMode mode) {
            const auto& map = getBaseCommandMap();
            auto it = map.find("wearDetection");
            if (it != map.end()) {
                data = it->second;
                if (data.size()) {
                    data[data.size() - 1] = static_cast<uint8_t>(mode);
                }
            }

        }


    };

    static const std::unordered_map<std::string, std::vector<uint8_t>>& getBaseCommandMap() {
        static const std::unordered_map<std::string, std::vector<uint8_t>> cmdMap = {
            { "setNoise", { 0xFF, 0x03, 0x00, 0x02, 0x00, 0x1b, 0x01, 0x30, 0x00, 0x03 } },
            { "earDoubleClick", { 0xFF, 0x03, 0x00, 0x01, 0x00, 0x1b, 0x01, 0x02, 0x00 } },
            { "doubleOrLongPress", { 0xFF, 0x03, 0x00, 0x02, 0x00, 0x1b, 0x01, 0x50, 0x03, 0x01 } },
            { "earLongPress", { 0xFF, 0x03, 0x00, 0x03, 0x00, 0x1b, 0x01, 0x31, 0x05, 0x00, 0x00 } },
            { "deepXEffect", { 0xFF, 0x03, 0x00, 0x01, 0x00, 0x1b, 0x01, 0x18, 0x00 } },
            { "wearDetection", { 0xFF, 0x03, 0x00, 0x01, 0x00, 0x1b, 0x01, 0x03, 0x00 } },


        };
        return cmdMap;
    }

    static const std::vector<uint8_t> &getBaseCommand(const std::string& name) {
        const auto& map = getBaseCommandMap();
        auto it = map.find(name);
        if (it != map.end())
            return it->second;
        return {};
    }
};

class VivoDevice {
public:
    VivoDevice() : socket(nullptr)
    {

    }
    VivoDevice(std::string name, const QString &addr)
    {
        deviceName = name;
        macAddr = addr;
        printf("created vivo device: %s\r\n", macAddr.toLocal8Bit().toStdString().c_str());
        QBluetoothAddress btAddr(macAddr);
        socket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
        
        QObject::connect(socket, &QBluetoothSocket::disconnected, [&]() {
            printf("device disconnected.\r\n");
            socket->deleteLater();
        });
        QObject::connect(socket, &QBluetoothSocket::connected, [&]() {
            printf("device connected.\r\n");
        });
        /*socket->connectToService(btAddr, 13);*/
    }

    void write(std::vector<uint8_t> data)
    {
        if (!socket)
            return;

        if (socket->state() == QBluetoothSocket::SocketState::ConnectedState) {
 
            QByteArray byteArray(reinterpret_cast<const char*>(data.data()), data.size());
            socket->write(byteArray);
        }
        else {
         
            pendingWriteData = data;

            QObject::connect(socket, &QBluetoothSocket::connected, [&]() {
                if (!pendingWriteData.empty()) {
                    QByteArray byteArray(reinterpret_cast<const char*>(pendingWriteData.data()), pendingWriteData.size());
                    socket->write(byteArray);
                    pendingWriteData.clear();
                
                }
            });

            socket->connectToService(QBluetoothAddress(macAddr), 13);
        }
    }


    std::string deviceName;
    QString macAddr;
    std::vector<uint8_t> pendingWriteData;
    bool waitForConnected(int timeoutMs)
    {
        if (!socket)
            return false;

        const int intervalMs = 10;
        int elapsed = 0;

        while (socket->state() != QBluetoothSocket::SocketState::ConnectedState) {

            QThread::msleep(intervalMs);
            elapsed += intervalMs;
            if (elapsed >= timeoutMs)
                return false;
        }
        return true;
    }

    void connect()
    {
        if(!socket)
            return;
        printf("try connect to channel 13\r\n");
        socket->connectToService(QBluetoothAddress(macAddr), 13);
    }

    ~VivoDevice()
    {
        if (socket) {
         
            printf("destoryed socket, name: %s, addr: %s\r\n", deviceName.c_str(), macAddr.toLocal8Bit().toStdString().c_str());
            socket->close();
            socket->deleteLater();
            socket = nullptr;
        }
            
    }
private:
    QBluetoothSocket* socket;
};

class VivoController {
public:

    static std::unique_ptr<VivoDevice> tryMatch()
    {
        auto result = getPairedBluetoothDevices();
        for (auto dev : result) {
            printf("device: %s\r\n", dev.name.c_str());
            for (auto matchName : supportDevices) {
                if (dev.name == matchName) {
                    QString addrStr = btAddressToString(dev.address);
                    std::unique_ptr<VivoDevice> newDevice = std::make_unique<VivoDevice>(std::string(matchName), addrStr);
                   
                    printf("matched: %s\r\n", addrStr.toLocal8Bit().toStdString().c_str());
                    return newDevice;
                }
            }

        }
        
    }

    static std::unique_ptr<VivoDevice> init()
    {
        auto matchedDevicePtr = tryMatch();
        
        printf("inited.\r\n");
        return matchedDevicePtr;
    }

    static void test()
    {

    }
    
private:
    static constexpr std::array<std::string_view, 1> supportDevices = {
        "vivo TWS 3e",
    };

    struct BtDevice {
        std::string name;
        BLUETOOTH_ADDRESS address;
    };


    /*static void connectToDeviceWithQt(QString& addrStr);*/

    static QString btAddressToString(const BLUETOOTH_ADDRESS& addr);
    static std::string wideToUtf8(const std::wstring& wstr);
    static std::vector<BtDevice> getPairedBluetoothDevices();

    static std::vector<BtDevice> deviceStorage;
};