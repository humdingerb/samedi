/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 * Based on Ondřej Lukeš HaikuMIDILogger, 2019
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#ifndef _H_MIDI_CONSUMER
#define _H_MIDI_CONSUMER

#include "Constants.h"
#include "Pad.h"

#include <FileGameSound.h>
#include <MidiConsumer.h>
#include <SupportDefs.h>
#include <cstdio>


struct playerConfig {
	uchar note[kPadCount] = { 43, 44, 45, 46, 47, 48, 49, 50 };
	bool mute[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	bool solo[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	bool loop[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
};


class MidiConsumer : public BMidiLocalConsumer{
public:
				MidiConsumer(playerConfig* config, BMessenger* messenger);
	virtual		~MidiConsumer();

private:
	void		NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time);

	playerConfig*		fConfig;
	BMessenger*			fCaller;
	BFileGameSound*		fPlayers[kPadCount];
};


#endif // _H_MIDI_CONSUMER
