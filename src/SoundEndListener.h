
#include<irrKlang.h>
#include<iostream>

namespace Aftr {
    class SoundEndListener : public irrklang::ISoundStopEventReceiver {
        public:
            virtual void OnSoundStopped(irrklang::ISound* sound, irrklang::E_STOP_EVENT_CAUSE reason, void* userData) {
                sound->setIsPaused(true);
                std::cout << "Sound has stopped!" << std::endl;
            }
    };
}
