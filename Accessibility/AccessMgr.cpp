#include "Accessibility/AccessMgr.h"
#include "spdlog/spdlog.h"


namespace Accessibility {

int Speaker<SpeakerType::ZDSR>::init()
{
	HMODULE ZDSRDLL = LoadLibraryA("ZDSRAPI_x64.dll");
	if (!ZDSRDLL) {
		spdlog::info("ZDSR load dll failed.");
		return -1;
	}
	InitTTS = (ZDSRInitTTSFuncType)GetProcAddress(ZDSRDLL, "InitTTS");
	if (!InitTTS) {
		spdlog::info("ZDSR cannot find InitTTS in dll.");
		return -1;
	}

	Speak = (ZDSRSpeakFuncType)GetProcAddress(ZDSRDLL, "Speak");
	if (!Speak) {
		spdlog::info("ZDSR cannot find Speak in dll.");
		return -1;
	}

	StopSpeak = (ZDSRStopSpeakFuncType)GetProcAddress(ZDSRDLL, "StopSpeak");
	if (!StopSpeak) {
		spdlog::info("ZDSR cannot find StopSpeak in dll.");
		return -1;
	}
	return InitTTS(0, NULL, FALSE);

}

int Speaker<SpeakerType::ZDSR>::speak(std::wstring word, bool interrupt)
{
    if(!Speak)
        return -1;

    return Speak(word.c_str(), interrupt);
}

void Speaker<SpeakerType::ZDSR>::stopSpeak()
{
	if(StopSpeak)
		StopSpeak();
}

int Speaker<SpeakerType::QT_SAPI>::init()
{
	speaker = std::make_unique<QTextToSpeech>();

	QList<QVoice> voices = speaker->availableVoices();
	if (!voices.isEmpty()) {
		speaker->setVoice(voices.first());
	}
		
	return 0;
}

int Speaker<SpeakerType::QT_SAPI>::speak(std::wstring word, bool interrupt)
{
	if(!speaker)
		return -1;

	speaker->say(QString::fromStdWString(word));
	return 0;
}

void Speaker<SpeakerType::QT_SAPI>::stopSpeak()
{
	speaker->stop();
}



} /* Accessibility */