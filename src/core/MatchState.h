#pragma once

#include <string>
#include <optional>
#include <vector>

namespace tennis_scoreboard {

enum class TeamId { TeamA, TeamB };
enum class CompetitionType { Singles, Doubles };
enum class GameScoringMode { Advantage, GoldenPoint };
enum class MatchStatus {
	NotStarted,
	Warmup,
	InProgress,
	SetBreak,
	MedicalTimeout,
	Suspended,
	Interrupted,
	Delayed,
	Retired,
	Walkover,
	Finished
};

struct Player {
	std::string firstName;
	std::string lastName;
	std::string shortName;
};

struct Team {
	std::string displayName;
	std::string logoPath;
	std::vector<Player> players;
};

struct Theme {
	std::string primaryColor = "#111827";
	std::string secondaryColor = "#1F2937";
	std::string accentColor = "#F59E0B";
	std::string eventTitleColor = "#111827";
	std::string eventLogoTintColor;
	std::string textColor = "#FFFFFF";
	std::string mutedTextColor = "#D1D5DB";
	std::string teamAColor = "#2563EB";
	std::string teamBColor = "#DC2626";
	double backgroundOpacity = 0.85;
	int borderRadius = 12;
};

struct MatchFormat {
	int bestOfSets = 3;
	int gamesPerSet = 6;
	int winByGames = 2;
	int tiebreakAt = 6;
	GameScoringMode gameScoringMode = GameScoringMode::Advantage;
};

struct SetScore {
	int teamA = 0;
	int teamB = 0;
};

struct GameScore {
	int teamAPoints = 0;
	int teamBPoints = 0;
};

struct MatchState {
	CompetitionType competitionType = CompetitionType::Singles;
	MatchStatus status = MatchStatus::NotStarted;
	std::string eventName = "Your Event Name";
	std::string eventLogoPath;
	std::string customNotice;
	MatchFormat format;
	Team teamA = {"Player 1", "", {{"", "Player 1", "Player 1"}}};
	Team teamB = {"Player 2", "", {{"", "Player 2", "Player 2"}}};
	std::vector<SetScore> sets = {SetScore{}};
	GameScore currentGame;
	TeamId servingTeam = TeamId::TeamA;
	std::optional<TeamId> winnerTeam;
	Theme theme;
};

std::string teamIdToString(TeamId team);
std::string competitionTypeToString(CompetitionType type);
std::string gameScoringModeToString(GameScoringMode mode);
std::string matchStatusToString(MatchStatus status);
std::string pointLabelFor(const GameScore &game, TeamId team);
std::string matchStateToJson(const MatchState &state);

} // namespace tennis_scoreboard
