let websocket;

window.addEventListener("DOMContentLoaded", () => {
  const waveformSelect = document.getElementById("waveform");
  const scaleSelect = document.getElementById("scale");
  const modeRadios = document.querySelectorAll('input[name="mode"]');

  waveformSelect.addEventListener("change", () => {
    const selected = waveformSelect.value;
    sendWaveformRequest(selected);
  });

  scaleSelect.addEventListener("change", () => {
    const selectedScale = scaleSelect.value;
    sendScaleRequest(selectedScale);
  });

  modeRadios.forEach(radio => {
    radio.addEventListener("change", () => {
      if (radio.checked) {
        sendModeRequest(radio.value);
      }
    });
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

function sendModeRequest(mode) {
  fetch(`/setMode?value=${encodeURIComponent(mode)}`)
    .then(response => response.text())
    .then(data => {
      console.log("Mode set:", data);
    })
    .catch(err => {
      console.error("Failed to send mode:", err);
    });
}

function sendScaleRequest(scale) {
  fetch(`/setScale?value=${encodeURIComponent(scale)}`)
    .then(response => response.text())
    .then(data => {
      console.log("Scale set:", data);
    })
    .catch(err => {
      console.error("Failed to send scale:", err);
    });
}
