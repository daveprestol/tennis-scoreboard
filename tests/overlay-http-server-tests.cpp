#include "overlay/OverlayHttpServer.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTcpSocket>
#include <QTimer>

#include <cstdlib>
#include <iostream>

using namespace tennis_scoreboard;

namespace {

QByteArray request(quint16 port, const QByteArray &path, const QByteArray &body = {})
{
	QTcpSocket socket;
	QByteArray response;
	QEventLoop loop;
	QTimer timeout;
	timeout.setSingleShot(true);

	QObject::connect(&socket, &QTcpSocket::connected, &socket, [&] {
		const QByteArray method = body.isEmpty() ? "GET" : "POST";
		socket.write(method + " " + path +
			     " HTTP/1.1\r\nHost: 127.0.0.1\r\nContent-Type: application/json\r\nContent-Length: " +
			     QByteArray::number(body.size()) + "\r\nConnection: close\r\n\r\n" + body);
	});
	QObject::connect(&socket, &QTcpSocket::readyRead, &socket, [&] { response += socket.readAll(); });
	QObject::connect(&socket, &QTcpSocket::disconnected, &loop, &QEventLoop::quit);
	QObject::connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

	timeout.start(3000);
	socket.connectToHost("127.0.0.1", port);
	loop.exec();
	response += socket.readAll();
	return response;
}

void require(bool condition, const char *message)
{
	if (!condition) {
		std::cerr << message << "\n";
		std::exit(1);
	}
}

void expectOk(const QByteArray &response, const QByteArray &content)
{
	require(response.startsWith("HTTP/1.1 200 OK"), "unexpected HTTP status");
	require(response.contains(content), "expected response content was not found");
}

} // namespace

int main(int argc, char **argv)
{
	QCoreApplication application(argc, argv);
	OverlayHttpServer server(TEST_RESOURCE_PATH);
	server.setStateJson(R"({"status":"test"})");
	require(server.start(0, 0), "overlay server failed to start");

	expectOk(request(server.scorePort(), "/score"), "scoreboard.js");
	expectOk(request(server.configPort(), "/config"), "config.js");
	expectOk(request(server.scorePort(), "/state.json"), R"("status":"test")");

	int actions = 0;
	server.setActionHandler([&](const QString &action, const QMap<QString, QString> &params) {
		require(action == "setSetup", "action was not decoded");
		require(params.value("eventName") == "Office Match", "setup value was not decoded");
		++actions;
		server.setStateJson(R"({"eventName":"Office Match"})");
	});
	for (const auto port : {server.configPort(), server.scorePort()}) {
		expectOk(request(port, "/config"), "config.js");
		expectOk(request(port, "/api/action", R"({"action":"setSetup","eventName":"Office Match"})"),
			 "Office Match");
		expectOk(request(server.scorePort(), "/state.json"), "Office Match");
	}
	require(actions == 2, "both config URLs must support updates");

	server.stop();
	std::cout << "overlay-http-server-tests passed\n";
	return 0;
}
