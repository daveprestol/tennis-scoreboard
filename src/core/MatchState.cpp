#include "MatchState.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace tennis_scoreboard {
namespace {

std::string escapeJson(const std::string &value)
{
	std::ostringstream out;
	for (char c : value) {
		switch (c) {
		case '"':
			out << "\\\"";
			break;
		case '\\':
			out << "\\\\";
			break;
		case '\n':
			out << "\\n";
			break;
		case '\r':
			out << "\\r";
			break;
		case '\t':
			out << "\\t";
			break;
		default:
			out << c;
		}
	}
	return out.str();
}

int scoreFor(const SetScore &set, TeamId team)
{
	return team == TeamId::TeamA ? set.teamA : set.teamB;
}

int pointFor(const GameScore &game, TeamId team)
{
	return team == TeamId::TeamA ? game.teamAPoints : game.teamBPoints;
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

bool winsSetWithGame(const MatchState &state, TeamId team)
{
	if (state.sets.empty())
		return false;

	const auto &set = state.sets.back();
	const int own = scoreFor(set, team) + 1;
	const int other = scoreFor(set, otherTeam(team));

	if (isTiebreak(state))
		return true;

	return own >= state.format.gamesPerSet && own >= other + state.format.winByGames;
}

int setsWonBy(const MatchState &state, TeamId team)
{
	int won = 0;
	for (const auto &set : state.sets) {
		const int own = scoreFor(set, team);
		const int other = scoreFor(set, otherTeam(team));
		if (own >= state.format.gamesPerSet && own >= other + state.format.winByGames)
			++won;
		if (state.format.tiebreakAt > 0 && own == state.format.tiebreakAt + 1 && other == state.format.tiebreakAt)
			++won;
	}
	return won;
}

int setsToWin(const MatchState &state)
{
	return (std::max(1, state.format.bestOfSets) / 2) + 1;
}

bool winsMatchWithGame(const MatchState &state, TeamId team)
{
	return winsSetWithGame(state, team) && setsWonBy(state, team) + 1 >= setsToWin(state);
}

bool canWinGameNextPoint(const MatchState &state, TeamId team)
{
	const int own = pointFor(state.currentGame, team);
	const int other = pointFor(state.currentGame, otherTeam(team));
	if (isTiebreak(state))
		return own >= 6 && own >= other + 1;
	if (state.format.gameScoringMode == GameScoringMode::GoldenPoint && own == 3 && other == 3)
		return true;
	return own >= 3 && own > other;
}

std::string primaryScoreTerm(const MatchState &state)
{
	const int a = state.currentGame.teamAPoints;
	const int b = state.currentGame.teamBPoints;
	const bool tiebreak = isTiebreak(state);
	const bool goldenPoint =
		!tiebreak && state.format.gameScoringMode == GameScoringMode::GoldenPoint && a == 3 && b == 3;
	bool hasAdvantage = false;
	bool hasDeuce = !goldenPoint && !tiebreak && a == b && a >= 3;
	bool hasTiebreak = tiebreak;
	bool hasGamePoint = false;
	bool hasBreakPoint = false;
	bool hasSetPoint = false;
	bool hasMatchPoint = false;
	bool servingForSet = false;
	bool servingForMatch = false;

	for (const auto team : {TeamId::TeamA, TeamId::TeamB}) {
		const bool isServer = team == state.servingTeam;
		const int own = pointFor(state.currentGame, team);
		const int other = pointFor(state.currentGame, otherTeam(team));
		const bool advantage = !tiebreak && own >= 4 && own == other + 1;

		hasAdvantage = hasAdvantage || advantage;

		if (canWinGameNextPoint(state, team)) {
			hasMatchPoint = hasMatchPoint || winsMatchWithGame(state, team);
			hasSetPoint = hasSetPoint || winsSetWithGame(state, team);
			hasBreakPoint = hasBreakPoint || !isServer;
			hasGamePoint = true;
		}
	}

	servingForSet = winsSetWithGame(state, state.servingTeam);
	servingForMatch = winsMatchWithGame(state, state.servingTeam);

	if (hasMatchPoint)
		return "Match point";
	if (hasSetPoint)
		return "Set point";
	if (goldenPoint)
		return "Golden Point";
	if (hasDeuce)
		return "Deuce";
	if (hasBreakPoint)
		return "Break point";
	if (hasGamePoint)
		return "Game point";
	if (hasAdvantage)
		return "Advantage / Ad";
	if (hasTiebreak)
		return "Tiebreak";
	if (servingForMatch)
		return "Serving for the match";
	if (servingForSet)
		return "Serving for the set";
	return "";
}

void appendScoreIntelligence(std::ostringstream &out, const MatchState &state)
{
	const auto term = primaryScoreTerm(state);
	out << "\"scoreIntelligence\":{\"terms\":[";
	if (!term.empty())
		out << "\"" << escapeJson(term) << "\"";
	out << "]}";
}

std::string noticeForStatus(MatchStatus status)
{
	switch (status) {
	case MatchStatus::Suspended:
		return "SUSPENDED";
	case MatchStatus::Interrupted:
		return "INTERRUPTED";
	case MatchStatus::Delayed:
		return "DELAYED";
	case MatchStatus::Retired:
		return "RETIRED / RET";
	case MatchStatus::Walkover:
		return "WALKOVER / WO";
	default:
		return "";
	}
}

void appendTeamJson(std::ostringstream &out, const Team &team)
{
	out << "{\"displayName\":\"" << escapeJson(team.displayName) << "\",";
	out << "\"logoPath\":\"" << escapeJson(team.logoPath) << "\",";
	out << "\"players\":[";
	for (size_t i = 0; i < team.players.size(); ++i) {
		const auto &player = team.players[i];
		if (i > 0)
			out << ",";
		out << "{\"firstName\":\"" << escapeJson(player.firstName) << "\",";
		out << "\"lastName\":\"" << escapeJson(player.lastName) << "\",";
		out << "\"shortName\":\"" << escapeJson(player.shortName) << "\"}";
	}
	out << "]}";
}

}

std::string teamIdToString(TeamId team)
{
	return team == TeamId::TeamA ? "teamA" : "teamB";
}

std::string competitionTypeToString(CompetitionType type)
{
	return type == CompetitionType::Singles ? "singles" : "doubles";
}

std::string gameScoringModeToString(GameScoringMode mode)
{
	return mode == GameScoringMode::GoldenPoint ? "golden_point" : "advantage";
}

std::string matchStatusToString(MatchStatus status)
{
	switch (status) {
	case MatchStatus::NotStarted:
		return "not_started";
	case MatchStatus::Warmup:
		return "warmup";
	case MatchStatus::InProgress:
		return "in_progress";
	case MatchStatus::SetBreak:
		return "set_break";
	case MatchStatus::MedicalTimeout:
		return "medical_timeout";
	case MatchStatus::Suspended:
		return "suspended";
	case MatchStatus::Interrupted:
		return "interrupted";
	case MatchStatus::Delayed:
		return "delayed";
	case MatchStatus::Retired:
		return "retired";
	case MatchStatus::Walkover:
		return "walkover";
	case MatchStatus::Finished:
		return "finished";
	}
	return "not_started";
}

std::string pointLabelFor(const GameScore &game, TeamId team)
{
	const int own = team == TeamId::TeamA ? game.teamAPoints : game.teamBPoints;
	const int other = team == TeamId::TeamA ? game.teamBPoints : game.teamAPoints;

	if (own >= 4 && own >= other + 1)
		return own == other + 1 ? "AD" : "GAME";

	switch (own) {
	case 0:
		return "0";
	case 1:
		return "15";
	case 2:
		return "30";
	default:
		return "40";
	}
}

std::string tiebreakLabelFor(const GameScore &game, TeamId team)
{
	return std::to_string(pointFor(game, team));
}

std::string matchStateToJson(const MatchState &state)
{
	std::ostringstream out;
	out << "{\"type\":\"MATCH_STATE_UPDATED\",\"version\":1,\"payload\":{";
	out << "\"competitionType\":\"" << competitionTypeToString(state.competitionType) << "\",";
	out << "\"status\":\"" << matchStatusToString(state.status) << "\",";
	const auto notice = noticeForStatus(state.status);
	if (!state.customNotice.empty())
		out << "\"matchNotice\":\"" << escapeJson(state.customNotice) << "\",";
	else if (!notice.empty())
		out << "\"matchNotice\":\"" << notice << "\",";
	if (state.winnerTeam.has_value())
		out << "\"winnerTeam\":\"" << teamIdToString(*state.winnerTeam) << "\",";
	out << "\"eventName\":\"" << escapeJson(state.eventName) << "\",";
	out << "\"eventLogoPath\":\"" << escapeJson(state.eventLogoPath) << "\",";
	out << "\"format\":{\"bestOfSets\":" << state.format.bestOfSets << ",\"gamesPerSet\":" << state.format.gamesPerSet
	    << ",\"winByGames\":" << state.format.winByGames << ",\"tiebreakAt\":" << state.format.tiebreakAt
	    << ",\"gameScoringMode\":\"" << gameScoringModeToString(state.format.gameScoringMode) << "\"},";
	out << "\"teamA\":";
	appendTeamJson(out, state.teamA);
	out << ",\"teamB\":";
	appendTeamJson(out, state.teamB);
	out << ",\"score\":{\"sets\":[";
	for (size_t i = 0; i < state.sets.size(); ++i) {
		if (i > 0)
			out << ",";
		out << "{\"teamA\":" << state.sets[i].teamA << ",\"teamB\":" << state.sets[i].teamB << "}";
	}
	out << "],\"setsWon\":{\"teamA\":" << setsWonBy(state, TeamId::TeamA) << ",\"teamB\":"
	    << setsWonBy(state, TeamId::TeamB) << "},\"currentGame\":{\"teamA\":\""
	    << (isTiebreak(state) ? tiebreakLabelFor(state.currentGame, TeamId::TeamA)
				  : pointLabelFor(state.currentGame, TeamId::TeamA))
	    << "\",\"teamB\":\""
	    << (isTiebreak(state) ? tiebreakLabelFor(state.currentGame, TeamId::TeamB)
				  : pointLabelFor(state.currentGame, TeamId::TeamB))
	    << "\"},";
	out << "\"servingTeam\":\"" << teamIdToString(state.servingTeam) << "\"},";
	out << "\"theme\":{\"primaryColor\":\"" << state.theme.primaryColor << "\",";
	out << "\"secondaryColor\":\"" << state.theme.secondaryColor << "\",";
	out << "\"accentColor\":\"" << state.theme.accentColor << "\",";
	out << "\"eventTitleColor\":\"" << state.theme.eventTitleColor << "\",";
	out << "\"eventLogoTintColor\":\"" << escapeJson(state.theme.eventLogoTintColor) << "\",";
	out << "\"textColor\":\"" << state.theme.textColor << "\",";
	out << "\"mutedTextColor\":\"" << state.theme.mutedTextColor << "\",";
	out << "\"teamAColor\":\"" << state.theme.teamAColor << "\",";
	out << "\"teamBColor\":\"" << state.theme.teamBColor << "\",";
	out << "\"backgroundOpacity\":" << state.theme.backgroundOpacity << ",";
	out << "\"borderRadius\":" << state.theme.borderRadius << "},";
	appendScoreIntelligence(out, state);
	out << "}}";
	return out.str();
}

}
