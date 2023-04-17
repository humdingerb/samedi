/*
 * Copyright 2011. All rights reserved.
 * Distributed under the terms of the MIT license.
 *
 * Author:
 *	Humdinger, humdingerb@gmail.com
 */

#ifndef APP_H
#define APP_H

#include "MainWindow.h"

#include <Application.h>


class App : public BApplication {
public:
					App();
	virtual			~App();

	virtual void	AboutRequested();
	virtual	void	ArgvReceived(int32 argc, char** argv);
	virtual	void	MessageReceived(BMessage* msg);
	virtual void 	RefsReceived(BMessage* msg);

private:
	void			_ShowLatencyAlert();

	MainWindow*		fMainWindow;
};

#endif /* APP_H */
