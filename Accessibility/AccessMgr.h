#pragma once
#include <Windows.h>
#include <string>
#include <memory>
#include <QTextToSpeech>
namespace Accessibility {

enum class SpeakerType {
    ZDSR,
    /* most of the time qt uses sapi? */
    QT_SAPI,
};

class SpeakerBase {
public:
    virtual int init() = 0;
    virtual int speak(std::wstring word, bool interrupt) = 0;
    virtual void stopSpeak() = 0;
};

template <SpeakerType>
class Speaker;

template <>
class Speaker<SpeakerType::ZDSR> : public SpeakerBase {
public:
    int init() override;

    int speak(std::wstring word, bool interrupt) override;

    void stopSpeak() override;

private:
    using ZDSRSpeakFuncType = int (*)(const WCHAR*, BOOL);
    using ZDSRInitTTSFuncType = int (*)(int, const WCHAR*, BOOL);
    using ZDSRStopSpeakFuncType = void (*)(void);
    ZDSRSpeakFuncType Speak = nullptr;
    ZDSRInitTTSFuncType InitTTS = nullptr;
    ZDSRStopSpeakFuncType StopSpeak = nullptr;
    

};

template <>
class Speaker<SpeakerType::QT_SAPI> : public SpeakerBase {
public:
    int init() override;

    int speak(std::wstring word, bool interrupt) override;

    void stopSpeak() override;

private:
    std::unique_ptr<QTextToSpeech> speaker;
};

class AccessMgr {
public:
    AccessMgr(SpeakerType type)
    {
        if (SpeakerType::ZDSR == type) {
            speaker = std::make_unique<Speaker<SpeakerType::ZDSR>>();
        }
        else if(SpeakerType::QT_SAPI == type)
        {
            speaker = std::make_unique<Speaker<SpeakerType::QT_SAPI>>();
        }
    }
    std::unique_ptr<SpeakerBase> speaker;
    
};



} /* Accessibility */