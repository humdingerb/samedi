/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#include "MainWindow.h"

#include <Box.h>
#include <Catalog.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Path.h>
#include <ScrollView.h>
#include <SeparatorView.h>

#include <stdio.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


MainWindow::MainWindow()
	:
	BWindow(BRect(200, 200, 600, 300), B_TRANSLATE_SYSTEM_NAME("Samedi"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE
			| B_AUTO_UPDATE_SIZE_LIMITS),
	fOpenPanel(new BFilePanel(B_OPEN_PANEL))
{
	fPlayerConfig = new playerConfig;

	// init pads, player-config and players
	const char* path = "/HiQ-Data/audio/soundeffects/notify.wav";
	for (int32 i = 0; i < kPadCount; i++) {
		fPads[i] = new Pad(i, fPlayerConfig->note[i]);
		fPlayerConfig->sample[i] = path;
		fPlayers[i] = NULL;
		_SetSample(i);
	}

	// building layout
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu = new BMenu(B_TRANSLATE_SYSTEM_NAME("Samedi"));
	BMenuItem* menuItem = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(menuItem);
	menuBar->AddItem(menu);

	BView* padView = new BView("padView", 0);
	BLayoutBuilder::Group<>(padView, B_VERTICAL, 2)
		.Add(fPads[0])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[1])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[2])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[3])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[4])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[5])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[6])
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(fPads[7]);

	BScrollView* scrollView = new BScrollView("scrollview", padView, B_WILL_DRAW, false, true);

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.AddGroup(B_VERTICAL)
			.SetInsets(-2, -2, -2, -2)
			.Add(scrollView)
			.End();

	fMessenger = new BMessenger(this, NULL);
	fConsumer = new MidiConsumer(fMessenger);
	fRoster = BMidiRoster::MidiRoster();
	fRoster->StartWatching(fMessenger);
}


MainWindow::~MainWindow()
{
	delete fOpenPanel;
	delete fMessenger;
	fConsumer->Release();
}


// #pragma mark -


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_MIDI_EVENT:
		{
			_HandleMIDI(msg);
			break;
		}
		case NOTE:
		{
			int32 pad = -1;
			int32 note = -1;
			msg->FindInt32("pad", &pad);
			msg->FindInt32("note", &note);
			fPlayerConfig->note[pad] = note;
			break;
		}
		case MUTE:
		{
			int32 pad = -1;
			int32 on_off = -1;
			msg->FindInt32("pad", &pad);
			msg->FindInt32("mute", &on_off);
			fPlayerConfig->mute[pad] = on_off;
			break;
		}
		case SOLO:
		{
			int32 pad = -1;
			int32 on_off = -1;
			msg->FindInt32("pad", &pad);
			msg->FindInt32("solo", &on_off);

			if (on_off == B_CONTROL_ON) {
				for (int32 i = 0; i < kPadCount; i++) {
					fPlayerConfig->mute[i] = true;
					fPads[i]->Mute(B_CONTROL_ON);
				}
				fPlayerConfig->mute[pad] = false;
				fPads[pad]->Mute(B_CONTROL_OFF);
			} else {
				for (int32 i = 0; i < kPadCount; i++) {
					fPlayerConfig->mute[i] = false;
					fPads[i]->Mute(B_CONTROL_OFF);
				}
			}
			break;
		}
		case LOOP:
		{
			int32 pad = -1;
			int32 on_off = -1;
			msg->FindInt32("pad", &pad);
			msg->FindInt32("loop", &on_off);
			fPlayerConfig->loop[pad] = on_off;
			_SetSample(pad);
			break;
		}
		case PLAY:
		{
			int32 note = -1;
			msg->FindInt32("note", &note);
			for (int32 i = 0; i < kPadCount; i++) {
				if (note == fPlayerConfig->note[i] and fPlayerConfig->mute[i] == false)
					fPlayers[i]->StartPlaying();
			}
			break;
		}
		case EJECT:
		{
			int32 pad = -1;
			msg->FindInt32("pad", &pad);
			fPlayerConfig->sample[pad] = "";
			_SetSample(pad);
			break;
		}
		case OPEN:
		{
			int32 pad = -1;
			msg->FindInt32("pad", &pad);
			BMessage* openMsg = new BMessage(LOAD);
			openMsg->AddInt32("pad", pad);
			fOpenPanel->SetTarget(this);
			fOpenPanel->SetMessage(openMsg);
			fOpenPanel->Show();
			break;
		}
		case LOAD:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK) {
				int32 pad = -1;
				msg->FindInt32("pad", &pad);
				BPath path(&ref);
				fPlayerConfig->sample[pad] = path.Path();
				_SetSample(pad);
			}
			break;
		}
		default:
		{
			BWindow::MessageReceived(msg);
			break;
		}
	}
}


void
MainWindow::_SetSample(int32 pad)
{
	if (fPlayers[pad] != NULL)
		delete fPlayers[pad];

	entry_ref ref;
	::get_ref_for_path(fPlayerConfig->sample[pad], &ref);
	bool loop = fPlayerConfig->loop[pad];

	fPlayers[pad] = new BFileGameSound(&ref, loop);
	if (fPlayers[pad]->InitCheck() == B_OK) {
		fPlayers[pad]->Preload();
		BPath path(&ref);
		fPads[pad]->SetSampleName(path.Leaf());
	}
}


void
MainWindow::_HandleMIDI(BMessage* msg)
{
	int32 op;
	msg->FindInt32("be:op", &op);
	switch (op) {
		case B_MIDI_REGISTERED:
		{
			int32 id = 0;
			BMidiProducer* producer;
			if ((producer = fRoster->NextProducer(&id)) != NULL) {
				printf("Found producer %d: %s\n", id, producer->Name());
				producer->Connect(fConsumer);
			}
			break;
		}
	}
}


void
MainWindow::_PrintConfig()
{
	for (int32 i = 0; i < kPadCount; i++) {
		printf("Pad: %i\n", i);
		printf("note:\t");
		for (int32 j = 0; j < kPadCount; j++)
			printf("%i, ", fPlayerConfig->note[j]);
		printf("\nmute:\t");
		for (int32 j = 0; j < kPadCount; j++)
			printf("%i, ", fPlayerConfig->mute[j]);
		printf("\nloop:\t");
		for (int32 j = 0; j < kPadCount; j++)
			printf("%i, ", fPlayerConfig->loop[j]);
		printf("\nsample:\t");
		for (int32 j = 0; j < kPadCount; j++)
			printf("%s\n", fPlayerConfig->sample[j]);
		printf("\n");
	}
	printf("\n");
}
