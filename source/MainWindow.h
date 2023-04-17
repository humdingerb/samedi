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
#include <Menu.h>
#include <Messenger.h>
#include <MidiProducer.h>
#include <MidiRoster.h>
#include <StringList.h>
#include <Window.h>


class MainWindow : public BWindow {
public:
					MainWindow();
	virtual			~MainWindow();

	virtual void	MenusBeginning();
	virtual void	MessageReceived(BMessage* msg);

private:
	BMenuBar*		_BuildMenu();
	BView*			_BuildPadViews();
	BView*			_BuildHeaderView();

	void			_PopulateOpenRecentMenu();
	void			_PopulateMidiInMenu();
	void			_HandleMIDI(BMessage* msg);

	void			_LoadSettings();
	void			_SaveSettings();

	void			_OpenHelp();
	void			_LoadEnsemble(entry_ref ref);
	void			_SaveEnsemble();
	void			_AddRecentEnsemble(BString path);

	void			_UpdateWindowTitle();

	void			_SendSample(BMessage* msg, entry_ref ref);
	void			_SetSample(int32 pad, BString samplepath);
	void			_SetNote(int32 pad, int32 note);

	Pad*			fPads[kPadCount];

	BFilePanel*		fOpenSamplePanel;
	BFilePanel*		fOpenEnsemblePanel;
	BFilePanel*		fSaveEnsemblePanel;

	BPath			fEnsemblePath;
	BStringList		fRecentEnsemblePaths;

	BMenu*			fOpenRecentMenu;
	BMenu*			fMidiInMenu;
	BMenuItem*		fSaveMenu;

	BMessage*		fSettings;
	BMessenger*		fMessenger;
	BMidiRoster*	fRoster;
	MidiConsumer*	fConsumer;
};

#endif /* MAINWINDOW_H */
