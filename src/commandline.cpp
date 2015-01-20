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
#include "commandline.h"
#include "utils.h"

#include <iostream>
#include <fcntl.h>

#ifdef Q_OS_WIN32
	#include <Windows.h>
	#include <io.h>
#endif

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

CommandLine::CommandLine():m_manager(NULL), m_consoleAllocated(false)
{
}

CommandLine::~CommandLine()
{
}

bool CommandLine::parseArguments(const QStringList &args)
{
	bool hasSwitch = false;
	QStringList filenames;

	foreach(QString arg, args)
	{
		if (arg[0] == '-')
		{
			hasSwitch = true;
			arg.remove(0, 1);

			// another 'less'
			if (arg[0] == '-') arg.remove(0, 1);
		}

		if (hasSwitch)
		{
			if (arg == "?" || arg == "h" || arg == "help")
			{
				// show help
			}

			hasSwitch = false;
		}
		else
		{
			// parse filenames
			filenames << arg;
		}
	}

	foreach(const QString &filename, filenames)
	{
		QFileInfo info(filename);
		QDir dir;

		QStringList files;

		if (info.isDir())
		{
			dir.setPath(filename);

			files = dir.entryList(QDir::Files | QDir::NoSymLinks);
		}
		else
		{
			dir = info.dir();

			files = dir.entryList(QStringList(info.fileName()), QDir::Files | QDir::NoSymLinks);
		}

		foreach(const QString &file, files)
		{
			// append directory and put it in files list
			m_filenames << dir.filePath(file);
		}
	}

	m_filenames.sort();
	m_filenames.removeDuplicates();

	return true;
}

bool CommandLine::processCommand()
{
	prepareLogin();

	return true;
}

bool CommandLine::openConsole()
{
#ifdef _WIN32
#ifdef ATTACH_PARENT_PROCESS
	if (!AttachConsole(ATTACH_PARENT_PROCESS))
	{
		if (AllocConsole())
		{
			m_consoleAllocated = true;
		}
	}
#endif

	if (m_consoleAllocated)
	{
		freopen("CON", "w", stdout);

#if defined(_WINCON_) && !defined(NOGDI)
		HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		UINT index = 0;
		CONSOLE_FONT_INFOEX console_font;
		console_font.cbSize = sizeof(CONSOLE_FONT_INFOEX);

		if (GetCurrentConsoleFontEx(handle, FALSE, &console_font))
		{
			memcpy(console_font.FaceName, L"Lucida Console", 30);
			console_font.nFont = index;
			console_font.FontFamily = FF_DONTCARE;
			SetCurrentConsoleFontEx(handle, FALSE, &console_font);
		}
		else
		{
			DWORD error = GetLastError();
			char buffer[256];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, buffer, 256, NULL);
			printf("%s\n", buffer);
		}
#endif

		SetConsoleOutputCP(CP_UTF8);
	}
#endif

	return true;
}

bool CommandLine::closeConsole()
{
#ifdef _WIN32
	if (m_consoleAllocated)
	{
		if (!FreeConsole()) return false;
	}
#endif

	return true;
}

void CommandLine::printQt(const QString &str)
{
	fprintf(stdout, "%s\n", str.toUtf8().data());
}

void CommandLine::init()
{
	if (m_manager) return;

	m_manager = new QNetworkAccessManager(this);
//	m_manager->setCookieJar(new Cookies(m_manager));

	connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));
}

void CommandLine::addUserAgent(QNetworkRequest &req) const
{
	if (m_userAgent.isEmpty()) return;

	req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
}

bool CommandLine::get(const QString &url)
{
	init();

	QNetworkRequest req;
	req.setUrl(QUrl(url));

	addUserAgent(req);

	QNetworkReply *reply = m_manager->get(req);

	return true;
}

bool CommandLine::post(const QString &url, const QByteArray &data)
{
	init();

	QNetworkRequest req;
	req.setUrl(QUrl(url));
	addUserAgent(req);
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QNetworkReply *reply = m_manager->post(req, data);

	return true;
}

QString CommandLine::getUserAgent()
{
	if (m_userAgent.isEmpty())
	{
		QString system;
#ifdef Q_OS_WIN32
		system = "Windows ";

		switch (QSysInfo::WindowsVersion)
		{
			case QSysInfo::WV_32s: system += "3.1 with Win32s"; break;
			case QSysInfo::WV_95: system += "95"; break;
			case QSysInfo::WV_98: system += "98"; break;
			case QSysInfo::WV_Me: system += "Me"; break;
			case QSysInfo::WV_DOS_based: system += "DOS"; break;

			case QSysInfo::WV_4_0: system += "NT 4.0"; break; // Windows NT 4
			case QSysInfo::WV_5_0: system += "NT 5.0"; break; // Windows 2000
			case QSysInfo::WV_5_1: system += "NT 5.1"; break; // Windows XP
			case QSysInfo::WV_5_2: system += "NT 5.2"; break; // Windows Vista
			case QSysInfo::WV_6_0: system += "NT 6.0"; break; // Windows 7
			case QSysInfo::WV_6_1: system += "NT 6.1"; break; // Windows 8
			case QSysInfo::WV_6_2: system += "NT 6.2"; break; // Windows 8.1
			case QSysInfo::WV_6_3: system += "NT 6.3"; break;
			case QSysInfo::WV_NT_based: system += "NT"; break;

			case QSysInfo::WV_CE: system += "CE"; break;
			case QSysInfo::WV_CENET: system += "CE Net"; break;
			case QSysInfo::WV_CE_5: system += "CE 5"; break;
			case QSysInfo::WV_CE_6: system += "CE 6"; break;
			case QSysInfo::WV_CE_based: system += "CE"; break;
		}

		system += "; ";

		// Windows target processor
		system += QString("Win%1").arg(IsOS64bits() ? 64:32);

		system += "; ";

		// application target processor
#ifdef _WIN64
		system += "x64; ";
#else
		system += "i386;";
#endif

		system += QLocale::system().name().replace('_', '-');
#else
#endif
		m_userAgent = QString("%1/%2 (%3)").arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion()).arg(system);
	}

	return m_userAgent;
}

bool CommandLine::prepareLogin()
{
	return get(getLoginUrl());
}

bool CommandLine::prepareUploadFile()
{
	return get(getFilesNewUrl());
}

bool CommandLine::login()
{
	QUrlQuery params;
	params.addQueryItem("utf8", m_utf8);
	params.addQueryItem("authenticity_token", m_authenticityToken);
	params.addQueryItem("back_url", m_baseUrl);
	params.addQueryItem("username", m_username);
	params.addQueryItem("password", m_password);

	QByteArray data = params.query().toUtf8();

	return post(getLoginUrl(), data);
}

bool CommandLine::uploadFile(const QString &filename)
{
	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly))
	{
//		emit errorReceived(tr("Unable to read file %1").arg(filename));

		return false;
	}

	QFileInfo info(filename);
	QString ext = info.suffix().toLower();

    QMimeDatabase mimeDatabase;
	QMimeType mimeType = mimeDatabase.mimeTypeForFile(info, QMimeDatabase::MatchContent);
	QString mime = mimeType.name();

	QNetworkRequest req;
	req.setUrl(QUrl(getFilesUrl()));
	addUserAgent(req);

	QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

	QHttpPart utf8Part;
	utf8Part.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"utf8\"");
	utf8Part.setBody(m_utf8.toUtf8());

	QHttpPart tokenPart;
	tokenPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"authenticity_token\"");
	tokenPart.setBody(m_authenticityToken.toUtf8());
	
	QHttpPart versionPart;
	versionPart.setHeader(QNetworkRequest::ContentDispositionHeader, "form-data; name=\"version_id\"");
	versionPart.setBody(m_versionId.toUtf8());

	QHttpPart filePart;
	filePart.setHeader(QNetworkRequest::ContentTypeHeader, mime);
	filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"attachments[dummy][file]\"; filename=\"%1\"").arg(info.fileName()));
	filePart.setBody(file.readAll());

	multiPart->append(utf8Part);
	multiPart->append(tokenPart);
	multiPart->append(versionPart);
	multiPart->append(filePart);

	QNetworkReply *reply = m_manager->post(req, multiPart);

	connect(reply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(onUploadProgress(qint64, qint64)));
	connect(reply, SIGNAL(finished()), this, SLOT(onUploadFinished()));

	return true;
}

bool CommandLine::parseAuthenticityToken(const QByteArray &content)
{
	QRegExp reg("authenticity_token\" type=\"hidden\" value=\"([A-Z0-9a-z/=+]{44})\"");

	if (reg.indexIn(content) < 0) return false;

	m_authenticityToken = reg.cap(1);

	return true;
}

bool CommandLine::parseVersionId(const QByteArray &content)
{
	QRegExp reg(QString("<option value=\"([0-9]+)\">%1</option>").arg(m_version));

	if (reg.indexIn(content) < 0) return false;

	m_versionId = reg.cap(1);

	return true;
}

QString CommandLine::getLoginUrl() const
{
	return QString("%1/login").arg(m_baseUrl);
}

QString CommandLine::getFilesUrl() const
{
	return QString("%1/projects/%2/files").arg(m_baseUrl).arg(m_project);
}

QString CommandLine::getFilesNewUrl() const
{
	return QString("%1/projects/%2/files/new").arg(m_baseUrl).arg(m_project);
}

void CommandLine::onReply(QNetworkReply *reply)
{
	QString redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().toString();
	QString url = reply->url().toString();
	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (reply->error() == QNetworkReply::NoError)
	{
		QByteArray content = reply->readAll();

		// if status code 302 (redirection), we can clear content
		if (statusCode == 302) content.clear();

#ifdef _DEBUG
		qDebug() << "URL:" << url;
		if (!redirection.isEmpty()) qDebug() << "Redirection:" << redirection;
#endif
		if (!content.isEmpty())
		{
			if (url == getLoginUrl())
			{
				if (parseAuthenticityToken(content))
				{
					// send login information
					login();
				}
				else
				{
				}
			}
			else if (url == getFilesNewUrl())
			{
				if (parseAuthenticityToken(content) && parseVersionId(content))
				{
					foreach(const QString &filename, m_filenames)
					{
						uploadFile(filename);
					}
				}
				else
				{
				}
			}
			else if (url == getFilesUrl())
			{
			}
		}
		else
		{
			if (url == getLoginUrl() && redirection == m_baseUrl)
			{
				// login successful
				prepareUploadFile();
			}
			else if (url == getFilesUrl() && redirection == url)
			{
				// upload successful
			}
		}
	}
	else
	{
	}

	// always delete QNetworkReply to avoid memory leaks
	reply->deleteLater();
}

void CommandLine::onUploadProgress(qint64 value, qint64 total)
{
//	fprintf(stdout, "%c", '.');
	fputc('.', stdout);
}

void CommandLine::onUploadFinished()
{
	printQt("");
	printQt(tr("Upload done."));
}
