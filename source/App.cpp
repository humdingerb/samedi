/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include <AboutWindow.h>
#include <Alert.h>
#include <Catalog.h>
#include <PathFinder.h>
#include <Roster.h>
#include <StringList.h>

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
	delete fMainWindow;
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


void
App::ArgvReceived(int32 argc, char** argv)
{
	BMessenger messenger(fMainWindow);
	BMessage message(B_REFS_RECEIVED);

	BEntry entry(argv[1], true); // traverse links
	entry_ref ref;
	entry.GetRef(&ref);

	if (entry.Exists())
		message.AddRef("refs", &ref);
	else {
		printf("%s: Ensemble not found: %s\n", argv[0], argv[1]);
		return;
	}
	messenger.SendMessage(&message);
}


void
App::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case FIRST_LAUNCH:
		{
			_ShowLatencyAlert();
			break;
		}
		default:
		{
			BApplication::MessageReceived(msg);
			break;
		}
	}
}


void
App::_ShowLatencyAlert()
{
	BString text(B_TRANSLATE("If you experience delays between hitting a key and "
		"hearing a sound, please see the 'Latency' section of Samedi's Help document."));

	BAlert* alert = new BAlert("First launch", text,
		B_TRANSLATE("Cancel"), B_TRANSLATE("Show latency help"));
	alert->SetShortcut(0, B_ESCAPE);
	alert->MoveTo(BPoint(280, 250));
	int32 choice = alert->Go();

	switch (choice) {
		case 0:
			return;
		case 1:
		{
			BPathFinder pathFinder;
			BStringList paths;
			BPath path;

			pathFinder.FindPaths(B_FIND_PATH_DOCUMENTATION_DIRECTORY,
				"packages/Samedi", paths);
			if (!paths.IsEmpty()) {
				if (path.SetTo(paths.StringAt(0)) == B_OK) {
					BMessage message(B_REFS_RECEIVED);
					BString url;
					url << "file://" << path.Path() << "/ReadMe.html#latency";
					message.AddString("url", url);
					be_roster->Launch("text/html", &message);
				}
			}
		}
	}
}


int
main()
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
