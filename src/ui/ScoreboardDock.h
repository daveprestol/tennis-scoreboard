#pragma once

#include "core/TennisRules.h"
#include "overlay/OverlayHttpServer.h"

#include <QMap>
#include <QWidget>

class QLabel;
class QPushButton;
class QShowEvent;

namespace tennis_scoreboard {

class ScoreboardDock final : public QWidget {
public:
	explicit ScoreboardDock(QString resourcePath, QWidget *parent = nullptr);

protected:
	void showEvent(QShowEvent *event) override;

private:
	TennisRules rules_;
	OverlayHttpServer overlayServer_;
	QLabel *serverStatus_ = nullptr;
	QLabel *localInfo_ = nullptr;
	QLabel *networkInfo_ = nullptr;
	QLabel *closeInfo_ = nullptr;
	QPushButton *serverToggle_ = nullptr;

	void buildUi();
	void bindActions();
	void handleRemoteAction(const QString &action, const QMap<QString, QString> &params);
	void startServerIfNeeded();
	void stopServer();
	void refreshServerInfo();
	void refreshPreview();
};

}
