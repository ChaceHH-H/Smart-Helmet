// Your web app's Firebase configuration
const firebaseConfig = {
    apiKey: "AIzaSyDQLBZquq18ScMUz-CusPv1LZtTwV4dJxA",
    authDomain: "fir-flutter-codelab-a80ae.firebaseapp.com",
    projectId: "fir-flutter-codelab-a80ae",
    storageBucket: "fir-flutter-codelab-a80ae.appspot.com",
    messagingSenderId: "226303980858",
    appId: "1:226303980858:web:b887d62cba3c22fff34c03"
};

// Initialize Firebase
firebase.initializeApp(firebaseConfig);
const db = firebase.firestore();

const canvas = document.getElementById('heartRateCanvas');
const ctx = canvas.getContext('2d');
let heartRateData = [];
let uploadTimestamp = '';
const maxDataPoints = 60; // Show last 60 seconds of data

async function fetchLatestData() {
    const latestDataSnapshot = await db.collection("Helmet-data-3in1")
        .orderBy("timestamp", "desc")
        .limit(1)
        .get();

    const latestImpactSnapshot = await db.collection("Helmet-data-impact")
        .orderBy("timestamp", "desc")
        .limit(1)
        .get();

    if (!latestDataSnapshot.empty && !latestImpactSnapshot.empty) {
        const data = latestDataSnapshot.docs[0].data();
        const impactData = latestImpactSnapshot.docs[0].data();

        // Add latest data items to the dashboard
        const dataContainer = document.getElementById('dataContainer');
        dataContainer.innerHTML = ''; // Clear previous data
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/test-tube.png', 'C2H5OH', data.C2H5OH, data.C2H5OHalarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/gas.png', 'CH4', data.CH4, data.CH4alarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/poison-bottle.png', 'CO', data.CO, data.COalarm);
        addDataItemSimple(dataContainer, '<i class="fas fa-hard-hat"></i>', 'FSR', data.fsr);
        addImpactDataItem(dataContainer, '<i class="fas fa-car-crash"></i>', impactData.impact, impactData.timestamp); // Use impact collection data
        uploadTimestamp = new Date(data.timestamp.seconds * 1000).toLocaleString();
        addTimestampItem(dataContainer, '<i class="fas fa-clock"></i>', uploadTimestamp);

        // Update heart rate data
        updateHeartRateData(data.avgbpm);
    }
}

function addDataItem(container, icon, label, value, alarm) {
    const item = document.createElement('div');
    item.className = 'data-item';
    item.innerHTML = `
        <img src="${icon}" alt="${label} Icon">
        <span>${label}: ${value}</span>
        <span>${label} Alarm: ${alarm ? 'Yes' : 'No'}</span>
    `;
    container.appendChild(item);
}

function addDataItemSimple(container, icon, label, value) {
    const item = document.createElement('div');
    item.className = 'data-item';
    item.innerHTML = `
        ${icon}
        <span>${label}: ${value ? 'Yes' : 'No'}</span>
    `;
    container.appendChild(item);
}

function addImpactDataItem(container, icon, impact, timestamp) {
    const item = document.createElement('div');
    item.className = 'data-item';
    const formattedTimestamp = new Date(timestamp.seconds * 1000).toLocaleString();
    item.innerHTML = `
        ${icon}
        <span>Impact: ${impact ? 'Yes' : 'No'}</span>
        <span>Timestamp: ${formattedTimestamp}</span>
    `;
    container.appendChild(item);
}

function addTimestampItem(container, icon, timestamp) {
    const item = document.createElement('div');
    item.className = 'data-item';
    item.innerHTML = `
        ${icon}
        <span>Timestamp: ${timestamp}</span>
    `;
    container.appendChild(item);
}

function updateHeartRateData(avgbpm) {
    const centerIndex = Math.floor(maxDataPoints / 2);
    heartRateData[centerIndex] = avgbpm;

    // Fill left side with previous data or 0 if no previous data
    for (let i = centerIndex - 1; i >= 0; i--) {
        heartRateData[i] = heartRateData[i + 1] || 0;
    }

    // Fill right side with 0
    for (let i = centerIndex + 1; i < maxDataPoints; i++) {
        heartRateData[i] = 0;
    }

    drawHeartRateChart();
}

function drawHeartRateChart() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // Draw grid lines and labels
    drawGridLinesAndLabels();

    ctx.beginPath();
    ctx.moveTo(0, canvas.height / 2);

    heartRateData.forEach((bpm, index) => {
        const x = (index / maxDataPoints) * canvas.width;
        const y = canvas.height - (bpm / 200) * canvas.height; // Assuming max bpm is 200
        ctx.lineTo(x, y);
    });

    ctx.strokeStyle = 'blue';
    ctx.stroke();

    // Draw upload timestamp
    ctx.font = '16px Arial';
    ctx.fillStyle = 'black';
    ctx.fillText(`Last upload: ${uploadTimestamp}`, 10, 30);
}

function drawGridLinesAndLabels() {
    ctx.strokeStyle = '#ccc';
    ctx.lineWidth = 0.5;
    ctx.font = '12px Arial';
    ctx.fillStyle = '#000';

    // Draw horizontal grid lines and labels
    for (let i = 0; i <= 200; i += 50) {
        const y = canvas.height - (i / 200) * canvas.height;
        ctx.beginPath();
        ctx.moveTo(0, y);
        ctx.lineTo(canvas.width, y);
        ctx.stroke();
        ctx.fillText(i, 5, y - 5);
    }

    // Draw vertical grid lines and labels
    for (let i = 0; i <= 60; i += 10) {
        const x = (i / 60) * canvas.width;
        ctx.beginPath();
        ctx.moveTo(x, 0);
        ctx.lineTo(x, canvas.height);
        ctx.stroke();
        ctx.fillText(i, x - 10, canvas.height - 5);
    }
}

// Fetch latest data initially
fetchLatestData();

// Use requestAnimationFrame for smooth animation
function animate() {
    drawHeartRateChart();
    requestAnimationFrame(animate);
}
requestAnimationFrame(animate);
