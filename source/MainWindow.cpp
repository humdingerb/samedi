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
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Path.h>
#include <Screen.h>
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
	fOpenPanel(new BFilePanel(B_OPEN_PANEL)),
	fSavePanel(new BFilePanel(B_SAVE_PANEL)),
	fSettings(NULL)
{
	_LoadSettings();

	BRect frame;
	if (fSettings->FindRect("main window frame", &frame) == B_OK) {
		BScreen screen(this);
		if (frame.Intersects(screen.Frame())) {
			MoveTo(frame.LeftTop());
			ResizeTo(frame.Width(), frame.Height());
		}
	}

	// init pads
	for (int32 i = 0; i < kPadCount; i++)
		fPads[i] = new Pad(i, kDefaultNote + i);

	// building menu
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu = new BMenu(B_TRANSLATE("File"));

	BMenuItem* menuItem = new BMenuItem(B_TRANSLATE("Open ensemble" B_UTF8_ELLIPSIS),
		new BMessage(OPEN_ENSEMBLE), 'O');
	menu->AddItem(menuItem);

	fSaveMenu = new BMenuItem(B_TRANSLATE("Save"), new BMessage(SAVE_ENSEMBLE), 'S');
	fSaveMenu->SetEnabled(false);
	menu->AddItem(fSaveMenu);

	menuItem = new BMenuItem(B_TRANSLATE("Save ensemble" B_UTF8_ELLIPSIS),
		new BMessage(SAVE_AS_ENSEMBLE), 'S', B_SHIFT_KEY);
	menu->AddItem(menuItem);

	menu->AddSeparatorItem();
	menuItem = new BMenuItem(B_TRANSLATE("About Samedi"), new BMessage(B_ABOUT_REQUESTED));
	menuItem->SetTarget(be_app);
	menu->AddItem(menuItem);

	menuItem = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(menuItem);
	menuBar->AddItem(menu);

	// building layout
	BView* padView = new BView("padView", 0);
	BLayoutBuilder::Group<>(padView, B_VERTICAL, 0)
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

	padView->SetExplicitMinSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(padView)
		.End();

	fMessenger = new BMessenger(this, NULL);
	fConsumer = new MidiConsumer(fMessenger);
	fRoster = BMidiRoster::MidiRoster();
	fRoster->StartWatching(fMessenger);
}


MainWindow::~MainWindow()
{
	delete fOpenPanel;
	delete fSavePanel;
	delete fMessenger;
	fConsumer->Release();

	_SaveSettings();
}


// #pragma mark -


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) == B_OK)
				_LoadEnsemble(ref);
			break;
		}
		case B_MIDI_EVENT:
		{
			_HandleMIDI(msg);
			break;
		}
		case OPEN_ENSEMBLE:
		{
			BMessage* openMsg = new BMessage(B_REFS_RECEIVED);
			fOpenPanel->SetTarget(this);
			fOpenPanel->SetMessage(openMsg);
			fOpenPanel->Show();
			break;
		}
		case SAVE_ENSEMBLE:
		{
			_SaveEnsemble();
			break;
		}
		case SAVE_AS_ENSEMBLE:
		{
			BMessenger messenger(this);
			fSavePanel->SetTarget(this);
			fSavePanel->Show();
			break;
		}
		case B_SAVE_REQUESTED:
		{
			entry_ref ref;
			const char* name;
			if (msg->FindRef("directory", &ref) == B_OK
				&& msg->FindString("name", &name) == B_OK) {
				BDirectory directory(&ref);
				BEntry entry(&directory, name);
				fEnsemblePath = BPath(&entry);
				_SaveEnsemble();
			}
			break;
		}
		case SOLO:
		{
			int32 soloPad;
			int32 state;
			if ((msg->FindInt32("pad", &soloPad) == B_OK) and
				(msg->FindInt32("solo", &state) == B_OK)) {

				if (state == B_CONTROL_ON) {
					for (int32 i = 0; i < kPadCount; i++) {
						// mute all other pads
						if (i != soloPad)
							fPads[i]->Mute(B_CONTROL_ON);
						// unmute the solo pad
						else
							fPads[i]->Mute(B_CONTROL_OFF);
					}
				} else {
					for (int32 i = 0; i < kPadCount; i++)
						fPads[i]->Mute(B_CONTROL_OFF);
				}
			}
			break;
		}
		case PLAY:
		{
			int32 note;
			if (msg->FindInt32("note", &note) == B_OK) {
				for (int32 i = 0; i < kPadCount; i++)
					fPads[i]->Play(note);
			}
			break;
		}
		case OPEN_SAMPLE:
		{
			int32 pad;
			if (msg->FindInt32("pad", &pad) == B_OK) {
				BMessage* openMsg = new BMessage(LOAD_SAMPLE);
				openMsg->AddInt32("pad", pad);
				fOpenPanel->SetTarget(this);
				fOpenPanel->SetMessage(openMsg);
				fOpenPanel->Show();
			}
			break;
		}
		case LOAD_SAMPLE:
		{
			entry_ref ref;
			int32 pad;
			if ((msg->FindRef("refs", &ref) == B_OK) and
				(msg->FindInt32("pad", &pad) == B_OK)) {
				BPath path(&ref);
				_SetSample(pad, path.Path());
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
MainWindow::_LoadSettings()
{
	fSettings = new BMessage();

	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) != B_OK)
		return;

	path.Append("Samedi_settings");
	BFile file(path.Path(), B_READ_ONLY);

	if ((file.InitCheck() == B_OK) and (fSettings->Unflatten(&file) != B_OK))
		fSettings->AddRect("main window frame", BRect(200, 200, 600, 300));
}


void
MainWindow::_SaveSettings()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;

	BMessage settings;
	settings.AddRect("main window frame", Frame());

	path.Append("Samedi_settings");
	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK)
		settings.Flatten(&file);
}


void
MainWindow::_LoadEnsemble(entry_ref ref)
{
	BFile file(&ref, B_READ_ONLY);
	BMessage ensemble;

	if (file.InitCheck() == B_OK && ensemble.Unflatten(&file) == B_OK) {
		int32 note = kDefaultNote;
		BString samplepath = "";
		for (int32 i = 0; i < kPadCount; i++) {
			ensemble.FindInt32("note", i, &note);
			_SetNote(i, note++);
			ensemble.FindString("sample", i, &samplepath);
			_SetSample(i, samplepath);
		}
	fSaveMenu->SetEnabled(true);
	fEnsemblePath = BPath(&ref);
	}
}


void
MainWindow::_SaveEnsemble()
{
	BMessage ensemble;

	for (int32 i = 0; i < kPadCount; i++) {
		ensemble.AddInt32("note", fPads[i]->GetNote());
		ensemble.AddString("sample", fPads[i]->GetSamplePath());
	}

	BFile file(fEnsemblePath.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK) {
		ensemble.Flatten(&file);
		fSaveMenu->SetEnabled(true);
	}
}


void
MainWindow::_SetSample(int32 pad, BString samplepath)
{
	BPath path(samplepath.String());
	if (path.InitCheck() == B_OK)
		fPads[pad]->SetSample(path);
}


void
MainWindow::_SetNote(int32 pad, int32 note)
{
	fPads[pad]->SetNote(note);
}
