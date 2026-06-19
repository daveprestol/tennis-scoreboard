const fallbackState = {
  type: "MATCH_STATE_UPDATED",
  version: 1,
  payload: {
    eventName: "Your Event Name",
    teamA: { displayName: "Player 1", logoPath: "" },
    teamB: { displayName: "Player 2", logoPath: "" },
    score: {
      sets: [{ teamA: 0, teamB: 0 }],
      currentGame: { teamA: "0", teamB: "0" },
      servingTeam: "teamA"
    },
    scoreIntelligence: { terms: [] },
    theme: {
      primaryColor: "#111827",
      secondaryColor: "#1F2937",
      accentColor: "#F59E0B",
      eventTitleColor: "#111827",
      eventLogoTintColor: "",
      textColor: "#FFFFFF",
      mutedTextColor: "#D1D5DB",
      teamAColor: "#2563EB",
      teamBColor: "#DC2626",
      backgroundOpacity: 0.85,
      borderRadius: 12
    }
  }
};

function setLogo(element, path) {
  if (!path) {
    element.hidden = true;
    element.removeAttribute("src");
    return;
  }

  element.hidden = false;
  element.src = path;
}

function cssUrl(path) {
  return `url("${String(path).replaceAll('"', "%22")}")`;
}

function setEventLogo(element, path, tintColor) {
  element.hidden = !path;
  element.classList.toggle("is-tinted", Boolean(path && tintColor));
  element.style.backgroundImage = "";
  element.style.webkitMaskImage = "";
  element.style.maskImage = "";
  element.style.backgroundColor = "";

  if (!path) {
    return;
  }

  if (tintColor) {
    element.style.backgroundColor = tintColor;
    element.style.webkitMaskImage = cssUrl(path);
    element.style.maskImage = cssUrl(path);
  } else {
    element.style.backgroundImage = cssUrl(path);
  }
}

function displayNameFor(team, fallback) {
  return team?.displayName || fallback;
}

function updateNameColumn(root, teamAName, teamBName) {
  const longest = Math.max(5, teamAName.length, teamBName.length);
  root.style.setProperty("--name-width", `${Math.min(longest + 5, 42)}ch`);
}

function applyState(message) {
  const state = message.payload ?? message;
  const set = state.score?.sets?.at(-1) ?? { teamA: 0, teamB: 0 };
  const setsWon = state.score?.setsWon ?? { teamA: 0, teamB: 0 };
  const theme = state.theme ?? {};
  const root = document.documentElement;

  root.style.setProperty("--primary", theme.primaryColor ?? "#111827");
  root.style.setProperty("--secondary", theme.secondaryColor ?? "#1F2937");
  root.style.setProperty("--accent", theme.accentColor ?? "#F59E0B");
  root.style.setProperty("--event-title", theme.eventTitleColor ?? "#111827");
  root.style.setProperty("--text", theme.textColor ?? "#FFFFFF");
  root.style.setProperty("--muted", theme.mutedTextColor ?? "#D1D5DB");
  root.style.setProperty("--team-a", theme.teamAColor ?? "#2563EB");
  root.style.setProperty("--team-b", theme.teamBColor ?? "#DC2626");
  root.style.setProperty("--opacity", String(theme.backgroundOpacity ?? 0.85));
  root.style.setProperty("--radius", `${theme.borderRadius ?? 12}px`);

  const teamAName = displayNameFor(state.teamA, "Player 1");
  const teamBName = displayNameFor(state.teamB, "Player 2");
  updateNameColumn(root, teamAName, teamBName);

  const isFinished = state.status === "finished" && state.winnerTeam;
  const winnerTeam = state.winnerTeam === "teamB" ? state.teamB : state.teamA;
  const winnerElement = document.querySelector(".winner");
  const noticeElement = document.querySelector(".notice");
  const hasNotice = Boolean(state.matchNotice) && !isFinished;
  document.querySelector(".scoreboard").classList.toggle("is-finished", Boolean(isFinished));
  document.querySelector(".scoreboard").classList.toggle("has-notice", hasNotice);
  winnerElement.hidden = !isFinished;
  noticeElement.hidden = !hasNotice;
  if (isFinished) {
    document.querySelector(".winner-name").textContent = winnerTeam?.displayName ?? "";
    document.querySelector(".winner-label").textContent = state.competitionType === "doubles" ? "WINNERS" : "WINNER";
  }
  if (hasNotice) {
    document.querySelector(".notice-label").textContent = state.matchNotice;
  }

  document.querySelector(".scoreboard").dataset.serving = state.score?.servingTeam ?? "teamA";
  document.querySelector(".event-name").textContent = state.eventName ?? "";
  setEventLogo(document.querySelector(".event-logo"), state.eventLogoPath, theme.eventLogoTintColor);
  document.querySelector(".team-a .name").textContent = teamAName;
  document.querySelector(".team-b .name").textContent = teamBName;
  document.querySelector(".team-a .sets").textContent = setsWon.teamA ?? 0;
  document.querySelector(".team-b .sets").textContent = setsWon.teamB ?? 0;
  document.querySelector(".team-a .games").textContent = set.teamA ?? 0;
  document.querySelector(".team-b .games").textContent = set.teamB ?? 0;
  document.querySelector(".team-a .game").textContent = state.score?.currentGame?.teamA ?? "0";
  document.querySelector(".team-b .game").textContent = state.score?.currentGame?.teamB ?? "0";
  setLogo(document.querySelector(".team-a .team-logo"), state.teamA?.logoPath);
  setLogo(document.querySelector(".team-b .team-logo"), state.teamB?.logoPath);

  const terms = state.scoreIntelligence?.terms ?? [];
  const termsElement = document.querySelector(".terms");
  termsElement.hidden = terms.length === 0;
  termsElement.replaceChildren(...terms.map((term) => {
    const badge = document.createElement("span");
    badge.className = "term";
    badge.textContent = term;
    return badge;
  }));
}

window.TennisScoreboard = { applyState };
applyState(fallbackState);

async function refreshFromPlugin() {
  try {
    const response = await fetch("/state.json", { cache: "no-store" });
    if (response.ok) {
      applyState(await response.json());
    }
  } catch {
    // File previews keep using the fallback state.
  }
}

refreshFromPlugin();
setInterval(refreshFromPlugin, 250);
