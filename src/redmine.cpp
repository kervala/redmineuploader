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
#include "utils.h"

#include <fcntl.h>

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

Redmine::Redmine():m_manager(NULL)
{
	m_utf8 = "&#x2713;";
	m_manager = new QNetworkAccessManager(this);

	connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReply(QNetworkReply*)));

	updateUserAgent();
}

Redmine::~Redmine()
{
}

bool Redmine::setRootUrl(const QString &root)
{
	m_rootUrl = root;

	return true;
}

bool Redmine::setProject(const QString &project)
{
	m_project = project;

	return true;
}

bool Redmine::setUsername(const QString &username)
{
	m_username = username;

	return true;
}

bool Redmine::setPassword(const QString &password)
{
	m_password = password;

	return true;
}

bool Redmine::setVersion(const QString &version)
{
	m_version = version;

	return true;
}

bool Redmine::setFilenames(const QStringList &filenames)
{
	m_filenames.clear();

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

bool Redmine::upload()
{
	return prepareLogin();
}

void Redmine::printQt(const QString &str)
{
	fprintf(stdout, "%s", str.toUtf8().data());
}

void Redmine::printQtLine(const QString &str)
{
	fprintf(stdout, "%s\n", str.toUtf8().data());
}

void Redmine::addUserAgent(QNetworkRequest &req) const
{
	if (m_userAgent.isEmpty()) return;

	req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
}

bool Redmine::get(const QString &url)
{
	QNetworkRequest req;
	req.setUrl(QUrl(url));

	addUserAgent(req);

	QNetworkReply *reply = m_manager->get(req);

	return true;
}

bool Redmine::post(const QString &url, const QByteArray &data)
{
	QNetworkRequest req;
	req.setUrl(QUrl(url));
	addUserAgent(req);
	req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QNetworkReply *reply = m_manager->post(req, data);

	return true;
}

void Redmine::updateUserAgent()
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
		case QSysInfo::WV_5_2: system += "NT 5.2"; break; // Windows 2003
		case QSysInfo::WV_6_0: system += "NT 6.0"; break; // Windows Vista
		case QSysInfo::WV_6_1: system += "NT 6.1"; break; // Windows 7
		case QSysInfo::WV_6_2: system += "NT 6.2"; break; // Windows 8
		case QSysInfo::WV_6_3: system += "NT 6.3"; break; // Windows 8.1
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

bool Redmine::prepareLogin()
{
	return get(getLoginUrl());
}

bool Redmine::prepareUploadFile()
{
	return get(getFilesNewUrl());
}

bool Redmine::login()
{
	printQtLine(tr("Login to %1 with username %2").arg(m_rootUrl).arg(m_username));

	QUrlQuery params;
	params.addQueryItem("utf8", m_utf8);
	params.addQueryItem("authenticity_token", m_authenticityToken);
	params.addQueryItem("back_url", m_rootUrl);
	params.addQueryItem("username", m_username);
	params.addQueryItem("password", m_password);

	return post(getLoginUrl(), params.query().toUtf8());
}

bool Redmine::uploadFile(const QString &filename)
{
	QFile file(filename);

	if (!file.open(QIODevice::ReadOnly))
	{
		printQtLine(tr("Unable to read file %1").arg(filename));

		return false;
	}

	printQt(tr("Uploading %1").arg(filename));

	QFileInfo info(filename);

	// check mime type
    QMimeDatabase mimeDatabase;
	QMimeType mimeType = mimeDatabase.mimeTypeForFile(info, QMimeDatabase::MatchContent);
	QString mime = mimeType.name();

	// create the request
	QNetworkRequest req;
	req.setUrl(QUrl(getFilesUrl()));
	addUserAgent(req);

	// create multipart
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

	return true;
}

bool Redmine::parseError(const QByteArray &content, QString &error)
{
	QRegExp reg("<div class=\"flash error\" id=\"flash_error\">([^<]*)</div>");

	if (reg.indexIn(content) < 0) return false;

	error = reg.cap(1);

	return true;
}

bool Redmine::parseAuthenticityToken(const QByteArray &content)
{
	QRegExp reg("authenticity_token\" type=\"hidden\" value=\"([A-Z0-9a-z/=+]{44})\"");

	if (reg.indexIn(content) < 0) return false;

	m_authenticityToken = reg.cap(1);

	printQtLine(tr("Found authenticity token %1").arg(m_authenticityToken));

	return true;
}

bool Redmine::parseVersionId(const QByteArray &content)
{
	QRegExp reg(QString("<option value=\"([0-9]+)\">%1</option>").arg(m_version));

	if (reg.indexIn(content) < 0)
	{
		m_versionId.clear();
	}
	else
	{
		m_versionId = reg.cap(1);

		printQtLine(tr("Found version ID %1").arg(m_versionId));
	}

	return true;
}

QString Redmine::getLoginUrl() const
{
	return QString("%1/login").arg(m_rootUrl);
}

QString Redmine::getFilesUrl() const
{
	return QString("%1/projects/%2/files").arg(m_rootUrl).arg(m_project);
}

QString Redmine::getFilesNewUrl() const
{
	return QString("%1/projects/%2/files/new").arg(m_rootUrl).arg(m_project);
}

void Redmine::onReply(QNetworkReply *reply)
{
	QString redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl().toString();
	QString url = reply->url().toString();
	int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	QByteArray content = reply->readAll();
	QString error;

	// always delete QNetworkReply to avoid memory leaks
	reply->deleteLater();

	if (reply->error() == QNetworkReply::NoError)
	{
		// if status code 302 (redirection), we can clear content
		if (statusCode == 302) content.clear();

		if (!content.isEmpty() && !parseError(content, error))
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
					error = tr("Unable to find authenticity token");
				}
			}
			else if (url == getFilesNewUrl())
			{
				if (!parseAuthenticityToken(content))
				{
					error = tr("Unable to find authenticity token");
				}
				else if (!parseVersionId(content))
				{
					// it should never happen
					error = tr("Unable to find version ID");
				}
				else if (m_filenames.isEmpty())
				{
					error = tr("No files to upload");
				}
				else
				{
					uploadFile(m_filenames.takeFirst());
				}
			}
			else
			{
				error = tr("Unknown URL: %1").arg(url);
			}
		}
		else
		{
			if (url == getLoginUrl() && redirection == m_rootUrl)
			{
				// login successful
				prepareUploadFile();
			}
			else if (url == getFilesUrl() && redirection == url)
			{
				// upload successful
				if (!m_filenames.isEmpty())
				{
					// upload next file
					prepareUploadFile();
				}
				else
				{
					printQtLine();
					printQtLine(tr("All files successfully uploaded"));

					QCoreApplication::exit(0);
				}
			}
			else
			{
				error = tr("Unknown URL: %1 or redirection: %2").arg(url).arg(redirection);
			}
		}
	}
	else
	{
		error = reply->errorString();
	}

	if (!error.isEmpty())
	{
		printQtLine(tr("Error %1: %2").arg(statusCode).arg(error));

		QCoreApplication::exit(1);
	}
}

void Redmine::onUploadProgress(qint64 value, qint64 total)
{
	printQt(".");
}