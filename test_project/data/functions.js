let websocket;

window.addEventListener("DOMContentLoaded", () => {
  const waveformSelect = document.getElementById("waveform");

  waveformSelect.addEventListener("change", () => {
    const selected = waveformSelect.value;
    sendWaveformRequest(selected);
  });
});

function initializeWebSocket() {
  websocket = new WebSocket(`ws://${window.location.hostname}/ws`);

  websocket.onopen = () => {
    console.log("WebSocket connection opened");
  };

  websocket.onmessage = (event) => {
    console.log("Received frequency:", event.data);
    const frequencyDisplay = document.getElementById("frequencyDisplay");
    frequencyDisplay.textContent = `Current Frequency: ${event.data} Hz`;
  };

  websocket.onclose = () => {
    console.log("WebSocket connection closed");
    setTimeout(initializeWebSocket, 1000); // Attempt to reconnect after 1 second
  };
}

function sendWaveformRequest(waveform) {
  fetch(`/setWaveform?type=${encodeURIComponent(waveform)}`)
    .then(response => response.text())
    .then(data => {
      console.log("Waveform set:", data);
    })
    .catch(err => {
      console.error("Failed to send waveform:", err);
    });
}