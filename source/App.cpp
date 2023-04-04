/*
 * Copyright 2023. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#include "App.h"
#include "MainWindow.h"

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


int
main()
{
	App* app = new App();
	app->Run();
	delete app;
	return 0;
}
