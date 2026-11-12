// Initial data for all 3 floors
const floorsData = {
  1: [
    { id: "1A-01", status: "available", forced: false, entry: "--:--", duration: "0m 0s" },
    { id: "1A-02", status: "available", forced: false, entry: "--:--", duration: "0m 0s" },
    { id: "1A-03", status: "available", forced: false, entry: "--:--", duration: "0m 0s" },
    { id: "1A-04", status: "available", forced: false, entry: "--:--", duration: "0m 0s" },
  ],
  2: Array.from({ length: 10 }, (_, i) => ({
    id: `2A-${String(i + 1).padStart(2, '0')}`,
    status: i % 3 === 0 ? "occupied" : "available",
    forced: false,
    entry: i % 3 === 0 ? "04:05" : "--:--",
    duration: i % 3 === 0 ? "21m 10s" : "0m 0s"
  })),
  3: Array.from({ length: 10 }, (_, i) => ({
    id: `3A-${String(i + 1).padStart(2, '0')}`,
    status: i < 2 ? "occupied" : "available",
    forced: false,
    entry: i < 2 ? "03:55" : "--:--",
    duration: i < 2 ? "31m 05s" : "0m 0s"
  }))
};

let currentFloor = 1;
let selectedIndex = 3; // Default to 1A-04

function renderDashboard() {
  const slots = floorsData[currentFloor];
  const topRow = document.getElementById("topRow");
  const bottomRow = document.getElementById("bottomRow");
  
  topRow.innerHTML = "";
  bottomRow.innerHTML = "";

  slots.forEach((slot, index) => {
    const box = document.createElement("div");
    const stateClass = slot.forced ? "forced" : slot.status;
    const isSelected = index === selectedIndex ? "selected" : "";
    
    box.className = `slot-box ${stateClass} ${isSelected}`;
    box.onclick = () => selectSlot(index);

    const dotColor = slot.status === "occupied" ? "red" : "green";
    const statusLabel = slot.status === "occupied" ? "Occupied" : "Available";

    box.innerHTML = `
      <div class="slot-status">
        <span class="dot ${dotColor}"></span>
        <span>${statusLabel}</span>
      </div>
      <div class="slot-id">${slot.id}</div>
    `;

    if (index < 5) {
      topRow.appendChild(box);
    } else {
      bottomRow.appendChild(box);
    }
  });

  updateTelemetry();
}

function updateTelemetry() {
  const slots = floorsData[currentFloor];
  const currentSlot = slots[selectedIndex];
  const occupiedCount = slots.filter(s => s.status === "occupied").length;

  document.getElementById("statLot").textContent = currentSlot.id;
  
  const beamEl = document.getElementById("statBeam");
  if (currentSlot.status === "occupied") {
    beamEl.textContent = "Blocked";
    beamEl.className = "stat-value text-red";
  } else {
    beamEl.textContent = "Clear";
    beamEl.className = "stat-value text-green";
  }

  document.getElementById("statEntry").textContent = currentSlot.entry;
  document.getElementById("statDuration").textContent = currentSlot.duration;
  document.getElementById("statTotalFull").textContent = `${occupiedCount} / ${slots.length}`;

  // Update Control Labels
  document.getElementById("simTitle").textContent = `Simulate Spot ${currentSlot.id} Physical Obstruction`;
  document.getElementById("btnSimulateText").textContent = 
    currentSlot.status === "occupied" ? "Clear Car (Unblock Beam)" : "Park Car (Block Beam)";

  document.getElementById("btnOverrideText").textContent = 
    currentSlot.forced ? "Release Manual Override" : "Force Occupied Status";
}

function selectSlot(index) {
  selectedIndex = index;
  renderDashboard();
}

function switchFloor(floorNum, btnElement) {
  currentFloor = floorNum;
  selectedIndex = 0;
  document.getElementById("zoneTitle").textContent = `Floor ${floorNum} · Zone A`;

  document.querySelectorAll(".tab-btn").forEach(btn => btn.classList.remove("active"));
  btnElement.classList.add("active");

  renderDashboard();
}

function getCurrentTimeStr() {
  const now = new Date();
  return `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}`;
}

function toggleSimulation() {
  const slot = floorsData[currentFloor][selectedIndex];
  slot.forced = false;

  if (slot.status === "occupied") {
    slot.status = "available";
    slot.entry = "--:--";
    slot.duration = "0m 0s";
  } else {
    slot.status = "occupied";
    slot.entry = getCurrentTimeStr();
    slot.duration = "0m 1s";
  }
  renderDashboard();
}

function toggleOverride() {
  const slot = floorsData[currentFloor][selectedIndex];
  if (!slot.forced) {
    slot.forced = true;
    slot.status = "occupied";
    if (slot.entry === "--:--") {
      slot.entry = getCurrentTimeStr();
      slot.duration = "0m 1s";
    }
  } else {
    slot.forced = false;
    slot.status = "available";
    slot.entry = "--:--";
    slot.duration = "0m 0s";
  }
  renderDashboard();
}

// Initialize dashboard on load
renderDashboard();
