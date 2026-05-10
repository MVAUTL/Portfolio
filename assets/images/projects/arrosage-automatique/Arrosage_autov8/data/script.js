document.addEventListener("DOMContentLoaded", function() {
    fetchStatus();
    InfoModeAuto();
});

function controlValve(valveNumber, action) {
    const xhr = new XMLHttpRequest();
    xhr.open("GET", `/control?valve=${valveNumber}&action=${action}`, true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            updateIndicator(valveNumber, action);
        }
    };
    xhr.send();
}

function controlValveWithTimer(valveNumber) {
    const time = document.getElementById(`time${valveNumber}`).value;
    const xhr = new XMLHttpRequest();
    xhr.open("GET", `/timer?valve=${valveNumber}&time=${time}`, true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            updateIndicator(valveNumber, 'on');
        }
    };
    xhr.send();
}

function setAutoMode(valveNumber) {
    const start = document.getElementById(`start${valveNumber}`).value;
    const duration = document.getElementById(`duration${valveNumber}`).value;
    const xhr = new XMLHttpRequest();
    xhr.open("GET", `/auto?valve=${valveNumber}&start=${start}&duration=${duration}`, true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            fetchStatus();  // Fetch the updated status
        }
    };
    xhr.send();
    InfoModeAuto();
}

function updateIndicator(valveNumber, action) {
    const indicator = document.getElementById(`status${valveNumber}`);
    if (indicator) {
        if (action === 'on') {
            indicator.classList.remove('red');
            indicator.classList.add('green');
        } else {
            indicator.classList.remove('green');
            indicator.classList.add('red');
        }
    }
}

function updateAutoIndicator(valveNumber, isActive) {
    const autoIndicator = document.getElementById(`autoStatus${valveNumber}`);
    if (autoIndicator) {
        if (isActive) {
            autoIndicator.classList.remove('gray');
            autoIndicator.classList.add('dark-blue');
        } else {
            autoIndicator.classList.remove('dark-blue');
            autoIndicator.classList.add('gray');
        }
    }
}

function updatePumpIndicator(action) {
    const indicator = document.getElementById('status6');
    if (indicator) {
        if (action === 'on') {
            indicator.classList.remove('red');
            indicator.classList.add('green');
        } else {
            indicator.classList.remove('green');
            indicator.classList.add('red');
        }
    }
}


function valueStartTimeModeAuto(valveNumber, AutoStartTime){
    let Hours = Math.floor(AutoStartTime / 60);  // Calcul des heures
    let Minutes = AutoStartTime % 60;  // Calcul des minutes
    const Time = `${String(Hours).padStart(2, '0')}:${String(Minutes).padStart(2, '0')}`;
    document.getElementById(`start${valveNumber}`).value = Time;
}

function valueTimeModeAuto(valveNumber, AutoTime){
    document.getElementById(`duration${valveNumber}`).value = AutoTime;
}


function fetchStatus() {
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "/status", true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            const statusData = xhr.responseText.split(",");

            // Il y a 5 relais (index 0 à 4)
            // Puis 5 modes auto (index 5 à 9)
            // Puis la pompe (index 10)
            for (let i = 0; i < 5; i++) {
                const relayStatus = statusData[i] === '1' ? 'on' : 'off';
                updateIndicator(i + 1, relayStatus);

                const autoStatus = statusData[i + 5] === '1';
                updateAutoIndicator(i + 1, autoStatus);
            }

            // Mettre à jour la pompe si tu as une fonction pour ça
            const pumpStatus = statusData[10] === '1' ? 'on' : 'off';
            updatePumpIndicator(pumpStatus); // Assure-toi d’avoir cette fonction
        }
    };
    xhr.send();
}

function InfoModeAuto() {
    const xhr = new XMLHttpRequest();
    xhr.open("GET", "/InfoModeAuto", true);
    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            const statusData = xhr.responseText.split(",");
            for (let i = 0; i < 6; i++) {
                const AutoStartTime = parseInt(statusData[i]);
                valueStartTimeModeAuto(i + 1, AutoStartTime);

                const AutoTime = parseInt(statusData[i + 6]);
                valueTimeModeAuto(i + 1, AutoTime);
            }
        }
    };
    xhr.send();
}

setInterval(fetchStatus, 1000);  // Appelle fetchStatus toutes les secondes pour actualiser les voyants

