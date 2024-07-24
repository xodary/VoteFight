#pragma once
#include "inc/fmod.hpp"
#pragma comment (lib, "lib/fmodex64_vc.lib")

using namespace FMOD;
class CSoundManager
{
private:
	FMOD::System*  m_System{};

	FMOD::Channel* m_Channels[MAX_BGM_SOUNDS + MAX_SFX_SOUNDS]{};
	FMOD::Sound*   m_Sounds[MAX_BGM_SOUNDS + MAX_SFX_SOUNDS]{};

private:
	CSoundManager() = default;
	virtual ~CSoundManager();

public:
	static CSoundManager* GetInstance();

	void Init();

	bool IsPlaying(SOUND_TYPE SoundType);

	void Play(SOUND_TYPE SoundType, float Volume, bool Overlap);
	void Stop(SOUND_TYPE SoundType);
	void Pause(SOUND_TYPE SoundType);
	void Resume(SOUND_TYPE SoundType);

	void Update();

	const char* FMOD_ErrorString(FMOD_RESULT result);
	const string GetSoundPath(const char* SoundName);
};
