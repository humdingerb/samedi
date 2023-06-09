/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */

#include "Constants.h"
#include "Pad.h"

#include <Catalog.h>
#include <ControlLook.h>
#include <LayoutBuilder.h>

#include <stdio.h>
#include <stdlib.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Pad"

static const char* kNoSample = B_TRANSLATE_MARK("<click to load a sample>");
static const char* kSampleNotFound = B_TRANSLATE_MARK("⚠ - Failed loading '%samplefile%'");


Pad::Pad(int32 number, int32 note)
	:
	BView("pad", B_WILL_DRAW | B_SUPPORTS_LAYOUT),
	fPadNumber(number),
	fNote(note),
	fSamplePath(""),
	fPlayer(NULL)
{
	BString padNr;
	padNr << fPadNumber + 1;
	BString noteNr;
	noteNr << fNote;
	fNoteControl = new BTextControl("notecontrol", padNr, noteNr, new BMessage(NOTE));
	// only allow numbers
	for (uint32 i = 0; i < '0'; i++)
		fNoteControl->TextView()->DisallowChar(i);
	for (uint32 i = '9' + 1; i < 255; i++)
		fNoteControl->TextView()->DisallowChar(i);

	fDetectButton = new BButton("detect" , new BMessage(DETECT_NOTE));
	fDetectButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fDetectButton->SetToolTip(B_TRANSLATE("Detect MIDI note"));

	fMuteButton = new BButton("M", new BMessage(MUTE));
	fMuteButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fMuteButton->SetToolTip(B_TRANSLATE("Mute"));

	fSoloButton = new BButton("S" , new BMessage(SOLO));
	fSoloButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fSoloButton->SetToolTip(B_TRANSLATE("Solo"));

	fLoopButton = new BButton("∞" , new BMessage(LOOP));
	fLoopButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fLoopButton->SetToolTip(B_TRANSLATE("Loop"));

	fSampleButton = new BButton("sample", B_TRANSLATE_NOCOLLECT(kNoSample), new BMessage(OPEN_SAMPLE));
	fSampleButton->SetFlat(true);
	fSampleButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	fPlayButton = new BButton("⯈" , new BMessage(PLAY));
	fStopButton = new BButton("⏹" , new BMessage(STOP));
	fEjectButton = new BButton("⏏" , new BMessage(EJECT));

	// limit widget sizes
	float height;
	fNoteControl->GetPreferredSize(NULL, &height);
	BSize size(height + 2, height);
	fMuteButton->SetExplicitSize(size);
	fSoloButton->SetExplicitSize(size);
	fLoopButton->SetExplicitSize(size);
	fPlayButton->SetExplicitSize(size);
	fStopButton->SetExplicitSize(size);
	fEjectButton->SetExplicitSize(size);

	float width = height * 0.7;
	size = BSize(width, height);
	fDetectButton->SetExplicitSize(size);

	width = be_plain_font->StringWidth("XXXXX");
	size = BSize(width, height);
	fNoteControl->SetExplicitSize(size);

	fSampleButton->SetExplicitMinSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));

	const float kSpacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, kSpacing / 2, B_USE_WINDOW_SPACING, kSpacing / 2)
		.Add(fNoteControl)
		.Add(fDetectButton)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fMuteButton)
		.Add(fSoloButton)
		.Add(fLoopButton)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fSampleButton)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fPlayButton)
		.Add(fStopButton)
		.Add(fEjectButton)
	.End();

	SetEventMask(B_KEYBOARD_EVENTS);
}


Pad::~Pad()
{
	if (fPlayer != NULL) {
		fPlayer->StopPlaying();
		fPlayer = NULL;
		delete fPlayer;
	}
}


void
Pad::AttachedToWindow()
{
	fNoteControl->SetTarget(this);
	fDetectButton->SetTarget(this);
	fMuteButton->SetTarget(this);
	fSoloButton->SetTarget(this);
	fLoopButton->SetTarget(this);
	fSampleButton->SetTarget(this);
	fPlayButton->SetTarget(this);
	fStopButton->SetTarget(this);
	fEjectButton->SetTarget(this);
}


void
Pad::KeyDown(const char* bytes, int32 numBytes)
{
	if (Window()->IsActive() == false)
		return;

	if (bytes[0] == B_ESCAPE) {
		if (fDetectButton->Value() == B_CONTROL_ON)
			_SetDetectMode(false);
	} else {
		uint8 key = bytes[0];
		if (key == '0' + fPadNumber + 1)
			BMessenger(this).SendMessage(new BMessage(PLAY));
	}
	BView::KeyDown(bytes, numBytes);
}


void
Pad::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case NOTE:
		{
			BString newNote = fNoteControl->Text();
			if (newNote == "") {
				newNote << fNote;
				fNoteControl->SetText(newNote);
			} else
				fNote = atoi(newNote);

			fNoteControl->MakeFocus(false);

			if (fDetectButton->Value() == B_CONTROL_ON)
				_SetDetectMode(false);

			break;
		}
		case DETECT_NOTE:
		{
			if (fDetectButton->Value() == B_CONTROL_ON) {
				fNoteControl->SetText("?");
				_SetDetectMode(true);
			} else
				_SetDetectMode(false);

			break;
		}
		case MUTE:
		{
			Mute(fMuteButton->Value());
			break;
		}
		case SOLO:
		{
			msg->AddInt32("pad", fPadNumber);
			msg->AddInt32("solo", fSoloButton->Value());
			Window()->PostMessage(msg);
			break;
		}
		case LOOP:
		{
			SetSample(fSamplePath);
			break;
		}
		case OPEN_SAMPLE:
		{
			msg->AddInt32("pad", fPadNumber);
			Window()->PostMessage(msg);
			break;
		}
		case PLAY:
		{
			if ((fPlayer != NULL) && (fMuteButton->Value() == B_CONTROL_OFF))
				fPlayer->StartPlaying();
			break;
		}
		case STOP:
		{
			if (fPlayer != NULL)
				fPlayer->StopPlaying();
			break;
		}
		case EJECT:
		{
			_Eject();
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}


// #pragma mark -


void
Pad::Mute(int32 state)
{
	fMuteButton->SetValue(state);

	if (state == B_CONTROL_ON) {
		fSoloButton->SetValue(B_CONTROL_OFF); // in case this pad was in solo mode
		if (fPlayer != NULL)
			fPlayer->StopPlaying();
	}
}


void
Pad::Play(int32 note)
{
	if (fDetectButton->Value() == B_CONTROL_ON) {
		SetNote(note);
		return;
	}

	if (fMuteButton->Value() == B_CONTROL_ON)
		return;

	if (note != fNote)
		return;

	if (fPlayer != NULL)
		fPlayer->StartPlaying();
}


void
Pad::SetNote(int32 note)
{
	fNote = note;
	_SetDetectMode(false);
}


void
Pad::SetSample(BPath sample)
{
	if (sample.InitCheck() != B_OK) {
		_Eject();
		return;
	}

	fSamplePath = sample;
	if (fPlayer != NULL) {
		fPlayer->StopPlaying();
		fPlayer = NULL;
		delete fPlayer;
	}

	fPlayer = new BFileGameSound(fSamplePath.Path(), fLoopButton->Value() == B_CONTROL_ON);
	if (fPlayer->InitCheck() == B_OK) {
		fPlayer->Preload();
		fSampleButton->SetLabel(fSamplePath.Leaf());
	} else {
		BString label(B_TRANSLATE_NOCOLLECT(kSampleNotFound));
		label.ReplaceFirst("%samplefile%", fSamplePath.Leaf());
		fSampleButton->SetLabel(label);
	}
}


void
Pad::_Eject()
{
	fSampleButton->SetLabel(B_TRANSLATE_NOCOLLECT(kNoSample));
	fSamplePath = BPath("");
	if (fPlayer != NULL) {
		fPlayer->StopPlaying();
		fPlayer = NULL;
		delete fPlayer;
	}
}


void
Pad::_SetDetectMode(bool state)
{
	if (state == true) {
		fDetectButton->SetValue(B_CONTROL_ON);
		fNoteControl->SetToolTip(B_TRANSLATE("Press key"));
		fNoteControl->MarkAsInvalid(true);
	} else {
		fDetectButton->SetValue(B_CONTROL_OFF);
		BString text;
		text << fNote;
		fNoteControl->SetText(text);
		fNoteControl->SetToolTip("");
		fNoteControl->MakeFocus(false);
		fNoteControl->MarkAsInvalid(false);
	}
}
