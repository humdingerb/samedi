/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Constants.h"
#include "MidiConsumer.h"
#include "Pad.h"

#include <FileGameSound.h>
#include <FilePanel.h>
#include <Messenger.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <Window.h>

struct playerConfig {
	uchar note[kPadCount] = { 43, 44, 45, 46, 47, 48, 49, 50 };
	bool mute[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	bool solo[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	bool loop[kPadCount] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	const char* sample[kPadCount] = { "", "", "", "", "", "", "", "",  };
};


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	void			MessageReceived(BMessage* msg);

private:
	void			_SetSample(int32 pad);
	void			_HandleMIDI(BMessage* msg);
	void		_PrintConfig();		// for debugging

	Pad*			fPads[kPadCount];

	BFileGameSound*	fPlayers[kPadCount];
	playerConfig*	fPlayerConfig;

	BFilePanel*		fOpenPanel;

	BMessenger*		fMessenger;
	BMidiRoster*	fRoster;
	MidiConsumer*	fConsumer;
};

#endif /* MAINWINDOW_H */
