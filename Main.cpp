/* Main.cpp - Part of the MidiBeeper - v1.0 - HaselLoyance - public domain.
No warranty implied; use at your own risk

LICENSE
This software is dual-licensed to the public domain and under the following
license: you are granted a perpetual, irrevocable license to copy, modify,
publish, and distribute this file as you see fit.
*/

#include "Main.h"

// Returns index of element with the next closest timeMs to the ms
// Accounts for any processor lag
int GetClosest(int start, int ms, const vector<__EventPoint> & notes)
{
	for (int i = start; i < notes.size(); i++)
	{
		if (notes[i].timeMs > ms)
			return i-1;
	}
	return start;
}

int main(int argc, char** argv) 
{
	bool prioritizeHighest = false;
	bool doNoteRecalc = false;
	unsigned int minFrequency = 0;
	string inputFileName = "";

	// Parse argv parameters
	for (int i = 0; i < argc; i++)
	{
		if (string(argv[i]) == "-f")
		{
			minFrequency = stoi(argv[++i]);
		}
		else if (string(argv[i]) == "-i")
		{
			inputFileName = argv[++i];
		}
		else if (string(argv[i]).find('p') != string::npos)
		{
			prioritizeHighest = true;
		}
		else if (string(argv[i]).find('r') != string::npos)
		{
			doNoteRecalc = true;
		}
		else if (string(argv[i]).find('h') != string::npos)
		{
			cout << "MidiBeeper by HaselLoyance https://github.com/HaselLoyance/MidiBeeper" << endl;
			cout << "-i <string> - path to midi file" << endl;
			cout << "-f <number> - only play frequncies that are higher than <number>" << endl;
			cout << "-p - if same tick has multiple notes, only the highest one will play" << endl;
			cout << "-r - do note timing recalculations while midi play (accounts for processor lag)" << endl;
			cout << "-h - show this message" << endl;
			return 0;
		}
	}

	// Events from MIDI file
	vector<__EventPoint> eventPoints;

	{
		// Load midi and calculate time of notes start and end points
		MidiFile midifile;
		midifile.read(inputFileName);
		midifile.joinTracks();
		midifile.linkNotePairs();
		midifile.doTimeAnalysis();

		for (int i = 0; i < midifile[0].size(); i++) {
			MidiEvent * mev = &midifile[0][i];
			__EventPoint tempEventPoint;
			__Note tempNote;

			// Ignore any event that is not "note begin"
			if ((int)(*mev)[0] >= 0x90 && (int)(*mev)[0] <= 0x99)
			{
				tempNote.frequencyHz = round(440.0 * (pow(2.0, (((int)(*mev)[1] - 69.0) / 12.0))));
				tempEventPoint.timeMs = mev->seconds*1000.0;
				tempNote.durationMs = mev->getDurationInSeconds() * 1000.0;

				//Ignore lowest notes
				if (tempNote.frequencyHz < minFrequency)
					continue;

				if (eventPoints.size() != 0 && eventPoints.back().timeMs == tempEventPoint.timeMs)
				{
					if (prioritizeHighest && eventPoints.back().notes[0].frequencyHz < tempNote.frequencyHz)
					{
						eventPoints.back().notes[0] = tempNote;
					}
					else if(!prioritizeHighest)
					{
						eventPoints.back().notes.push_back(tempNote);
					}
				}
				else
				{
					tempEventPoint.notes.push_back(tempNote);
					eventPoints.push_back(tempEventPoint);
				}
			}
		}
	}

	cout << "Playing: " << inputFileName << endl;

	milliseconds startTick = duration_cast<milliseconds>(system_clock::now().time_since_epoch());

	// Position of the next note to play in eventPoints vector
	int i = 0;

	while (true)
	{
		if (i + 1 == eventPoints.size())
			break;

		milliseconds currentTick = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
		unsigned int songTick = currentTick.count() - startTick.count();

		// Any event occurs at the current time mark
		if (songTick >= eventPoints[i].timeMs)
		{
			// Play all notes
			// As you can see - there is no Beep function at all. The reason for this is
			// that Beep is a blocking function. While Beep is running - the entire program
			// waits for it to finish playing the sound. I tested it with multithreading and
			// async calls, but every Beep call interrupts previously playing beep, so instead
			// I cut note frequencies into wav files that actually can be played asynchronously
			for each (__Note n in eventPoints[i].notes)
				mciSendString(("play sounds/" + to_string(n.frequencyHz) + ".wav to " + to_string(n.durationMs)).c_str(), NULL, NULL, NULL);

			if (doNoteRecalc)
			{
				// Recalculate what note should play next
				// Accounts for any processor lag
				i = GetClosest(i, songTick, eventPoints);
			}
			else
				i++;
		}
	}
	return 0;
}