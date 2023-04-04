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
			| B_AUTO_UPDATE_SIZE_LIMITS)
{
	fPlayerConfig = new playerConfig;

	fMessenger = new BMessenger(this, NULL);
	fConsumer = new MidiConsumer(fPlayerConfig, fMessenger);
	fRoster = BMidiRoster::MidiRoster();
	fRoster->StartWatching(fMessenger);

	// building layout
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu = new BMenu(B_TRANSLATE_SYSTEM_NAME("Samedi"));
	BMenuItem* menuItem = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(menuItem);
	menuBar->AddItem(menu);

	for (int32 i = 0; i < kPadCount; i++)
		fPads[i] = new Pad(i, fPlayerConfig->note[i]);

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
}


MainWindow::~MainWindow()
{
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
			fPlayerConfig->solo[pad] = on_off;
			break;
		}
		case LOOP:
		{
			int32 pad = -1;
			int32 on_off = -1;
			msg->FindInt32("pad", &pad);
			msg->FindInt32("loop", &on_off);
			fPlayerConfig->loop[pad] = on_off;
			break;
		}
		case PLAY:
		{
			int32 pad = -1;
			msg->FindInt32("pad", &pad);
			printf("Pad %i: Ding!\n", pad);
		}
		case EJECT:
		{
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
