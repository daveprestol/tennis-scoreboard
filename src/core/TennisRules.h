#pragma once

#include "MatchState.h"

#include <vector>

namespace tennis_scoreboard {

class TennisRules {
public:
	explicit TennisRules(MatchState state = {});

	const MatchState &state() const;
	void setState(const MatchState &state);

	void startMatch();
	void finishMatch();
	void addPoint(TeamId team);
	void resetCurrentGame();
	void resetCurrentSet();
	void resetMatch();
	void setServer(TeamId team);
	bool undo();

private:
	MatchState state_;
	std::vector<MatchState> history_;

	void saveUndoPoint();
	void awardGame(TeamId team);
	void toggleServer();
	int setsWonBy(TeamId team) const;
	int setsToWin() const;
};

} // namespace tennis_scoreboard
