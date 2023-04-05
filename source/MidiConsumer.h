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

#include <Messenger.h>
#include <MidiConsumer.h>
#include <SupportDefs.h>
#include <cstdio>


class MidiConsumer : public BMidiLocalConsumer{
public:
				MidiConsumer(BMessenger* messenger);
	virtual		~MidiConsumer();

private:
	void		NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time);

	BMessenger*	fCaller;
};


#endif // _H_MIDI_CONSUMER
