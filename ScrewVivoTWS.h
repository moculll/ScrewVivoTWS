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
#include "spdlog/spdlog.h"
#include <fstream>
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
    void setUIFromStorage();
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

        std::shared_ptr<nlohmann::json> storageJson;

        VivoSettingStorage()
        {
            storageJson = std::make_shared<nlohmann::json>();
            if(!LoadStorageFromJson("storage.json"))
                return;
            nlohmann::json& storageData = (*storageJson);
            
            noiseMode = static_cast<decltype(noiseMode)>(storageData["NoiseMode"].get<int>());
            deepxEffectMode = static_cast<decltype(deepxEffectMode)>(storageData["DeepXEffectMode"].get<int>());
            doubleClickModeLeft.mode = static_cast<decltype(doubleClickModeLeft.mode)>(storageData["DoubleClickModeLeft"].get<int>());
            doubleClickModeRight.mode = static_cast<decltype(doubleClickModeRight.mode)>(storageData["DoubleClickModeRight"].get<int>());
            longPressModeLeft.mode = static_cast<decltype(longPressModeLeft.mode)>(storageData["LongPressModeLeft"].get<int>());
            longPressModeRight.mode = static_cast<decltype(longPressModeRight.mode)>(storageData["LongPressModeRight"].get<int>());
            doubleClickAcceptCallMode = static_cast<decltype(doubleClickAcceptCallMode)>(storageData["DoubleClickAcceptcallMode"].get<int>());
            longPressRefuseCallMode = static_cast<decltype(longPressRefuseCallMode)>(storageData["LongPressRefuseCallMode"].get<int>());
            wearDetectionMode = static_cast<decltype(wearDetectionMode)>(storageData["WearDetectionMode"].get<int>());

            /*spdlog::info("default: {} {} {} {} {} {} {} {} {}", static_cast<int>(noiseMode), static_cast<int>(deepxEffectMode), static_cast<int>(doubleClickModeLeft.mode), static_cast<int>(doubleClickModeRight.mode), 
                static_cast<int>(longPressModeLeft.mode), static_cast<int>(longPressModeRight.mode),
                static_cast<int>(doubleClickAcceptCallMode), static_cast<int>(longPressRefuseCallMode), static_cast<int>(wearDetectionMode));*/

        }

        bool SaveStorgeToJson(std::string fileName)
        {
            if (!storageJson) {
                spdlog::error("No JSON data to save!");
                return false;
            }

            (*storageJson)["NoiseMode"] = static_cast<int>(noiseMode);
            (*storageJson)["DeepXEffectMode"] = static_cast<int>(deepxEffectMode);
            (*storageJson)["DoubleClickModeLeft"] = static_cast<int>(doubleClickModeLeft.mode);
            (*storageJson)["DoubleClickModeRight"] = static_cast<int>(doubleClickModeRight.mode);
            (*storageJson)["LongPressModeLeft"] = static_cast<int>(longPressModeLeft.mode);
            (*storageJson)["LongPressModeRight"] = static_cast<int>(longPressModeRight.mode);
            (*storageJson)["DoubleClickAcceptcallMode"] = static_cast<int>(doubleClickAcceptCallMode);
            (*storageJson)["LongPressRefuseCallMode"] = static_cast<int>(longPressRefuseCallMode);
            (*storageJson)["WearDetectionMode"] = static_cast<int>(wearDetectionMode);

            try {
                std::ofstream file(fileName);
                if (!file.is_open()) {
                    spdlog::error("Failed to open {} for writing.", fileName);
                    return false;
                }
                file << storageJson->dump(4);
                file.close();
            }
            catch (const std::exception& e) {
                spdlog::error("Error saving to JSON file: {}", e.what());
                return false;
            }

            spdlog::info("Storage saved to {}", fileName);
            return true;
        }

        bool LoadStorageFromJson(std::string fileName)
        {
            
            std::ifstream file(fileName);
            if (!file.is_open()) {
                spdlog::debug("can not load %s.", fileName);
                return false;
            }

            try {
                file >> *storageJson.get();
            }
            catch (nlohmann::json::parse_error& err) {
                spdlog::debug("%s parsing error: %s", fileName, err.what());
                return false;
            }
            return true;
            
        }

        

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