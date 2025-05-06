function setupThereminController() {
  const waveformSelect = document.getElementById("waveform");

  waveformSelect.addEventListener("change", (event) => {
    const waveform = event.target.value;

    if (window.oscillator) {
      oscillator.type = waveform;
    }

    console.log("Waveform changed to:", waveform);
  });

  // Web Audio API setup
  const AudioContextClass = window.AudioContext || window.webkitAudioContext;
  const audioCtx = new AudioContextClass();
  const oscillator = audioCtx.createOscillator();
  oscillator.type = "sine";
  oscillator.frequency.setValueAtTime(440, audioCtx.currentTime);
  oscillator.connect(audioCtx.destination);
  oscillator.start();

  window.oscillator = oscillator;
}

window.addEventListener("DOMContentLoaded", setupThereminController);
