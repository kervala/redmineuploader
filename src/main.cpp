/*
 *  RedmineUploader is a tool to upload files to Redmine
 *  Copyright (C) 2015  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include "redmine.h"

#ifdef HAVE_CONFIG_H
	#include "config.h"
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag (_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	QCoreApplication app(argc, argv);

	// set application information
	QCoreApplication::setApplicationName(PRODUCT);
	QCoreApplication::setOrganizationName(AUTHOR);
	QCoreApplication::setApplicationVersion(VERSION);

	// detect system locale
	QString locale = QLocale::system().name().left(2);

	// detect directory with translations
	QString localFolder, systemFolder;
	QDir dir(QCoreApplication::applicationDirPath());
	
#if defined(Q_OS_WIN32)
	localFolder = dir.absolutePath();

	// under Windows, both files are in the same directory
	systemFolder = localFolder;
#else
	dir.cdUp();

#ifdef Q_OS_MAC
	localFolder = dir.absolutePath() + "/Resources";

	// under OS X, both files are in the same directory
	systemFolder = localFolder;
#elif defined(SHARE_PREFIX)
	localFolder = SHARE_PREFIX;
	systemFolder = "/usr/share/qt5";
#else
	localFolder = QString("%1/share/%2").arg(dir.absolutePath()).arg(TARGET);
	systemFolder = "/usr/share/qt5";
#endif

#endif

	localFolder += "/translations";
	systemFolder += "/translations";

	// take the whole locale
	QTranslator localTranslator;
	if (localTranslator.load(QString("%1_%2").arg(TARGET).arg(locale), localFolder))
	{
		app.installTranslator(&localTranslator);
	}

	// take the whole locale
	QTranslator qtTranslator;
	if (qtTranslator.load("qt_" + locale, systemFolder))
	{
		app.installTranslator(&qtTranslator);
	}

	Redmine redmine;
	
	if (!redmine.parseCommandLine(app)) return 0;

	redmine.upload();

	// only memory leaks are from plugins
	return QCoreApplication::exec();
}
