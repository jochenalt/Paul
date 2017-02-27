
#ifndef PAUL_H_
#define PAUL_H_
                   
#include "Arduino.h"

#define LOW_BAT_3S (3.3*3) // low batt is indicated below 10.0 V (LiPo 3S)
#define LOW_BAT_2S (3.3*2)  // low batt is indicated below 6.6 V (LiPo 2S)

#define NUMBER_OF_SONGS 6
enum SongType {NoSong, EdelweissSong, FavouriteThingsSong1,FavouriteThingsSong2,FavouriteThingsSong3, FavouriteThingsSong4,HitMeWithYourBestShotSong};
#define NUMBER_OF_POEMS 3
enum PoemType {NoPoem, RunChickenRun, ShaveMyLegs, HowIHateTheNight};
#define NUMBER_OF_VOICES 11
enum VoiceType {PerfectPaul,HugeHarry,BeautifulBetty,UppityUrsula,DoctorDennis,KitTheKid,FrailFrank,RoughRita,WisperingWendy, ClintEastwood, RobotVoice};
#define NUMBER_OF_LANGUAGES 3
enum LanguageType {English=0, LatinSpanish=1, CastillianSpanish=2};
#define INITIAL_VOLUME 96
#define INITIAL_SPEECH_RATE 120

#define F_P(string_literal) (reinterpret_cast<const __FlashStringHelper *>(string_literal))

const __FlashStringHelper* getPoemName(PoemType poem);
const __FlashStringHelper* getPoemDescription(PoemType poem);
const __FlashStringHelper* getSongName(SongType poem);
const __FlashStringHelper* getSongDescription(SongType poem);
const __FlashStringHelper* getLangDescription(LanguageType  lang);
const __FlashStringHelper* getVoiceDescription(VoiceType voice);

// frequency the remote control sends a request to the main board
#define REMOTECONTROL_FREQUENCY 7 // [Hz]



#endif