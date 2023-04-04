/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 *
 */
#ifndef PAD_H
#define PAD_H


#include <Button.h>
#include <StringView.h>
#include <SupportDefs.h>
#include <TextControl.h>
#include <View.h>


class Pad : public BView {
public:
					Pad(int32 number, uchar note);
	virtual			~Pad();

	void			MessageReceived(BMessage* msg);

private:
	int32			fPadNumber;
	uchar			fNote;

	BButton*		fMute;
	BButton*		fSolo;
	BButton*		fLoop;
	BButton*		fOpenSample;
	BButton*		fPlay;
	BButton*		fEject;

	BStringView*	fSampleName;
	BTextControl*	fNoteControl;
};


#endif // PAD_H
