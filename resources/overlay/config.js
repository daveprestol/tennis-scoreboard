async function getState() {
  const response = await fetch("/state.json", { cache: "no-store" });
  return response.json();
}

const setupSelectors = [
  "#eventName",
  "#eventLogoPath",
  "#competitionType",
  "#bestOfSets",
  "#gamesPerSet",
  "#winByGames",
  "#tiebreakAt",
  "#gameScoringMode",
  "#teamAPlayer1",
  "#teamAPlayer2",
  "#teamBPlayer1",
  "#teamBPlayer2",
  "#primaryColor",
  "#secondaryColor",
  "#accentColor",
  "#eventTitleColor",
  "#eventLogoTintEnabled",
  "#eventLogoTintColor",
  "#textColor",
  "#teamAColor",
  "#teamBColor",
  "#backgroundOpacity"
];

let setupDirty = false;
let saveTimer = null;
let savingSetup = false;

function playerValue(team, index) {
  return team?.players?.[index]?.shortName || team?.players?.[index]?.lastName || team?.players?.[index]?.firstName || "";
}

function payloadFor(action) {
  const payload = { action };
  if (action === "setSetup") {
    payload.eventName = document.querySelector("#eventName").value;
    payload.eventLogoPath = document.querySelector("#eventLogoPath").value;
    payload.competitionType = document.querySelector("#competitionType").value;
    payload.bestOfSets = document.querySelector("#bestOfSets").value;
    payload.gamesPerSet = document.querySelector("#gamesPerSet").value;
    payload.winByGames = document.querySelector("#winByGames").value;
    payload.tiebreakAt = document.querySelector("#tiebreakAt").value;
    payload.gameScoringMode = document.querySelector("#gameScoringMode").value;
    payload.teamAPlayer1 = document.querySelector("#teamAPlayer1").value;
    payload.teamAPlayer2 = document.querySelector("#teamAPlayer2").value;
    payload.teamBPlayer1 = document.querySelector("#teamBPlayer1").value;
    payload.teamBPlayer2 = document.querySelector("#teamBPlayer2").value;
    payload.primaryColor = document.querySelector("#primaryColor").value;
    payload.secondaryColor = document.querySelector("#secondaryColor").value;
    payload.accentColor = document.querySelector("#accentColor").value;
    payload.eventTitleColor = document.querySelector("#eventTitleColor").value;
    payload.eventLogoTintEnabled = document.querySelector("#eventLogoTintEnabled").checked;
    payload.eventLogoTintColor = document.querySelector("#eventLogoTintColor").value;
    payload.textColor = document.querySelector("#textColor").value;
    payload.teamAColor = document.querySelector("#teamAColor").value;
    payload.teamBColor = document.querySelector("#teamBColor").value;
    payload.backgroundOpacity = document.querySelector("#backgroundOpacity").value;
  } else if (action === "customNotice") {
    payload.customNotice = document.querySelector("#customNotice").value;
  }
  return payload;
}

async function sendAction(action) {
  if (action === "setSetup") {
    savingSetup = true;
  }

  const response = await fetch("/api/action", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(payloadFor(action)),
    cache: "no-store"
  });
  const state = await response.json();

  if (action === "setSetup") {
    setupDirty = false;
    savingSetup = false;
    applyState(state, { forceSetup: true });
  } else {
    applyState(state);
  }
}

function applyState(message, options = {}) {
  const state = message.payload ?? message;
  const isDoubles = (state.competitionType ?? "singles") === "doubles";
  document.querySelectorAll(".doubles-only").forEach((element) => {
    element.hidden = !isDoubles;
  });

  if (!setupDirty || options.forceSetup) {
    setValue("#eventName", state.eventName ?? "", options.forceSetup);
    setValue("#eventLogoPath", state.eventLogoPath ?? "", options.forceSetup);
    setValue("#competitionType", state.competitionType ?? "singles", options.forceSetup);
    setValue("#bestOfSets", state.format?.bestOfSets ?? 3, options.forceSetup);
    setValue("#gamesPerSet", state.format?.gamesPerSet ?? 6, options.forceSetup);
    setValue("#winByGames", state.format?.winByGames ?? 2, options.forceSetup);
    setValue("#tiebreakAt", state.format?.tiebreakAt ?? 6, options.forceSetup);
    setValue("#gameScoringMode", state.format?.gameScoringMode ?? "advantage", options.forceSetup);
    setValue("#teamAPlayer1", playerValue(state.teamA, 0) || state.teamA?.displayName || "Player 1", options.forceSetup);
    setValue("#teamAPlayer2", playerValue(state.teamA, 1), options.forceSetup);
    setValue("#teamBPlayer1", playerValue(state.teamB, 0) || state.teamB?.displayName || "Player 2", options.forceSetup);
    setValue("#teamBPlayer2", playerValue(state.teamB, 1), options.forceSetup);
    setValue("#primaryColor", state.theme?.primaryColor ?? "#111827", options.forceSetup);
    setValue("#secondaryColor", state.theme?.secondaryColor ?? "#1F2937", options.forceSetup);
    setValue("#accentColor", state.theme?.accentColor ?? "#F59E0B", options.forceSetup);
    setValue("#eventTitleColor", state.theme?.eventTitleColor ?? "#111827", options.forceSetup);
    setChecked("#eventLogoTintEnabled", Boolean(state.theme?.eventLogoTintColor), options.forceSetup);
    setValue("#eventLogoTintColor", state.theme?.eventLogoTintColor || "#111827", options.forceSetup);
    setValue("#textColor", state.theme?.textColor ?? "#FFFFFF", options.forceSetup);
    setValue("#teamAColor", state.theme?.teamAColor ?? "#2563EB", options.forceSetup);
    setValue("#teamBColor", state.theme?.teamBColor ?? "#DC2626", options.forceSetup);
    setValue("#backgroundOpacity", state.theme?.backgroundOpacity ?? 0.85, options.forceSetup);
  }
  const opacityInput = document.querySelector("#backgroundOpacity");
  updateOpacityValue(document.activeElement === opacityInput ? opacityInput.value : state.theme?.backgroundOpacity ?? 0.85);

  document.querySelector("#teamALabel").textContent = state.teamA?.displayName ?? "Team A";
  document.querySelector("#teamBLabel").textContent = state.teamB?.displayName ?? "Team B";
  document.querySelector("#teamAScore").textContent = state.score?.currentGame?.teamA ?? "0";
  document.querySelector("#teamBScore").textContent = state.score?.currentGame?.teamB ?? "0";
}

function setValue(selector, value, force = false) {
  const element = document.querySelector(selector);
  if (force || document.activeElement !== element) {
    element.value = value;
  }
}

function setChecked(selector, value, force = false) {
  const element = document.querySelector(selector);
  if (force || document.activeElement !== element) {
    element.checked = value;
  }
}

function updateOpacityValue(value) {
  document.querySelector("#backgroundOpacityValue").textContent = `${Math.round(Number(value) * 100)}%`;
}

function scheduleSetupSave() {
  setupDirty = true;
  clearTimeout(saveTimer);
  saveTimer = setTimeout(() => {
    if (!savingSetup) {
      sendAction("setSetup").catch(() => {
        savingSetup = false;
      });
    }
  }, 500);
}

document.querySelectorAll("[data-action]").forEach((button) => {
  button.addEventListener("click", () => sendAction(button.dataset.action));
});

setupSelectors.forEach((selector) => {
  const element = document.querySelector(selector);
  element.addEventListener("input", scheduleSetupSave);
  element.addEventListener("input", () => {
    if (selector === "#backgroundOpacity") {
      updateOpacityValue(element.value);
    }
  });
  element.addEventListener("change", () => {
    if (selector === "#competitionType") {
      document.querySelectorAll(".doubles-only").forEach((field) => {
        field.hidden = element.value !== "doubles";
      });
    }
    scheduleSetupSave();
  });
});

getState().then(applyState).catch(() => {});
setInterval(() => getState().then(applyState).catch(() => {}), 1000);
