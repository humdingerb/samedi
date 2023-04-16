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

		void			AboutRequested();

private:
		void			_ShowLatencyAlert();

		MainWindow*		fMainWindow;
};

#endif /* APP_H */
