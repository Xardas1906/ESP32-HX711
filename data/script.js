const webSocket = new WebSocket('ws://' + window.location.hostname + ':81/');
var audioCtx = new (window.AudioContext || window.webkitAudioContext || window.audioContext);
let intervalID = null;
var LastInterval;
let torqueSetting;
let value_beep;
let Interval;

//Initialisierung des Graphen
const ctx = document.getElementById('historyChart').getContext('2d');
const historyChart = new Chart(ctx, {
  type: 'line',
  data: {
    labels: [],
    datasets: [{
      label: 'Drehmoment',
      data: [],
      fill: false,
      borderColor: 'rgb(75, 192, 192)',
      tension: 0.1
    }]
  },
  options: {
    scales: {
      y: {
        beginAtZero: true
      }
    }
  }
});

//Optionen Gauge
var opts = {
  angle: 0, // The span of the gauge arc
  lineWidth: 0.3, // The line thickness
  radiusScale: 1, // Relative radius
  pointer: {
    length: 0.55, // // Relative to gauge radius
    strokeWidth: 0.035, // The thickness
    color: '#000000' // Fill color
  },
  limitMax: true, // If false, max value increases automatically if value > maxValue
  limitMin: true, // If true, the min value of the gauge will be fixed
  colorStart: '#6FADCF', // Colors
  colorStop: '#8FC0DA', // just experiment with them
  percentColors: [
    [0.0, "#a9d70b"],
    [0.50, "#f9c802"],
    [1.0, "#ff0000"]
  ],
  strokeColor: '#E0E0E0', // to see which ones work best for you
  generateGradient: true,
  highDpiSupport: true, // High resolution support
  // renderTicks is Optional
  renderTicks: {
    divisions: 8,
    divWidth: 1.1,
    divLength: 0.7,
    divColor: '#333333',
    subDivisions: 5,
    subLength: 0.5,
    subWidth: 0.6,
    subColor: '#666666'
  },
  staticLabels: {
    font: "10px sans-serif", // Specifies font
    labels: [25, 50, 75, 100, 125, 150, 175, 200], // Print labels at these values
    color: "#000000", // Optional: Label text color
    fractionDigits: 0 // Optional: Numerical precision. 0=round off.
  }
};

//Animation für Buttons
document.querySelectorAll('button').forEach(button => {
  button.addEventListener('touchend', () => {
    button.blur();
  });
});

//Empfangen von Daten über den Websocket
webSocket.onmessage = function(event) {
  if (event.data.startsWith('calibrationFactor:')) {
    const calibrationFactor = parseFloat(event.data.split(':')[1]);
    document.getElementById('calibrationFactor').textContent = `Kalibrierungsfaktor: ${calibrationFactor}`;
  } else {
    // Annahme, dass alle anderen Nachrichten Gewichtsdaten sind
    const value = parseFloat(event.data);
    value_beep = value;
    document.getElementById('weightDisplay').textContent = `${value} Nm`;
    updateChart(value);
    gauge.set(value);
  }
};

//Websocket error handling
webSocket.onerror = function(event) {
  console.error("WebSocket error observed:", event);
};

//Websocket close handling
webSocket.onclose = function(event) {
  console.log("WebSocket closed:", event);
};

//Senden des gemessenen Wertes vom HX711
function calibrate() {
  const knownWeight = document.getElementById('knownWeight').value;
  webSocket.send(`setKnownWeight:${knownWeight}`);
}

//Senden des Wertes von Calibration Faktor Input
function setCalibrationFactor() {
  const calibrationFactorInput = document.getElementById('calibrationFactorInput').value;
  webSocket.send(`calibrationFactorInput:${calibrationFactorInput}`);
}

//Save Calibration Factor Button wurde gedrückt
document.getElementById('saveCalibrationFactor').addEventListener('click', function() {
  webSocket.send('saveCalibrationFactor');
});

//Reset Scale wurde gedrückt
document.getElementById('reset_scale').addEventListener('click', function() {
  webSocket.send('reset_scale');
});

//Tare Button wurde gedrückt
document.getElementById('tare').addEventListener('click', function() {
  webSocket.send('tare');
});

//Einstellen des Drehmoments
document.getElementById('torqueSetting').addEventListener('change', function() {
  torqueSetting = document.getElementById('torqueSetting').value;
  document.getElementById('test').textContent = `Test: ${Math.abs(torqueSetting)}`;
});

//Update der Werte für den Graphen
function updateChart(value) {
  const now = new Date();
  const label = `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`;

  if (historyChart.data.labels.length > 200) {
    historyChart.data.labels.shift();
    historyChart.data.datasets[0].data.shift();
  }

  historyChart.data.labels.push(label);
  historyChart.data.datasets[0].data.push(value);
  historyChart.update();
}
//Aufrufen der Gauge
var target = document.getElementById('gauge'); // your canvas element
var gauge = new Gauge(target).setOptions(opts); // create sexy gauge!
gauge.maxValue = 200; // set max gauge value
gauge.animationSpeed = 32; // set animation speed (32 is default value)

function SetupInterval() {
  if (value_beep > torqueSetting) {
    value_beep = torqueSetting;
  }

  Interval = 500 - 425 * ((400 / torqueSetting) * (value_beep - torqueSetting/4*3)*0.01);
  if (intervalID === null && torqueSetting > 0)
  {
    intervalID = setInterval(playBeep, Interval)
  }
  if (value_beep < torqueSetting/4*3 && intervalID != null) {
    clearInterval(intervalID);
    intervalID = null;
  }
  if (LastInterval !== Interval && !isNaN(Interval)) {
    console.log("Interval: ", Interval);
    LastInterval = Interval;
  }
  
}

function playBeep() {
  if (value_beep >= torqueSetting) {
    beep(2000, 2300, 1, "sine");
    console.log("Long Beep!");
    clearInterval(intervalID);
  } else if (value_beep >= (torqueSetting/4) * 3) {
    beep(100, 2300, 1, "sine");
    console.log("Beep!");
    clearInterval(intervalID);  // Beende das aktuelle Beep-Intervall
    intervalID = null;  // Setze die Interval-ID zurück
  }
}

function beep(duration, frequency, volume, type, callback) {
  var oscillator = audioCtx.createOscillator();
  var gainNode = audioCtx.createGain();
  
  oscillator.connect(gainNode);
  gainNode.connect(audioCtx.destination);
  
  if (volume){gainNode.gain.value = volume;}
  if (frequency){oscillator.frequency.value = frequency;}
  if (type){oscillator.type = type;}
  if (callback){oscillator.onended = callback;}
  
  oscillator.start(audioCtx.currentTime);
  oscillator.stop(audioCtx.currentTime + ((duration || 500) / 1000));
};

setInterval(SetupInterval, 75);  // Rufe updateInterval auf
