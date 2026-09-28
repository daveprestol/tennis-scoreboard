#include "overlay/OverlayHttpServer.h"

#include <QCoreApplication>
#include <QEventLoop>
#include <QTcpSocket>
#include <QTimer>

#include <cstdlib>
#include <iostream>

using namespace tennis_scoreboard;

namespace {

QByteArray request(quint16 port, const QByteArray &path)
{
	QTcpSocket socket;
	QByteArray response;
	QEventLoop loop;
	QTimer timeout;
	timeout.setSingleShot(true);

	QObject::connect(&socket, &QTcpSocket::connected, &socket, [&] {
		socket.write("GET " + path + " HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n");
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

	server.stop();
	std::cout << "overlay-http-server-tests passed\n";
	return 0;
}
