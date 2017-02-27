
                   
#include "Paul.h"
#include "Arduino.h"

const __FlashStringHelper* getPoemName(PoemType poem) {
	switch (poem) {
		case RunChickenRun: 
			return F("Run, Chicken Run");
		case ShaveMyLegs: 
			return F("Shave my Legs");
		case HowIHateTheNight: 
			return F("How I hate the night");
		default:
			return F("no poem");
	}
} 

const __FlashStringHelper* getPoemDescription(PoemType poem) {
	switch (poem) {
		case RunChickenRun: 
			return F("Helpful description how chicken can survive.");
		case ShaveMyLegs: 
			return F("Having beautiful legs are important for everyone.");
		case HowIHateTheNight: 
			return F("Emotional poem that does not even try to cheer you up. But true.");
		default:
			return F("no poem");
	}
} 


const __FlashStringHelper* getVoiceDescription(VoiceType voice) {

	switch (voice) {
		case PerfectPaul: 
			return F("Paul");
		case HugeHarry: 
			return F("Huge Harry");
		case BeautifulBetty: 
			return F("Beautiful Betty");
		case UppityUrsula: 
			return F("Uppity Ursula");
		case DoctorDennis: 
			return F("Doctor Dennis");
		case KitTheKid: 
			return F("Kit the Kid");
		case RoughRita: 
			return F("Rough Rita");
		case FrailFrank: 
			return F("Frail Frank");
		case WisperingWendy: 
			return F("Wisphering Wendy");
		case ClintEastwood: 
			return F("Client Eastwood");
		case RobotVoice: 
			return F("Robot Voice");
		default:
			return F("no voice");
	}
} 

const __FlashStringHelper* getSongName(SongType song) {
	switch (song) {
		case EdelweissSong: 
			return F("Edelweiﬂ");
		case FavouriteThingsSong1: 
			return F("Favourite things, 1st line");
		case FavouriteThingsSong2: 
			return F("Favourite things, 2nd line");
		case FavouriteThingsSong3: 
			return F("Favourite things, 3rd line");
		case FavouriteThingsSong4: 
			return F("Favourite things, 4th line");
		case HitMeWithYourBestShotSong: 
			return F("Hit me with your best shot");
		default:
			return F("no song");
	}
}

const __FlashStringHelper* getSongDescription(SongType song) {
	switch (song) {
		case EdelweissSong: 
			return F("Enchanting small song making you forget your sorrows.");
		case FavouriteThingsSong1: 
			return F("Depressing, tries to cheer you up, but does not work.");
		case FavouriteThingsSong2: 
			return F("Still.");
		case FavouriteThingsSong3: 
			return F("Yes, that's how I feel!");
		case FavouriteThingsSong4: 
			return F("Nothing left to be said");
		case HitMeWithYourBestShotSong: 
			return F("Good advice. I listen to that all the time.");
		default:
			return F("no song");
	}
} 


const __FlashStringHelper* getLangDescription(LanguageType  lang) {
	switch (lang) {
		case English: 
			return F("English");
		case LatinSpanish: 
			return F("Latin Spanish");
		case CastillianSpanish: 
			return F("Castillian Spanish");
		default:
			return F("no language");
	}
} 

