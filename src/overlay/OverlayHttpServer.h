#pragma once

#include <QObject>
#include <QTcpServer>

#include <functional>

class QTcpSocket;

namespace tennis_scoreboard {

class OverlayHttpServer final : public QObject {
public:
	using ActionHandler = std::function<void(const QString &, const QMap<QString, QString> &)>;

	explicit OverlayHttpServer(QString resourcePath, QObject *parent = nullptr);

	bool start(quint16 scorePort = 19876, quint16 configPort = 9876);
	bool isRunning() const;
	QString scoreUrl() const;
	QString configUrl() const;
	quint16 scorePort() const;
	quint16 configPort() const;
	void stop();
	void setStateJson(QByteArray stateJson);
	void setActionHandler(ActionHandler handler);

private:
	QString resourcePath_;
	QTcpServer scoreServer_;
	QTcpServer configServer_;
	quint16 scorePort_ = 0;
	quint16 configPort_ = 0;
	QByteArray stateJson_;
	ActionHandler actionHandler_;

	void handleConnection(QTcpServer *server);
	void handleRequest(QTcpSocket *socket, bool configServer);
	void handleAction(QTcpSocket *socket, const QUrl &url, const QByteArray &request);
	void sendResponse(QTcpSocket *socket, int statusCode, QByteArray contentType, QByteArray body);
	void sendFile(QTcpSocket *socket, const QString &relativePath, QByteArray contentType);
};

}
