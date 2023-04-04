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

#include <Messenger.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <Window.h>


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	void			MessageReceived(BMessage* msg);

	playerConfig*	fPlayerConfig;

private:
	void			_HandleMIDI(BMessage* msg);

	Pad*			fPads[kPadCount];

	BMessenger*		fMessenger;
	BMidiRoster*	fRoster;
	MidiConsumer*	fConsumer;
};

#endif /* MAINWINDOW_H */
