/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#include "MidiConsumer.h"


MidiConsumer::MidiConsumer(playerConfig* config, BMessenger* messenger)
	:
	fConfig(config),
	fCaller(messenger)
{
	const char* path = "/HiQ-Data/audio/soundeffects/notify.wav";
	entry_ref ref;
	::get_ref_for_path(path, &ref);

	for (int32 i = 0; i < kPadCount; i++) {
		fPlayers[i] = new BFileGameSound(&ref, false);
		if (fPlayers[i]->InitCheck() == B_OK)
			fPlayers[i]->Preload();
	}
}


MidiConsumer::~MidiConsumer()
{
}


void
MidiConsumer::NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time)
{
	int32 id = GetProducerID();
	printf("%lu: [Producer %d][Channel %d / Note %d]  Note on with velocity %d\n", time, id,
		channel, note, velocity);

	BMessage* msg = new BMessage(PLAY);
	for (int32 i = 0; i < kPadCount; i++) {
		if (note == fConfig->note[i]) {
			fPlayers[i]->StartPlaying();

			msg->AddInt32("pad", i);
			fCaller->SendMessage(msg);
		}
	}
}
