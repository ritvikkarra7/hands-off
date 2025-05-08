window.addEventListener("DOMContentLoaded", () => {
  const waveformSelect = document.getElementById("waveform");

  waveformSelect.addEventListener("change", () => {
    const selected = waveformSelect.value;
    sendWaveformRequest(selected);
  });
});

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
