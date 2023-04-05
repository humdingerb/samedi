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
#include <LayoutBuilder.h>

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "Pad"

static const char* kNoSample = B_TRANSLATE_MARK("<click to load a sample>");


Pad::Pad(int32 number, uchar note)
	:
	BView("pad", B_WILL_DRAW | B_SUPPORTS_LAYOUT),
	fPadNumber(number),
	fNote(note)
{
	BString text;
	text << fPadNumber + 1;
	BStringView* pad = new BStringView("pad", text);

	text = "";
	text << fNote;
	fNoteControl = new BTextControl("notecontrol", NULL, text, new BMessage(NOTE));
	fNoteControl->SetToolTip(B_TRANSLATE("Midi note"));

	fMute = new BButton("M", new BMessage(MUTE));
	fMute->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fMute->SetToolTip(B_TRANSLATE("Mute"));

	fSolo = new BButton("S" , new BMessage(SOLO));
	fSolo->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fSolo->SetToolTip(B_TRANSLATE("Solo"));

	fLoop = new BButton("∞" , new BMessage(LOOP));
	fLoop->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fLoop->SetToolTip(B_TRANSLATE("Loop"));

	fSampleName = new BStringView("samplename", kNoSample);

	fPlay = new BButton("⯈" , new BMessage(PLAY));
	fEject = new BButton("⏏" , new BMessage(EJECT));

	// limit button sizes
	float height;
	fNoteControl->GetPreferredSize(NULL, &height);
	BSize size(height, height);
	fMute->SetExplicitSize(size);
	fSolo->SetExplicitSize(size);
	fLoop->SetExplicitSize(size);
	fPlay->SetExplicitSize(size);
	fEject->SetExplicitSize(size);

	float width = be_plain_font->StringWidth("XXX");
	size = BSize(width, height);
	fNoteControl->SetExplicitSize(size);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(pad)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fNoteControl)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fMute)
		.Add(fSolo)
		.Add(fLoop)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fSampleName)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fPlay)
		.Add(fEject)
	.End();
}


Pad::~Pad()
{
}


void
Pad::MessageReceived(BMessage* msg)
{
	msg->AddInt32("pad", fPadNumber);

	switch (msg->what) {
		case NOTE:
		{
			int32 note = atoi(fNoteControl->Text());
			msg->AddInt32("note", note);
			Window()->PostMessage(msg);
			break;
		}
		case MUTE:
		{
			int32 on_off = fMute->Value();
			msg->AddInt32("mute", on_off);
			Window()->PostMessage(msg);
			break;
		}
		case SOLO:
		{
			int32 on_off = fSolo->Value();
			msg->AddInt32("solo", on_off);
			Window()->PostMessage(msg);
			break;
		}
		case LOOP:
		{
			int32 on_off = fLoop->Value();
			msg->AddInt32("loop", on_off);
			Window()->PostMessage(msg);
			break;
		}
		case PLAY:
		{
			msg->AddInt32("note", fNote);
			Window()->PostMessage(msg);
			break;
		}
		case EJECT:
		{
			fSampleName->SetText(kNoSample);
			Window()->PostMessage(msg);
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}
