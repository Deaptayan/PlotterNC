const statusEl = document.querySelector("#status");
const homeButton = document.querySelector("#home");

async function refreshStatus() {
  try {
    const response = await fetch("/api/status");
    if (!response.ok) return;
    const status = await response.json();
    statusEl.textContent = status.state || "Idle";
  } catch {
    statusEl.textContent = "Offline";
  }
}

homeButton.addEventListener("click", async () => {
  await fetch("/api/home", { method: "POST" });
  refreshStatus();
});

refreshStatus();
setInterval(refreshStatus, 1000);
