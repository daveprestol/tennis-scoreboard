#include "TennisRules.h"

#include <algorithm>
#include <utility>

namespace tennis_scoreboard {
namespace {

int &gamesFor(SetScore &set, TeamId team)
{
	return team == TeamId::TeamA ? set.teamA : set.teamB;
}

int gamesFor(const SetScore &set, TeamId team)
{
	return team == TeamId::TeamA ? set.teamA : set.teamB;
}

TeamId otherTeam(TeamId team)
{
	return team == TeamId::TeamA ? TeamId::TeamB : TeamId::TeamA;
}

bool isTiebreak(const MatchState &state)
{
	if (state.sets.empty() || state.format.tiebreakAt <= 0)
		return false;
	const auto &set = state.sets.back();
	return set.teamA == state.format.tiebreakAt && set.teamB == state.format.tiebreakAt;
}

}

TennisRules::TennisRules(MatchState state) : state_(std::move(state)) {}

const MatchState &TennisRules::state() const
{
	return state_;
}

void TennisRules::setState(const MatchState &state)
{
	saveUndoPoint();
	state_ = state;
}

void TennisRules::startMatch()
{
	saveUndoPoint();
	state_.status = MatchStatus::InProgress;
	state_.customNotice.clear();
}

void TennisRules::finishMatch()
{
	saveUndoPoint();
	state_.status = MatchStatus::Finished;
}

void TennisRules::addPoint(TeamId team)
{
	if (state_.status == MatchStatus::Finished)
		return;

	saveUndoPoint();
	int &own = team == TeamId::TeamA ? state_.currentGame.teamAPoints : state_.currentGame.teamBPoints;
	const int other = team == TeamId::TeamA ? state_.currentGame.teamBPoints : state_.currentGame.teamAPoints;

	++own;
	if (isTiebreak(state_)) {
		if (own >= 7 && own >= other + 2)
			awardGame(team);
		return;
	}

	if (state_.format.gameScoringMode == GameScoringMode::GoldenPoint && own >= 4 && other == 3) {
		awardGame(team);
		return;
	}

	if (own >= 4 && own >= other + 2)
		awardGame(team);
}

void TennisRules::resetCurrentGame()
{
	saveUndoPoint();
	state_.currentGame = {};
}

void TennisRules::resetCurrentSet()
{
	saveUndoPoint();
	if (state_.sets.empty())
		state_.sets.push_back({});
	state_.sets.back() = {};
	state_.currentGame = {};
}

void TennisRules::resetMatch()
{
	saveUndoPoint();
	const auto eventName = state_.eventName;
	const auto eventLogoPath = state_.eventLogoPath;
	const auto teamA = state_.teamA;
	const auto teamB = state_.teamB;
	const auto competitionType = state_.competitionType;
	const auto format = state_.format;
	const auto theme = state_.theme;
	const auto servingTeam = state_.servingTeam;
	state_ = {};
	state_.eventName = eventName;
	state_.eventLogoPath = eventLogoPath;
	state_.teamA = teamA;
	state_.teamB = teamB;
	state_.competitionType = competitionType;
	state_.format = format;
	state_.theme = theme;
	state_.servingTeam = servingTeam;
	state_.winnerTeam.reset();
	state_.customNotice.clear();
}

void TennisRules::setServer(TeamId team)
{
	saveUndoPoint();
	state_.servingTeam = team;
}

bool TennisRules::undo()
{
	if (history_.empty())
		return false;

	state_ = history_.back();
	history_.pop_back();
	return true;
}

void TennisRules::saveUndoPoint()
{
	history_.push_back(state_);
	if (history_.size() > 50)
		history_.erase(history_.begin());
}

void TennisRules::awardGame(TeamId team)
{
	if (state_.sets.empty())
		state_.sets.push_back({});

	auto &currentSet = state_.sets.back();
	const bool completingTiebreak = isTiebreak(state_);
	++gamesFor(currentSet, team);
	state_.currentGame = {};
	toggleServer();

	const int own = team == TeamId::TeamA ? currentSet.teamA : currentSet.teamB;
	const int other = team == TeamId::TeamA ? currentSet.teamB : currentSet.teamA;
	if (completingTiebreak || (own >= state_.format.gamesPerSet && own >= other + state_.format.winByGames)) {
		if (setsWonBy(team) >= setsToWin()) {
			state_.status = MatchStatus::Finished;
			state_.winnerTeam = team;
		} else {
			state_.sets.push_back({});
		}
	}
}

void TennisRules::toggleServer()
{
	state_.servingTeam = state_.servingTeam == TeamId::TeamA ? TeamId::TeamB : TeamId::TeamA;
}

int TennisRules::setsWonBy(TeamId team) const
{
	int won = 0;
	for (const auto &set : state_.sets) {
		const int own = gamesFor(set, team);
		const int other = gamesFor(set, otherTeam(team));
		if (own >= state_.format.gamesPerSet && own >= other + state_.format.winByGames)
			++won;
		else if (state_.format.tiebreakAt > 0 && own == state_.format.tiebreakAt + 1 && other == state_.format.tiebreakAt)
			++won;
	}
	return won;
}

int TennisRules::setsToWin() const
{
	return (std::max(1, state_.format.bestOfSets) / 2) + 1;
}

}
