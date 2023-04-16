/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#include "MidiConsumer.h"


MidiConsumer::MidiConsumer(BMessenger* messenger)
	:
	fCaller(messenger)
{
}


MidiConsumer::~MidiConsumer()
{
}


void
MidiConsumer::NoteOn(uchar channel, uchar note, uchar velocity, bigtime_t time)
{
	int32 id = GetProducerID();
	printf("%" B_PRId64 ": [Producer %" B_PRId32 "][Channel %d / Note %d]  Note on with velocity %d\n", time, id,
		channel, note, velocity);

	BMessage* msg = new BMessage(PLAY);
	msg->AddInt32("note", note);
	fCaller->SendMessage(msg);
}
