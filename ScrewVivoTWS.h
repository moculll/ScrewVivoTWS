#pragma once
#include <QtWidgets/QWidget>
#include <QTimer>
#include "VivoController/VivoController.h"
#include "MTrayMenu/MTrayMenu.h"
#include "nlohmann/json.hpp"
#include <string>
#include <typeinfo>
#include <unordered_map>
#include <QString>

class ScrewVivoTWS
{
public:
    
    ScrewVivoTWS();
    ~ScrewVivoTWS();
    std::unique_ptr<VivoDevice> device;
    std::unique_ptr<MTrayMenu> trayMenu;

    std::shared_ptr<nlohmann::json> uiTextJsonData;
    std::unique_ptr<QTimer> readAsyncTimer;

    std::string language;

    bool loadJson(std::string fileName);
    void getJsonToSettingStorage(nlohmann::json& data);
    void parseData(const std::vector<uint8_t>& buffer);

    template <typename EnumType, bool leftOrRight>
    struct DoubleClickKey {
        EnumType mode;
        static constexpr bool isLeft = leftOrRight;
        bool operator==(const DoubleClickKey<EnumType, leftOrRight>& other) const {
            return mode == other.mode;
        }
        
    };
    
    
    class EnumStringMapper {
    public:
        template<typename EnumType>
        void addMapping(EnumType enumValue, const QString& str) {
            auto& map = getMap<EnumType>();
            map[enumValue] = str;
        }

        template<typename EnumType>
        void removeMapping(EnumType enumValue) {
            auto& map = getMap<EnumType>();
            map.erase(enumValue);
        }

        template<typename EnumType>
        QString toString(EnumType enumValue) const {
            const auto& map = getMap<EnumType>();
            auto it = map.find(enumValue);
            return it != map.end() ? it->second : "Unknown";
        }

        template<typename EnumType, bool leftOrRight>
        void addDoubleClickMapping(EnumType enumValue, const QString& str) {
            DoubleClickKey<EnumType, leftOrRight> key;
            key.mode = enumValue;
            getDoubleClickMap<EnumType, leftOrRight>()[key] = str;
        }

        template<typename EnumType, bool leftOrRight>
        QString doubleClickToString(EnumType enumValue) const {
            DoubleClickKey<EnumType, leftOrRight> key;
            key.mode = enumValue;
            const auto& map = getDoubleClickMap<EnumType, leftOrRight>();
            auto it = map.find(key);
            return it != map.end() ? it->second : "Unknown";
        }

    private:
        template<typename EnumType>
        static std::unordered_map<EnumType, QString>& getMap() {
            static std::unordered_map<EnumType, QString> instance;
            return instance;
        }

        template<typename EnumType, bool leftOrRight>
        static std::unordered_map<DoubleClickKey<EnumType, leftOrRight>, QString>& getDoubleClickMap() {
            static std::unordered_map<DoubleClickKey<EnumType, leftOrRight>, QString> instance;
            return instance;
        }
    };

    

    struct VivoSettingStorage : EnumStringMapper {
        
        VivoDeviceCommand::NoiseMaker::NoiseMode noiseMode;
        VivoDeviceCommand::DeepxEffectMaker::DeepxEffectMode deepxEffectMode;
        DoubleClickKey<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, true> doubleClickModeLeft;
        DoubleClickKey<VivoDeviceCommand::EarDoubleClickMaker::EarDoubleClickMode, false> doubleClickModeRight;
        DoubleClickKey<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, true> longPressModeLeft;
        DoubleClickKey<VivoDeviceCommand::EarLongPressMaker::EarLongPressMode, false> longPressModeRight;
        VivoDeviceCommand::AcceptCallMaker::DoubleClickMode doubleClickAcceptCallMode;
        VivoDeviceCommand::AcceptCallMaker::LongPressMode longPressRefuseCallMode;
        VivoDeviceCommand::WearDetectionMaker::WearDetectionMode wearDetectionMode;

        QString leftBattery;
        QString rightBattery;

    };
    VivoSettingStorage settingStorage;
};

namespace std {

template <typename EnumType, bool leftOrRight>
struct hash<ScrewVivoTWS::DoubleClickKey<EnumType, leftOrRight>> {
    size_t operator()(const ScrewVivoTWS::DoubleClickKey<EnumType, leftOrRight>& key) const noexcept {
        return std::hash<EnumType>{}(key.mode);
    }
};
} /* std */