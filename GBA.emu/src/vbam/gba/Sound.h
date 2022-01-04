#ifndef SOUND_H
#define SOUND_H

// Sound emulation setup/options and GBA sound emulation

#include "../System.h"
#include "GBA.h"

namespace EmuEx
{
class EmuAudio;
}

//// Setup/options (these affect GBA and GB sound)

// Initializes sound and returns true if successful. Sets sound quality to
// current value in soundQuality global.
bool soundInit();

// sets the Sound throttle
void soundSetThrottle(unsigned short throttle);

// Manages sound volume, where 1.0 is normal
float soundGetVolume();

// Manages muting bitmask. The bits control the following channels:
// 0x001 Pulse 1
// 0x002 Pulse 2
// 0x004 Wave
// 0x008 Noise
// 0x100 PCM 1
// 0x200 PCM 2
void soundSetEnable( int mask );
int  soundGetEnable();

// Pauses/resumes system sound output
void soundPause();
void soundResume();
//extern bool soundPaused; // current paused state

// Cleans up sound. Afterwards, soundInit() can be called again.
void soundShutdown();

//// GBA sound options

unsigned soundGetSampleRate();
void soundSetSampleRate(GBASys &gba, unsigned sampleRate);

// Sound settings
extern bool &soundInterpolation; // 1 if PCM should have low-pass filtering
extern float &soundFiltering;    // 0.0 = none, 1.0 = max


//// GBA sound emulation

// GBA sound registers
#define SGCNT0_H 0x82
#define FIFOA_L 0xa0
#define FIFOA_H 0xa2
#define FIFOB_L 0xa4
#define FIFOB_H 0xa6

// Resets emulated sound hardware
void soundReset(GBASys &gba);

// Emulates write to sound hardware
void soundEvent( GBASys &gba, u32 addr, u8  data );
void soundEvent( GBASys &gba, u32 addr, u16 data ); // TODO: error-prone to overload like this

// Notifies emulator that a timer has overflowed
void soundTimerOverflow(GBASys &gba, ARM7TDMI &cpu, int which );

// Notifies emulator that PCM rate may have changed
void interp_rate();

// Notifies emulator that SOUND_CLOCK_TICKS clocks have passed
void psoundTickfn(EmuEx::EmuAudio *audio);
extern const int SOUND_CLOCK_TICKS;   // Number of 16.8 MHz clocks between calls to soundTick()
extern int &soundTicks;          // Number of 16.8 MHz clocks until soundTick() will be called

// Saves/loads emulator state
void soundSaveGame( gzFile );
void soundReadGame( GBASys &gba, gzFile, int version );

class Multi_Buffer;

void flush_samples(Multi_Buffer * buffer, EmuEx::EmuAudio *audio);

#endif // SOUND_H
