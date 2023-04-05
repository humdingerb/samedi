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
					Pad(int32 number, int32 note);
	virtual			~Pad() {};

	virtual	void	AttachedToWindow();
	void			MessageReceived(BMessage* msg);

	void			Mute(int32 state);
	void			SetSampleName(const char* sample);

private:
	int32			fPadNumber;
	int32			fNote;

	BButton*		fMute;
	BButton*		fSolo;
	BButton*		fLoop;
	BButton*		fSample;
	BButton*		fPlay;
	BButton*		fEject;

	BTextControl*	fNoteControl;
};


#endif // PAD_H
