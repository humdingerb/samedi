/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <AboutWindow.h>
#include <Catalog.h>

#include "App.h"
#include "MainWindow.h"

#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "App"

const char* kApplicationSignature = "application/x-vnd.humdinger-Samedi";


App::App()
	:
	BApplication(kApplicationSignature)
{
	fMainWindow = new MainWindow();
	fMainWindow->Show();
}


App::~App()
{
}


void
App::AboutRequested()
{
	BAboutWindow* aboutW
		= new BAboutWindow(B_TRANSLATE_SYSTEM_NAME("Samedi"), kApplicationSignature);
	aboutW->AddDescription(B_TRANSLATE(
		"Samedi provides 8 pads for loading audio samples, which can be triggered "
		"via notes played over MIDI."));
	aboutW->AddCopyright(2023, "Humdinger");
	aboutW->Show();
}


int
main()
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
