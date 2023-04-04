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


Pad::Pad(int32 number, uchar note)
	:
	BView("pad", B_WILL_DRAW | B_SUPPORTS_LAYOUT),
	fPadNumber(number),
	fNote(note)
{
	BString text;
	text << fPadNumber + 1;
	BStringView* pad = new BStringView("pad", text);

	BMessage* msg = new BMessage(NOTE);
	msg->AddInt32("pad", fPadNumber);
	text = "";
	text << fNote;
	fNoteControl = new BTextControl("notecontrol", NULL, text, msg);
	fNoteControl->SetToolTip(B_TRANSLATE("Midi note"));

	msg = new BMessage(MUTE);
	msg->AddInt32("pad", fPadNumber);
	fMute = new BButton("M", msg);
	fMute->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fMute->SetToolTip(B_TRANSLATE("Mute"));

	msg = new BMessage(SOLO);
	msg->AddInt32("pad", fPadNumber);
	fSolo = new BButton("S" , msg);
	fSolo->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fSolo->SetToolTip(B_TRANSLATE("Solo"));

	msg = new BMessage(LOOP);
	msg->AddInt32("pad", fPadNumber);
	fLoop = new BButton("∞" , msg);
	fLoop->SetBehavior(BButton::B_TOGGLE_BEHAVIOR);
	fLoop->SetToolTip(B_TRANSLATE("Loop"));

	fOpenSample = new BButton("O", new BMessage(OPENSAMPLE));

	fSampleName = new BStringView("samplename", B_TRANSLATE("<click to load a sample>"));

	msg = new BMessage(PLAYSTOP);
	msg->AddInt32("pad", fPadNumber);
	fPlayStop = new BButton("⯈" , msg);

	msg = new BMessage(EJECT);
	msg->AddInt32("pad", fPadNumber);
	fEject = new BButton("⏏" , msg);

	// limit button sizes
	float height;
	fNoteControl->GetPreferredSize(NULL, &height);
	BSize size(height, height);
	fMute->SetExplicitSize(size);
	fSolo->SetExplicitSize(size);
	fLoop->SetExplicitSize(size);
	fOpenSample->SetExplicitSize(size);
	fPlayStop->SetExplicitSize(size);
	fEject->SetExplicitSize(size);

	float width = be_plain_font->StringWidth("XXX");
	size = BSize(width, height);
	fNoteControl->SetExplicitSize(size);
//	fNoteControl->TextView()->SetExplicitSize(size);

	BLayoutBuilder::Group<>(this, B_HORIZONTAL, 0)
		.SetInsets(B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING, B_USE_WINDOW_SPACING)
		.Add(pad)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fNoteControl)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fMute)
		.Add(fSolo)
		.Add(fLoop)
//		.Add(fOpenSample)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fSampleName)
		.AddStrut(B_USE_SMALL_SPACING)
		.Add(fPlayStop)
		.Add(fEject)
	.End();
}


Pad::~Pad()
{
}


void
Pad::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case PLAYSAMPLE:
		{
			msg->PrintToStream();
			break;
		}
		default:
		{
			BView::MessageReceived(msg);
			break;
		}
	}
}

