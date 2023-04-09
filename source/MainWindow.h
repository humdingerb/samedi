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

#include <FilePanel.h>
#include <Messenger.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <Window.h>


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	void			MessageReceived(BMessage* msg);

private:
	void			_HandleMIDI(BMessage* msg);

	void			_LoadSettings();
	void			_SaveSettings();

	void			_LoadEnsemble(entry_ref ref);
	void			_SaveEnsemble();
	void			_UpdateWindowTitle();

	void			_SetSample(int32 pad, BString samplepath);
	void			_SetNote(int32 pad, int32 note);

	Pad*			fPads[kPadCount];

	BFilePanel*		fOpenPanel;
	BFilePanel*		fSavePanel;

	BPath			fEnsemblePath;

	BMenuItem*		fSaveMenu;

	BMessage*		fSettings;
	BMessenger*		fMessenger;
	BMidiRoster*	fRoster;
	MidiConsumer*	fConsumer;
};

#endif /* MAINWINDOW_H */
