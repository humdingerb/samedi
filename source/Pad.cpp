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

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Pad"

static const char* kNoSample = B_TRANSLATE_MARK("<click to load a sample>");
static const char* kSampleNotFound = B_TRANSLATE_MARK("☛ Failed loading sample ☚");


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
	fNoteControl->SetToolTip(B_TRANSLATE("Midi note"));

	fMuteButton = new BButton("M", NULL);
	fMuteButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fMuteButton->SetToolTip(B_TRANSLATE("Mute"));

	fSoloButton = new BButton("S" , new BMessage(SOLO));
	fSoloButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fSoloButton->SetToolTip(B_TRANSLATE("Solo"));

	fLoopButton = new BButton("∞" , new BMessage(LOOP));
	fLoopButton->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fLoopButton->SetToolTip(B_TRANSLATE("Loop"));

	fSampleButton = new BButton("sample", kNoSample, new BMessage(OPEN_SAMPLE));
	fSampleButton->SetFlat(true);
	fSampleButton->SetExplicitMaxSize(BSize(B_SIZE_UNLIMITED, B_SIZE_UNLIMITED));

	fPlayButton = new BButton("⯈" , new BMessage(PLAY));
	fEjectButton = new BButton("⏏" , new BMessage(EJECT));

	// limit widget sizes
	float height;
	fNoteControl->GetPreferredSize(NULL, &height);
	BSize size(height, height);
	fMuteButton->SetExplicitSize(size);
	fSoloButton->SetExplicitSize(size);
	fLoopButton->SetExplicitSize(size);
	fPlayButton->SetExplicitSize(size);
	fEjectButton->SetExplicitSize(size);

	float width = be_plain_font->StringWidth("XXXXX");
	size = BSize(width, height);
	fNoteControl->SetExplicitSize(size);

	fSampleButton->SetExplicitMinSize(BSize(B_SIZE_UNSET, B_SIZE_UNSET));

	const float kSpacing = be_control_look->DefaultItemSpacing();

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, kSpacing / 2, B_USE_WINDOW_SPACING, kSpacing / 2)
		.Add(fNoteControl)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fMuteButton)
		.Add(fSoloButton)
		.Add(fLoopButton)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fSampleButton)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fPlayButton)
		.Add(fEjectButton)
	.End();
}


void
Pad::AttachedToWindow()
{
	fNoteControl->SetTarget(this);
	fMuteButton->SetTarget(this);
	fSoloButton->SetTarget(this);
	fLoopButton->SetTarget(this);
	fSampleButton->SetTarget(this);
	fPlayButton->SetTarget(this);
	fEjectButton->SetTarget(this);
}


void
Pad::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case NOTE:
		{
			fNote = atoi(fNoteControl->Text());
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
			if (fMuteButton->Value() == B_CONTROL_ON)
				break;
			if (fPlayer != NULL)
				fPlayer->StartPlaying();
			break;
		}
		case EJECT:
		{
			fSampleButton->SetLabel(kNoSample);
			fSamplePath = BPath("");
			fPlayer = NULL;
			delete fPlayer;
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

	// in case this pad was in solo mode
	if (state == B_CONTROL_ON)
		fSoloButton->SetValue(B_CONTROL_OFF);
}


void
Pad::Play(int32 note)
{
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
	BString text;
	text << note;
	fNoteControl->SetText(text);
}


void
Pad::SetSample(BPath sample)
{
	if (sample.InitCheck() != B_OK)
		return;

	fSamplePath = sample;
	if (fPlayer != NULL)
		delete fPlayer;

	fPlayer = new BFileGameSound(fSamplePath.Path(), fLoopButton->Value() == B_CONTROL_ON);
	if (fPlayer->InitCheck() == B_OK) {
		fPlayer->Preload();
		fSampleButton->SetLabel(fSamplePath.Leaf());
	} else
		fSampleButton->SetLabel(kSampleNotFound);
}
