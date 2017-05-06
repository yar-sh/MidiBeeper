/* Main.h - Part of the MidiBeeper - v1.0 - HaselLoyance - public domain.
No warranty implied; use at your own risk

LICENSE
This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.
*/

#ifndef MIDI_BEEPER_MAIN
#define MIDI_BEEPER_MAIN

#include "MidiFile.h"
#include <iostream>
#include <Windows.h>
#include <chrono>
#include <future>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib,"Winmm.lib")

using namespace std;
using namespace std::chrono;

// Information about note (frequency, duration in millisecons)
struct __Note
{
	unsigned int durationMs = 0;
	unsigned int frequencyHz = 0;
};

// Holds a vector of all notes that occur at specified time in milliseconds
struct __EventPoint
{
	unsigned int timeMs = 0;
	vector<__Note> notes;

};
#endif