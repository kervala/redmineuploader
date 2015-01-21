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

#ifndef REDMINE_H
#define REDMINE_H

class QNetworkAccessManager;

class Redmine : public QObject
{
	Q_OBJECT

public:
	Redmine();
	virtual ~Redmine();

	bool parseCommandLine(const QCoreApplication &app);

	bool setFilenames(const QStringList &filenames);

	bool loadSettings();
	bool saveSettings();

	bool upload();

public slots:
	void onReply(QNetworkReply *reply);
	void onUploadProgress(qint64 value, qint64 total);

private:
	void printQt(const QString &str);
	void printQtLine(const QString &str = "");

	bool get(const QString &url);
	bool post(const QString &url, const QByteArray &data);

	void updateUserAgent();
	void addUserAgent(QNetworkRequest &req) const;

	bool prepareLogin();
	bool login();
	bool prepareUploadFile();
	bool uploadFile(const QString &filename);

	bool parseError(const QByteArray &content, QString &error);
	bool parseAuthenticityToken(const QByteArray &content);
	bool parseVersionId(const QByteArray &content);

	QString getLoginUrl() const;
	QString getFilesUrl() const;
	QString getFilesNewUrl() const;

	QStringList m_filenames;
	QString m_userAgent;
	QString m_rootUrl;
	QString m_project;
	QString m_username;
	QString m_password;
	QString m_authenticityToken;
	QString m_version;
	QString m_versionId;
	QString m_utf8;

	QNetworkAccessManager *m_manager;
};

#endif
