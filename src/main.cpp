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
	QString folder;
	QDir dir(QCoreApplication::applicationDirPath());
	
#if defined(Q_OS_WIN32)
	folder = dir.absolutePath();
#else
	dir.cdUp();

#ifdef Q_OS_MAC
	folder = dir.absolutePath() + "/Resources";
#elif defined(SHARE_PREFIX)
	folder = SHARE_PREFIX;
#else
	folder = QString("%1/share/%2").arg(dir.absolutePath()).arg(TARGET);
#endif

#endif

	folder += "/translations";

	// take the whole locale
	QTranslator localTranslator;
	if (localTranslator.load(QString("%1_%2").arg(TARGET).arg(locale), folder))
	{
		app.installTranslator(&localTranslator);
	}

	// take the whole locale
	QTranslator qtTranslator;
	if (qtTranslator.load("qt_" + locale, folder))
	{
		app.installTranslator(&qtTranslator);
	}

	// define commandline arguments
	QCommandLineParser parser;
	parser.setApplicationDescription(DESCRIPTION);
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("root", QCoreApplication::translate("main", "Redmine root URL"), QCoreApplication::translate("main", "<root>"));
	parser.addPositionalArgument("project", QCoreApplication::translate("main", "Redmine project identifier"), QCoreApplication::translate("main", "<project>"));
	parser.addPositionalArgument("version", QCoreApplication::translate("main", "Project version"), QCoreApplication::translate("main", "<version>"));
	parser.addPositionalArgument("username", QCoreApplication::translate("main", "Redmine username"), QCoreApplication::translate("main", "<username>"));
	parser.addPositionalArgument("password", QCoreApplication::translate("main", "Redmine password"), QCoreApplication::translate("main", "<password>"));
	
	parser.addPositionalArgument("filenames", QCoreApplication::translate("main", "Files to upload"), QCoreApplication::translate("main", "[filenames...]"));

	// process the actual command line arguments given by the user
	parser.process(app);

	QStringList args = parser.positionalArguments();

	if (args.size() > 5)
	{
		Redmine redmine;
		redmine.setRootUrl(args.takeFirst());
		redmine.setProject(args.takeFirst());
		redmine.setVersion(args.takeFirst());
		redmine.setUsername(args.takeFirst());
		redmine.setPassword(args.takeFirst());
		redmine.setFilenames(args);

		redmine.upload();

		// only memory leaks are from plugins
		return QCoreApplication::exec();
	}

	parser.showHelp();

	return 0;
}
