#include "ScoreboardDock.h"

#include <QAbstractSocket>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QNetworkInterface>
#include <QPushButton>
#include <QSizePolicy>
#include <QShowEvent>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>

namespace tennis_scoreboard {
namespace {

int boundedInt(const QMap<QString, QString> &params, const QString &key, int fallback, int minValue, int maxValue)
{
	bool ok = false;
	const int value = params.value(key, QString::number(fallback)).toInt(&ok);
	return ok ? std::clamp(value, minValue, maxValue) : fallback;
}

double boundedDouble(const QMap<QString, QString> &params, const QString &key, double fallback, double minValue,
		     double maxValue)
{
	bool ok = false;
	const double value = params.value(key, QString::number(fallback)).toDouble(&ok);
	return ok ? std::clamp(value, minValue, maxValue) : fallback;
}

Player playerFromName(const QString &name)
{
	const QString trimmed = name.trimmed();
	const int split = trimmed.lastIndexOf(' ');
	Player player;
	player.shortName = trimmed.toStdString();
	if (split > 0) {
		player.firstName = trimmed.left(split).toStdString();
		player.lastName = trimmed.mid(split + 1).toStdString();
	} else {
		player.lastName = trimmed.toStdString();
	}
	return player;
}

void setTeamPlayers(Team &team, const QString &player1, const QString &player2, CompetitionType competitionType,
		    const QString &fallback)
{
	team.players.clear();

	const bool usingFallback = player1.trimmed().isEmpty();
	const QString first = usingFallback ? fallback : player1.trimmed();
	const QString second = player2.trimmed();

	if (competitionType == CompetitionType::Doubles && second.isEmpty() && first.contains('/')) {
		const auto names = first.split('/', Qt::SkipEmptyParts);
		for (const auto &name : names)
			team.players.push_back(playerFromName(name));
	} else {
		team.players.push_back(playerFromName(first));

		if (competitionType == CompetitionType::Doubles && !second.isEmpty())
			team.players.push_back(playerFromName(second));
	}

	if (usingFallback) {
		team.displayName = fallback.toStdString();
		return;
	}

	if (competitionType == CompetitionType::Singles) {
		team.displayName = team.players.front().shortName;
		return;
	}

	QStringList displayNames;
	for (const auto &player : team.players) {
		const QString shortName = QString::fromStdString(player.shortName);
		if (!shortName.isEmpty())
			displayNames << shortName;
	}

	team.displayName = displayNames.join("/").toStdString();
}

} // namespace

ScoreboardDock::ScoreboardDock(QString resourcePath, QWidget *parent)
	: QWidget(parent),
	  overlayServer_(std::move(resourcePath), this)
{
	overlayServer_.setActionHandler([this](const QString &action, const QMap<QString, QString> &params) {
		handleRemoteAction(action, params);
	});
	buildUi();
	bindActions();
	refreshPreview();
}

void ScoreboardDock::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	startServerIfNeeded();
}

void ScoreboardDock::buildUi()
{
	auto *root = new QVBoxLayout(this);
	root->setContentsMargins(4, 4, 4, 4);
	root->setSpacing(4);

	auto *output = new QGroupBox("Tennis Scoreboard Server", this);
	auto *outputLayout = new QFormLayout(output);
	outputLayout->setContentsMargins(6, 6, 6, 6);
	outputLayout->setSpacing(4);
	serverStatus_ = new QLabel("Stopped", output);
	localInfo_ = new QLabel("Open this dock to start.", output);
	networkInfo_ = new QLabel("LAN URL appears while running.", output);
	serverToggle_ = new QPushButton("Start Server", output);
	serverToggle_->setFixedWidth(112);
	serverToggle_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	closeInfo_ = new QLabel("You can close this dock; the server keeps running.", output);
	auto *buttonRow = new QWidget(output);
	auto *buttonLayout = new QHBoxLayout(buttonRow);
	buttonLayout->setContentsMargins(0, 0, 0, 0);
	buttonLayout->addWidget(serverToggle_);
	buttonLayout->addStretch();
	localInfo_->setWordWrap(true);
	networkInfo_->setWordWrap(true);
	closeInfo_->setWordWrap(true);
	localInfo_->setTextInteractionFlags(Qt::TextSelectableByMouse);
	networkInfo_->setTextInteractionFlags(Qt::TextSelectableByMouse);
	outputLayout->addRow("Status", serverStatus_);
	outputLayout->addRow("This Mac", localInfo_);
	outputLayout->addRow("Other devices", networkInfo_);
	outputLayout->addRow(closeInfo_);
	outputLayout->addRow(buttonRow);

	root->addWidget(output);
	root->addStretch();
}

void ScoreboardDock::bindActions()
{
	connect(serverToggle_, &QPushButton::clicked, this, [this] {
		if (overlayServer_.isRunning())
			stopServer();
		else
			startServerIfNeeded();
	});
}

void ScoreboardDock::handleRemoteAction(const QString &action, const QMap<QString, QString> &params)
{
	if (action == "setSetup") {
		auto state = rules_.state();
		const auto competitionType = params.value("competitionType") == "doubles" ? CompetitionType::Doubles
											  : CompetitionType::Singles;
		state.eventName = params.value("eventName").toStdString();
		state.eventLogoPath = params.value("eventLogoPath").toStdString();
		state.competitionType = competitionType;
		state.format.bestOfSets = boundedInt(params, "bestOfSets", state.format.bestOfSets, 1, 9);
		if (state.format.bestOfSets % 2 == 0)
			++state.format.bestOfSets;
		state.format.gamesPerSet = boundedInt(params, "gamesPerSet", state.format.gamesPerSet, 1, 99);
		state.format.winByGames = boundedInt(params, "winByGames", state.format.winByGames, 1, 9);
		state.format.tiebreakAt = boundedInt(params, "tiebreakAt", state.format.tiebreakAt, 0, 99);
		state.format.gameScoringMode = params.value("gameScoringMode") == "golden_point"
						       ? GameScoringMode::GoldenPoint
						       : GameScoringMode::Advantage;
		setTeamPlayers(state.teamA, params.value("teamAPlayer1"), params.value("teamAPlayer2"), competitionType,
			       "Player 1");
		setTeamPlayers(state.teamB, params.value("teamBPlayer1"), params.value("teamBPlayer2"), competitionType,
			       "Player 2");
		state.theme.primaryColor =
			params.value("primaryColor", QString::fromStdString(state.theme.primaryColor)).toStdString();
		state.theme.secondaryColor =
			params.value("secondaryColor", QString::fromStdString(state.theme.secondaryColor)).toStdString();
		state.theme.accentColor =
			params.value("accentColor", QString::fromStdString(state.theme.accentColor)).toStdString();
		state.theme.eventTitleColor =
			params.value("eventTitleColor", QString::fromStdString(state.theme.eventTitleColor))
				.toStdString();
		state.theme.eventLogoTintColor =
			params.value("eventLogoTintEnabled") == "true"
				? params.value("eventLogoTintColor",
					       QString::fromStdString(state.theme.eventLogoTintColor))
					  .toStdString()
				: std::string{};
		state.theme.textColor =
			params.value("textColor", QString::fromStdString(state.theme.textColor)).toStdString();
		state.theme.teamAColor =
			params.value("teamAColor", QString::fromStdString(state.theme.teamAColor)).toStdString();
		state.theme.teamBColor =
			params.value("teamBColor", QString::fromStdString(state.theme.teamBColor)).toStdString();
		state.theme.backgroundOpacity =
			boundedDouble(params, "backgroundOpacity", state.theme.backgroundOpacity, 0.2, 1.0);
		rules_.setState(state);
	} else if (action == "pointA") {
		rules_.addPoint(TeamId::TeamA);
	} else if (action == "pointB") {
		rules_.addPoint(TeamId::TeamB);
	} else if (action == "serverA") {
		rules_.setServer(TeamId::TeamA);
	} else if (action == "serverB") {
		rules_.setServer(TeamId::TeamB);
	} else if (action == "start") {
		rules_.startMatch();
	} else if (action == "finish") {
		rules_.finishMatch();
	} else if (action == "suspended" || action == "interrupted" || action == "delayed" || action == "retired" ||
		   action == "walkover") {
		auto state = rules_.state();
		state.customNotice.clear();
		if (action == "suspended")
			state.status = MatchStatus::Suspended;
		else if (action == "interrupted")
			state.status = MatchStatus::Interrupted;
		else if (action == "delayed")
			state.status = MatchStatus::Delayed;
		else if (action == "retired")
			state.status = MatchStatus::Retired;
		else if (action == "walkover")
			state.status = MatchStatus::Walkover;
		rules_.setState(state);
	} else if (action == "customNotice") {
		auto state = rules_.state();
		const auto notice = params.value("customNotice").trimmed();
		if (!notice.isEmpty()) {
			state.status = MatchStatus::Interrupted;
			state.customNotice = notice.toStdString();
			rules_.setState(state);
		}
	} else if (action == "undo") {
		rules_.undo();
	} else if (action == "resetGame") {
		rules_.resetCurrentGame();
	} else if (action == "resetSet") {
		rules_.resetCurrentSet();
	} else if (action == "resetMatch") {
		rules_.resetMatch();
	}

	refreshPreview();
}

void ScoreboardDock::startServerIfNeeded()
{
	if (overlayServer_.isRunning())
		return;

	if (overlayServer_.start()) {
		refreshServerInfo();
	} else {
		serverStatus_->setText("Could not start ports 19876 / 9876");
	}
}

void ScoreboardDock::stopServer()
{
	overlayServer_.stop();
	refreshServerInfo();
}

void ScoreboardDock::refreshServerInfo()
{
	if (!overlayServer_.isRunning()) {
		serverStatus_->setText("Server stopped");
		localInfo_->setText("Click Start Server to enable pages.");
		networkInfo_->setText("Server is off.");
		serverToggle_->setText("Start Server");
		return;
	}

	QString networkText;
	for (const auto &address : QNetworkInterface::allAddresses()) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol && !address.isLoopback()) {
			networkText +=
				QString("Config/control: http://%1:%2/config\nBrowser Source URL: http://%1:%3/score\n")
					.arg(address.toString())
					.arg(overlayServer_.configPort())
					.arg(overlayServer_.scorePort());
		}
	}

	if (networkText.isEmpty())
		networkText = "No LAN IP detected. Connect the Mac to Wi-Fi/Ethernet to control from another device.";

	serverStatus_->setText("Running");
	localInfo_->setText(
		QString("Config/control: %1\nBrowser Source URL: %2\nCreate a Browser Source in OBS and use the Browser Source URL.")
			.arg(overlayServer_.configUrl(), overlayServer_.scoreUrl()));
	networkInfo_->setText(networkText.trimmed());
	serverToggle_->setText("Stop Server");
}

void ScoreboardDock::refreshPreview()
{
	overlayServer_.setStateJson(QByteArray::fromStdString(matchStateToJson(rules_.state())));
}

} // namespace tennis_scoreboard
