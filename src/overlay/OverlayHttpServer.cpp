#include "OverlayHttpServer.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>

namespace tennis_scoreboard {
namespace {

QByteArray reasonPhrase(int statusCode)
{
	return statusCode == 200 ? "OK" : "Not Found";
}

}

OverlayHttpServer::OverlayHttpServer(QString resourcePath, QObject *parent)
	: QObject(parent), resourcePath_(std::move(resourcePath))
{
	connect(&scoreServer_, &QTcpServer::newConnection, this, [this] { handleConnection(&scoreServer_); });
	connect(&configServer_, &QTcpServer::newConnection, this, [this] { handleConnection(&configServer_); });
}

bool OverlayHttpServer::start(quint16 scorePort, quint16 configPort)
{
	if (isRunning())
		return true;

	if (!scoreServer_.listen(QHostAddress::Any, scorePort))
		return false;

	if (!configServer_.listen(QHostAddress::Any, configPort)) {
		scoreServer_.close();
		return false;
	}

	scorePort_ = scoreServer_.serverPort();
	configPort_ = configServer_.serverPort();
	return true;
}

bool OverlayHttpServer::isRunning() const
{
	return scoreServer_.isListening() && configServer_.isListening();
}

QString OverlayHttpServer::scoreUrl() const
{
	return QString("http://127.0.0.1:%1/score").arg(scorePort_);
}

QString OverlayHttpServer::configUrl() const
{
	return QString("http://127.0.0.1:%1/config").arg(configPort_);
}

quint16 OverlayHttpServer::scorePort() const
{
	return scorePort_;
}

quint16 OverlayHttpServer::configPort() const
{
	return configPort_;
}

void OverlayHttpServer::stop()
{
	scoreServer_.close();
	configServer_.close();
	scorePort_ = 0;
	configPort_ = 0;
}

void OverlayHttpServer::setStateJson(QByteArray stateJson)
{
	stateJson_ = std::move(stateJson);
}

void OverlayHttpServer::setActionHandler(ActionHandler handler)
{
	actionHandler_ = std::move(handler);
}

void OverlayHttpServer::handleConnection(QTcpServer *server)
{
	while (auto *socket = server->nextPendingConnection()) {
		const bool isConfigServer = server == &configServer_;
		connect(socket, &QTcpSocket::readyRead, this, [this, socket, isConfigServer] {
			handleRequest(socket, isConfigServer);
		});
		connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
	}
}

void OverlayHttpServer::handleRequest(QTcpSocket *socket, bool configServer)
{
	const QByteArray request = socket->readAll();
	const QList<QByteArray> firstLine = request.split('\n').value(0).trimmed().split(' ');
	if (firstLine.size() < 2) {
		sendResponse(socket, 404, "text/plain", "Not found");
		return;
	}

	const QUrl url(QString::fromUtf8(firstLine.at(1)));
	const QString path = url.path();
	if (path == "/score" || (!configServer && (path == "/" || path == "/index.html"))) {
		sendFile(socket, "overlay/index.html", "text/html; charset=utf-8");
	} else if (path == "/config" || (configServer && path == "/")) {
		sendFile(socket, "overlay/config.html", "text/html; charset=utf-8");
	} else if (path == "/scoreboard.css") {
		sendFile(socket, "overlay/scoreboard.css", "text/css; charset=utf-8");
	} else if (path == "/scoreboard.js") {
		sendFile(socket, "overlay/scoreboard.js", "application/javascript; charset=utf-8");
	} else if (path == "/config.css") {
		sendFile(socket, "overlay/config.css", "text/css; charset=utf-8");
	} else if (path == "/config.js") {
		sendFile(socket, "overlay/config.js", "application/javascript; charset=utf-8");
	} else if (path == "/state.json") {
		sendResponse(socket, 200, "application/json; charset=utf-8", stateJson_);
	} else if (configServer && path == "/api/action") {
		handleAction(socket, url, request);
	} else {
		sendResponse(socket, 404, "text/plain; charset=utf-8", "Not found");
	}
}

void OverlayHttpServer::handleAction(QTcpSocket *socket, const QUrl &url, const QByteArray &request)
{
	QMap<QString, QString> params;
	const qsizetype bodyStart = request.indexOf("\r\n\r\n");
	const QByteArray body = bodyStart >= 0 ? request.sliced(bodyStart + 4) : QByteArray{};
	const QJsonDocument document = QJsonDocument::fromJson(body);

	if (document.isObject()) {
		const QJsonObject object = document.object();
		for (auto it = object.begin(); it != object.end(); ++it)
			params.insert(it.key(), it.value().toVariant().toString());
	} else {
		QUrlQuery query(url);
		for (const auto &item : query.queryItems(QUrl::FullyDecoded))
			params.insert(item.first, item.second);
	}

	if (actionHandler_)
		actionHandler_(params.value("action"), params);

	sendResponse(socket, 200, "application/json; charset=utf-8", stateJson_);
}

void OverlayHttpServer::sendResponse(QTcpSocket *socket, int statusCode, QByteArray contentType, QByteArray body)
{
	QByteArray response;
	response += "HTTP/1.1 " + QByteArray::number(statusCode) + " " + reasonPhrase(statusCode) + "\r\n";
	response += "Content-Type: " + contentType + "\r\n";
	response += "Cache-Control: no-store\r\n";
	response += "Access-Control-Allow-Origin: *\r\n";
	response += "Content-Length: " + QByteArray::number(body.size()) + "\r\n";
	response += "Connection: close\r\n\r\n";
	response += body;

	socket->write(response);
	socket->disconnectFromHost();
}

void OverlayHttpServer::sendFile(QTcpSocket *socket, const QString &relativePath, QByteArray contentType)
{
	QFile file(resourcePath_ + "/" + relativePath);
	if (!file.open(QIODevice::ReadOnly)) {
		sendResponse(socket, 404, "text/plain; charset=utf-8", "Not found");
		return;
	}

	sendResponse(socket, 200, std::move(contentType), file.readAll());
}

}
