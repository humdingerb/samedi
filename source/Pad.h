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
#include <FileGameSound.h>
#include <Path.h>
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

	void			Play(int32 note);
	void			Mute(int32 state);
	void			SetSample(BPath sample);

private:
	int32			fPadNumber;
	int32			fNote;

	BButton*		fMuteButton;
	BButton*		fSoloButton;
	BButton*		fLoopButton;
	BButton*		fSampleButton;
	BButton*		fPlayButton;
	BButton*		fEjectButton;

	BTextControl*	fNoteControl;
	BFileGameSound*	fPlayer;
	BPath			fSamplePath;

};


#endif // PAD_H
