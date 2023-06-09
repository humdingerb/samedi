/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#include "MainWindow.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <File.h>
#include <FindDirectory.h>
#include <LayoutBuilder.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <Node.h>
#include <NodeInfo.h>
#include <Path.h>
#include <PathFinder.h>
#include <Roster.h>
#include <Screen.h>
#include <ScrollView.h>
#include <SeparatorView.h>
#include <StringView.h>
#include <Volume.h>

#include <compat/sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "MainWindow"


class AudioFilter : public BRefFilter {
public:
	bool	Filter(const entry_ref* entryRef, BNode* node,
				struct stat_beos* stat, const char* fileType);
};


bool
AudioFilter::Filter(const entry_ref* ref, BNode* node, struct stat_beos* stat,
	const char* fileType)
{
	BEntry entry(ref, true); // traverse links

	// allow folders and links of folders
	if (entry.IsDirectory())
		return true;

	// allow audio and linked audio files
	char mimeType[B_MIME_TYPE_LENGTH];
	BNode traversedNode(&entry); // create a new node from the link-traversed BEntry
	BNodeInfo(&traversedNode).GetType(mimeType);
	if (strncmp("audio/", mimeType, 6) == 0)
		return true;

	// allow all, if volume doesn't know MIME
	BVolume volume;
	volume.SetTo((ref->device));
	if (volume.KnowsMime() == false)
		return true;

	return false;
}


MainWindow::MainWindow()
	:
	BWindow(BRect(200, 200, 600, 300), B_TRANSLATE_SYSTEM_NAME("Samedi"), B_TITLED_WINDOW,
		B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS | B_QUIT_ON_WINDOW_CLOSE
			| B_AUTO_UPDATE_SIZE_LIMITS),
	fRecentEnsemblePaths(10),
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

	// init file panels
	BMessenger messenger(this);
	entry_ref ref;

	fSettings->FindRef("last sample folder", &ref);
	fOpenSamplePanel = new BFilePanel(B_OPEN_PANEL, &messenger, &ref, B_FILE_NODE, false,
		NULL, new AudioFilter());

	fSettings->FindRef("last ensemble folder", &ref);
	BMessage message(B_REFS_RECEIVED);
	fOpenEnsemblePanel = new BFilePanel(B_OPEN_PANEL, &messenger, &ref, B_FILE_NODE, false,
		&message);
	fOpenEnsemblePanel->Window()->SetTitle(B_TRANSLATE("Samedi: Open ensemble"));

	fSettings->FindRef("last ensemble save folder", &ref);
	fSaveEnsemblePanel = new BFilePanel(B_SAVE_PANEL, &messenger, &ref, B_FILE_NODE, false);
	fSaveEnsemblePanel->Window()->SetTitle(B_TRANSLATE("Samedi: Save ensemble"));

	// init pads
	for (int32 i = 0; i < kPadCount; i++)
		fPads[i] = new Pad(i, kDefaultNote + i);

	// build layouts
	BMenuBar* menuBar = _BuildMenu();
	BView* padView = _BuildPadViews();
	BView* headerView = _BuildHeaderView();

	BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
		.Add(menuBar)
		.Add(headerView)
		.Add(new BSeparatorView(B_HORIZONTAL))
		.Add(padView)
		.End();

	// create MidiConsumer
	fMessenger = new BMessenger(this, NULL);
	fConsumer = new MidiConsumer(fMessenger);
	fRoster = BMidiRoster::MidiRoster();
	fRoster->StartWatching(fMessenger);

	// Auto-connect, to first MIDI producer found
	int32 id = 0;
	BMidiProducer* producer = NULL;
	if ((producer = fRoster->NextProducer(&id)) != NULL) {
		if (producer->IsValid())
			producer->Connect(fConsumer);
	}
}


MainWindow::~MainWindow()
{
	_SaveSettings();

	fConsumer->Release();
	delete fOpenSamplePanel;
	delete fOpenEnsemblePanel;
	delete fSaveEnsemblePanel;
	delete fMessenger;
}


// #pragma mark -


void
MainWindow::MenusBeginning()
{
	_PopulateMidiInMenu();
	_PopulateOpenRecentMenu();
}


void
MainWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case B_SIMPLE_DATA:
		case B_REFS_RECEIVED:
		{
			entry_ref ref;
			if (msg->FindRef("refs", &ref) != B_OK)
				break;

			BEntry entry(&ref, true); // traverse links
			BNode node(&entry);
			char mimeType[B_MIME_TYPE_LENGTH];
			BNodeInfo(&node).GetType(mimeType);
			if (strncmp("audio/", mimeType, 6) == 0)
				_SendSample(msg, ref);
			else
				_LoadEnsemble(ref);
			break;
		}
		case B_MIDI_EVENT:
		{
			_HandleMIDI(msg);
			break;
		}

		case HELP:
		{
			_OpenHelp();
			break;
		}
		case OPEN_ENSEMBLE:
		{
			fOpenEnsemblePanel->Show();
			break;
		}
		case OPEN_RECENT:
		{
			int32 menu;
			BString filepath;
			if ((msg->FindInt32("menu", &menu) == B_OK) &&
				(msg->FindString("filepath", &filepath) == B_OK)) {
				entry_ref ref;
				get_ref_for_path(filepath.String(), &ref);
				_LoadEnsemble(ref);
			}
			break;
		}
		case SAVE_ENSEMBLE:
		{
			_SaveEnsemble();
			break;
		}
		case SAVE_AS_ENSEMBLE:
		{
			fSaveEnsemblePanel->Show();
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
		case CLEARALL:
		{
			int32 note = kDefaultNote;
			BString samplepath = "";
			for (int32 i = 0; i < kPadCount; i++) {
				_SetNote(i, note++);
				_SetSample(i, samplepath);
			}
			break;
		}
		case MIDI_IN_MENU:
		{
			int32 id = msg->FindInt32("port_id");
			BMidiProducer* producer = fRoster->FindProducer(id);
			if (producer) {
				if (producer->IsConnected(fConsumer))
					producer->Disconnect(fConsumer);
				else
					producer->Connect(fConsumer);
			}
			break;
		}
		case SOLO:
		{
			int32 soloPad;
			int32 state;
			if ((msg->FindInt32("pad", &soloPad) == B_OK) &&
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
				fOpenSamplePanel->SetMessage(openMsg);

				BString title(B_TRANSLATE("Samedi: Open sample"));
				title << " #" << pad + 1;
				fOpenSamplePanel->Window()->SetTitle(title);
				fOpenSamplePanel->Show();
			}
			break;
		}
		case LOAD_SAMPLE:
		{
			entry_ref ref;
			int32 pad;
			if ((msg->FindRef("refs", &ref) == B_OK) &&
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


// #pragma mark -


BMenuBar*
MainWindow::_BuildMenu()
{
	BMenuBar* menuBar = new BMenuBar("menubar");
	BMenu* menu;
	BMenuItem* item;

	// menu Samedi
	menu = new BMenu(B_TRANSLATE_SYSTEM_NAME("Samedi"));
	item = new BMenuItem(B_TRANSLATE("Help" B_UTF8_ELLIPSIS), new BMessage(HELP), 'H');
	menu->AddItem(item);

	item = new BMenuItem(B_TRANSLATE("About Samedi"), new BMessage(B_ABOUT_REQUESTED));
	item->SetTarget(be_app);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem(B_TRANSLATE("Quit"), new BMessage(B_QUIT_REQUESTED), 'Q');
	menu->AddItem(item);
	menuBar->AddItem(menu);

	// menu File
	menu = new BMenu(B_TRANSLATE("Ensemble"));
	fOpenRecentMenu = new BMenu(B_TRANSLATE("Open recent"));
	item = new BMenuItem(fOpenRecentMenu);
	menu->AddItem(item);

	item = new BMenuItem(B_TRANSLATE("Open" B_UTF8_ELLIPSIS),
		new BMessage(OPEN_ENSEMBLE), 'O');
	menu->AddItem(item);

	fSaveMenu = new BMenuItem(B_TRANSLATE("Save"), new BMessage(SAVE_ENSEMBLE), 'S');
	fSaveMenu->SetEnabled(false);
	menu->AddItem(fSaveMenu);

	item = new BMenuItem(B_TRANSLATE("Save as" B_UTF8_ELLIPSIS),
		new BMessage(SAVE_AS_ENSEMBLE), 'S', B_SHIFT_KEY);
	menu->AddItem(item);

	menu->AddSeparatorItem();

	item = new BMenuItem(B_TRANSLATE("Clear all pads"),	new BMessage(CLEARALL), 'D', B_SHIFT_KEY);
	menu->AddItem(item);
	menuBar->AddItem(menu);

	// menu Midi in
	fMidiInMenu = new BMenu(B_TRANSLATE("MIDI in"));
	menuBar->AddItem(fMidiInMenu);

	return menuBar;
}

BView*
MainWindow::_BuildPadViews()
{
	// building layout padView
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

	return padView;
}


BView*
MainWindow::_BuildHeaderView()
{
	BStringView* pad = new BStringView("", B_TRANSLATE_COMMENT("MIDI note",
		"Header, as short as the English term if possible"));
	BStringView* modes = new BStringView("", B_TRANSLATE_COMMENT("Modes",
		"Header, as short as the English term if possible"));
	BStringView* sample = new BStringView("", B_TRANSLATE("Sample file"));
	sample->SetAlignment(B_ALIGN_CENTER);

	BStringView* dummy = new BStringView("", "");

	BFont font(be_plain_font);
	font.SetSize(ceilf(font.Size() * 0.8));
	pad->SetFont(&font, B_FONT_SIZE);
	modes->SetFont(&font, B_FONT_SIZE);
	sample->SetFont(&font, B_FONT_SIZE);
	dummy->SetFont(&font, B_FONT_SIZE);

	float width, height;
	fPads[0]->FindView("notecontrol")->GetPreferredSize(&width, &height);
	pad->SetExplicitSize(BSize(width, B_SIZE_UNSET));
	modes->SetExplicitSize(BSize(height * 3, B_SIZE_UNSET));
	fPads[0]->FindView("sample")->GetPreferredSize(&width, &height);
	sample->SetExplicitSize(BSize(width, B_SIZE_UNSET));
	dummy->SetExplicitSize(BSize(height * 3, B_SIZE_UNSET));

	const float kSpacing = be_control_look->DefaultItemSpacing();
	BView* headerView = new BView("headerView", B_SUPPORTS_LAYOUT);
	BLayoutBuilder::Group<>(headerView, B_HORIZONTAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, kSpacing / 2, B_USE_WINDOW_SPACING, kSpacing / 2)
		.Add(pad)
		.Add(modes)
		.AddGlue()
		.Add(sample)
		.Add(dummy)
		.AddGlue()
	.End();

	headerView->SetExplicitMinSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));

	rgb_color color = tint_color(ui_color(B_PANEL_BACKGROUND_COLOR), B_DARKEN_1_TINT);
	headerView->SetViewColor(color);

	return headerView;
}

void
MainWindow::_PopulateMidiInMenu()
{
	fMidiInMenu->RemoveItems(0, fMidiInMenu->CountItems(), true);

	int32 id = 0;
	BMidiProducer* producer = NULL;

	while ((producer = fRoster->NextProducer(&id)) != NULL) {
		if (producer->IsValid()) {
			BMessage* msg = new BMessage(MIDI_IN_MENU);
			msg->AddInt32("port_id", id);
			BMenuItem* item = new BMenuItem(producer->Name(), msg);
			fMidiInMenu->AddItem(item);
			if (producer->IsConnected(fConsumer))
				item->SetMarked(true);
			else
				item->SetMarked(false);
		}
	}

	// Auto-connect, if only one MIDI producer was found
	if (fMidiInMenu->CountItems() == 1) {
		int32 id = 0;
		BMidiProducer* producer = NULL;
		if ((producer = fRoster->NextProducer(&id)) != NULL) {
			if (producer->IsValid()) {
				producer->Connect(fConsumer);
				fMidiInMenu->ItemAt(0)->SetMarked(true);
			}
		}
	}

}


void
MainWindow::_PopulateOpenRecentMenu()
{
	fOpenRecentMenu->RemoveItems(0, fOpenRecentMenu->CountItems(), true);

	int32 menu = 0;
	BString filepath;
	for (int32 i = fRecentEnsemblePaths.CountStrings() - 1; i >= 0; i--) {
		filepath = fRecentEnsemblePaths.StringAt(i);
		BMessage* msg = new BMessage(OPEN_RECENT);
		msg->AddInt32("menu", menu++);
		msg->AddString("filepath", filepath);
		BPath path(filepath.String());
		BMenuItem* item = new BMenuItem(path.Leaf(), msg);
		fOpenRecentMenu->AddItem(item);

		BEntry entry(filepath.String());
		if (entry.Exists() == false)
			item->SetEnabled(false);
	}
}


void
MainWindow::_HandleMIDI(BMessage* msg)
{
	int32 op, id, producerid;
	msg->FindInt32("be:op", &op);
	msg->FindInt32("be:id", &id);
	msg->FindInt32("be:producer", &producerid);
	// printf("op: %i, id: %i, producerid: %i\n", op, id, producerid);
	switch (op) {
		case B_MIDI_REGISTERED:
		{
			int32 id = 0;
			BMidiProducer* producer;
			if ((producer = fRoster->NextProducer(&id)) != NULL) {
				printf("Found producer %" B_PRId32 ": %s\n", id, producer->Name());
				producer->Connect(fConsumer);
			}
			break;
		}
		case B_MIDI_UNREGISTERED:
		case B_MIDI_DISCONNECTED:
		{
			_PopulateMidiInMenu();
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

	if (file.InitCheck() != B_OK)
		be_app->PostMessage(new BMessage(FIRST_LAUNCH));

	if (fSettings->Unflatten(&file) != B_OK)
		fSettings->AddRect("main window frame", BRect(200, 200, 600, 300));
	else
		fSettings->FindStrings("recent ensemble", &fRecentEnsemblePaths);
}


void
MainWindow::_SaveSettings()
{
	BPath path;
	if (find_directory(B_USER_SETTINGS_DIRECTORY, &path) < B_OK)
		return;

	BMessage settings;
	settings.AddRect("main window frame", Frame());

	for (int32 i = 0; i < fRecentEnsemblePaths.CountStrings(); i++)
		settings.AddString("recent ensemble", fRecentEnsemblePaths.StringAt(i));

	entry_ref ref;
	fOpenSamplePanel->GetPanelDirectory(&ref);
	settings.AddRef("last sample folder", &ref);

	fOpenEnsemblePanel->GetPanelDirectory(&ref);
	settings.AddRef("last ensemble folder", &ref);

	fSaveEnsemblePanel->GetPanelDirectory(&ref);
	settings.AddRef("last ensemble save folder", &ref);

	path.Append("Samedi_settings");
	BFile file(path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() == B_OK)
		settings.Flatten(&file);
}


void
MainWindow::_OpenHelp()
{
	BPathFinder pathFinder;
	BStringList paths;
	BPath path;
	BEntry entry;

	status_t error = pathFinder.FindPaths(B_FIND_PATH_DOCUMENTATION_DIRECTORY,
		"packages/samedi", paths);

	for (int i = 0; i < paths.CountStrings(); ++i) {
		if (error == B_OK && path.SetTo(paths.StringAt(i)) == B_OK
			&& path.Append("ReadMe.html") == B_OK) {
			entry = path.Path();
			if (!entry.Exists())
				continue;
			entry_ref ref;
			entry.GetRef(&ref);
			be_roster->Launch(&ref);
		}
	}
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
			ensemble.FindString("sample", i, &samplepath);
			_SetNote(i, note++);
			_SetSample(i, samplepath);
		}
		fSaveMenu->SetEnabled(true);
		BPath path(&ref);
		fEnsemblePath = path;
		_AddRecentEnsemble(path.Path());
		_UpdateWindowTitle();
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
		_AddRecentEnsemble(fEnsemblePath.Path());
		_UpdateWindowTitle();
	}
}


void
MainWindow::_AddRecentEnsemble(BString path)
{
	int32 count = fRecentEnsemblePaths.CountStrings();
	// remove oldest item, if we exceed max number
	if (count == kMaxRecentEnsembles)
		fRecentEnsemblePaths.Remove(0);

	int32 index = fRecentEnsemblePaths.IndexOf(path);
	// add new item to the end, or move existing item to the end
	if (index < 0)
		fRecentEnsemblePaths.Add(path);
	else
		fRecentEnsemblePaths.Move(index, count - 1);
}


void
MainWindow::_UpdateWindowTitle()
{
	BString title(B_TRANSLATE_SYSTEM_NAME("Samedi"));
	if (fEnsemblePath != NULL)
		title << " : " << fEnsemblePath.Leaf();

	SetTitle(title);
}


void
MainWindow::_SendSample(BMessage* msg, entry_ref ref)
{
	BPath path(&ref);

	BPoint point;
	msg->FindPoint("_drop_point_", &point);

	for (int32 i = 0; i < kPadCount; i++) {
		BRect rect = fPads[i]->ConvertToScreen(fPads[i]->Bounds());
		if (rect.Contains(point)) {
			fPads[i]->SetSample(path.Path());
			return;
		}
	}
}


void
MainWindow::_SetSample(int32 pad, BString samplepath)
{
	BPath path(samplepath.String());
	fPads[pad]->SetSample(path);
}


void
MainWindow::_SetNote(int32 pad, int32 note)
{
	fPads[pad]->SetNote(note);
}
