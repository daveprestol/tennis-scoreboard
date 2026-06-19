#include "../src/core/TennisRules.h"

#include <cassert>
#include <iostream>
#include <string>

using namespace tennis_scoreboard;

int main()
{
	TennisRules rules;
	rules.startMatch();
	assert(rules.state().status == MatchStatus::InProgress);

	rules.addPoint(TeamId::TeamA);
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamA) == "15");

	rules.addPoint(TeamId::TeamA);
	rules.addPoint(TeamId::TeamA);
	rules.addPoint(TeamId::TeamB);
	rules.addPoint(TeamId::TeamB);
	rules.addPoint(TeamId::TeamB);
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamA) == "40");
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamB) == "40");

	rules.addPoint(TeamId::TeamA);
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamA) == "AD");

	rules.addPoint(TeamId::TeamA);
	assert(rules.state().sets.back().teamA == 1);
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamA) == "0");
	assert(rules.state().servingTeam == TeamId::TeamB);

	assert(rules.undo());
	assert(pointLabelFor(rules.state().currentGame, TeamId::TeamA) == "AD");

	rules.resetMatch();
	for (int set = 0; set < 2; ++set) {
		for (int game = 0; game < 6; ++game) {
			rules.addPoint(TeamId::TeamA);
			rules.addPoint(TeamId::TeamA);
			rules.addPoint(TeamId::TeamA);
			rules.addPoint(TeamId::TeamA);
		}
	}
	assert(rules.state().status == MatchStatus::Finished);
	assert(rules.state().winnerTeam.has_value());
	assert(*rules.state().winnerTeam == TeamId::TeamA);
	const auto finished = rules.state();
	rules.addPoint(TeamId::TeamB);
	assert(rules.state().sets.size() == finished.sets.size());
	assert(rules.state().currentGame.teamBPoints == finished.currentGame.teamBPoints);

	MatchState breakPointState;
	breakPointState.servingTeam = TeamId::TeamA;
	breakPointState.currentGame.teamBPoints = 3;
	const auto breakPointJson = matchStateToJson(breakPointState);
	assert(breakPointJson.find("Break point") != std::string::npos);
	assert(breakPointJson.find("Game point") == std::string::npos);

	MatchState deuceState;
	deuceState.currentGame.teamAPoints = 3;
	deuceState.currentGame.teamBPoints = 3;
	const auto deuceJson = matchStateToJson(deuceState);
	assert(deuceJson.find("Deuce") != std::string::npos);
	assert(deuceJson.find("Break point") == std::string::npos);
	assert(deuceJson.find("Game point") == std::string::npos);

	MatchState serverAdState;
	serverAdState.servingTeam = TeamId::TeamA;
	serverAdState.currentGame.teamAPoints = 4;
	serverAdState.currentGame.teamBPoints = 3;
	const auto serverAdJson = matchStateToJson(serverAdState);
	assert(serverAdJson.find("Game point") != std::string::npos);
	assert(serverAdJson.find("Advantage") == std::string::npos);

	MatchState receiverAdState;
	receiverAdState.servingTeam = TeamId::TeamA;
	receiverAdState.currentGame.teamAPoints = 3;
	receiverAdState.currentGame.teamBPoints = 4;
	const auto receiverAdJson = matchStateToJson(receiverAdState);
	assert(receiverAdJson.find("Break point") != std::string::npos);
	assert(receiverAdJson.find("Advantage") == std::string::npos);

	MatchState matchPointState;
	matchPointState.servingTeam = TeamId::TeamA;
	matchPointState.sets = {{6, 0}, {5, 0}};
	matchPointState.currentGame.teamAPoints = 3;
	const auto matchPointJson = matchStateToJson(matchPointState);
	assert(matchPointJson.find("Match point") != std::string::npos);
	assert(matchPointJson.find("Set point") == std::string::npos);

	MatchState goldenPointState;
	goldenPointState.format.gameScoringMode = GameScoringMode::GoldenPoint;
	goldenPointState.currentGame.teamAPoints = 3;
	goldenPointState.currentGame.teamBPoints = 3;
	const auto goldenPointJson = matchStateToJson(goldenPointState);
	assert(goldenPointJson.find("Golden Point") != std::string::npos);
	assert(goldenPointJson.find("Deuce") == std::string::npos);

	TennisRules goldenRules;
	auto goldenState = goldenRules.state();
	goldenState.format.gameScoringMode = GameScoringMode::GoldenPoint;
	goldenRules.setState(goldenState);
	goldenRules.addPoint(TeamId::TeamA);
	goldenRules.addPoint(TeamId::TeamA);
	goldenRules.addPoint(TeamId::TeamA);
	goldenRules.addPoint(TeamId::TeamB);
	goldenRules.addPoint(TeamId::TeamB);
	goldenRules.addPoint(TeamId::TeamB);
	assert(goldenRules.state().currentGame.teamAPoints == 3);
	assert(goldenRules.state().currentGame.teamBPoints == 3);
	goldenRules.addPoint(TeamId::TeamB);
	assert(goldenRules.state().sets.back().teamB == 1);
	assert(goldenRules.state().currentGame.teamAPoints == 0);
	assert(goldenRules.state().currentGame.teamBPoints == 0);

	std::cout << "tennis-rules-tests passed\n";
	return 0;
}
