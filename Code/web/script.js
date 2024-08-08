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
const maxDataPoints = 60; // Show last 60 seconds of data

// Initialize flatpickr
flatpickr("#datePicker", {
    dateFormat: "Y-m-d",
    locale: "en",
    onChange: fetchTimestamps
});

async function fetchTimestamps() {
    const datePicker = document.getElementById('datePicker');
    const selectedDate = datePicker.value;

    if (!selectedDate) {
        alert("Please select a date.");
        return;
    }

    const startOfDay = new Date(selectedDate).setHours(0, 0, 0, 0) / 1000;
    const endOfDay = new Date(selectedDate).setHours(23, 59, 59, 999) / 1000;

    const snapshot = await db.collection("Helmet-data-3in1")
        .where("timestamp", ">=", new firebase.firestore.Timestamp(startOfDay, 0))
        .where("timestamp", "<=", new firebase.firestore.Timestamp(endOfDay, 0))
        .orderBy("timestamp", "desc") // Sort by descending order
        .get();

    const timePicker = document.getElementById('timePicker');
    timePicker.innerHTML = ''; // Clear previous options

    if (snapshot.empty) {
        alert("No data available for the selected date.");
        return;
    }

    snapshot.forEach(doc => {
        const timestamp = doc.data().timestamp;
        const option = document.createElement('option');
        option.value = timestamp.seconds;
        option.textContent = new Date(timestamp.seconds * 1000).toLocaleTimeString('en-US');
        timePicker.appendChild(option);
    });

    // Fetch data for the latest timestamp by default
    if (timePicker.options.length > 0) {
        timePicker.selectedIndex = 0;
        fetchDataByTimestamp();
    }
}

async function fetchDataByTimestamp() {
    const timePicker = document.getElementById('timePicker');
    const selectedTimestamp = timePicker.value;

    if (!selectedTimestamp) {
        alert("Please select a timestamp.");
        return;
    }

    const snapshot = await db.collection("Helmet-data-3in1")
        .where("timestamp", "==", new firebase.firestore.Timestamp(parseInt(selectedTimestamp), 0))
        .get();

    if (!snapshot.empty) {
        const data = snapshot.docs[0].data();
        const dataContainer = document.getElementById('dataContainer');
        const timestampCard = document.getElementById('timestamp-card');
        dataContainer.innerHTML = ''; // Clear previous data

        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/test-tube.png', 'C2H5OH', data.C2H5OH, data.C2H5OHalarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/gas.png', 'CH4', data.CH4, data.CH4alarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/poison-bottle.png', 'CO', data.CO, data.COalarm);
        addDataItemSimple(dataContainer, '<i class="fas fa-hard-hat"></i>', 'FSR', data.fsr);

        // Update heart rate data
        updateHeartRateData(data.avgbpm);

        // Re-add the timestamp card at the end
        dataContainer.appendChild(timestampCard);
    }

    // Fetch and display impact data
    fetchAndDisplayImpactData();
}

async function fetchAndDisplayImpactData() {
    const latestImpactSnapshot = await db.collection("Helmet-data-impact")
        .orderBy("timestamp", "desc")
        .limit(10) // Fetch latest 10 impacts
        .get();

    const impactContainer = document.createElement('div');
    impactContainer.className = 'data-item';
    impactContainer.innerHTML = `
        <i class="fas fa-car-crash"></i>
        <span>Impact Data</span>
        <div class="impact-list" id="impactList"></div>
    `;

    const impactList = impactContainer.querySelector('#impactList');

    latestImpactSnapshot.forEach(doc => {
        const impactData = doc.data();
        const formattedTimestamp = new Date(impactData.timestamp.seconds * 1000).toLocaleString('en-US');
        const impactItem = document.createElement('div');
        impactItem.className = 'impact-item';
        impactItem.innerHTML = `
            <span>${formattedTimestamp}</span>
            <span>${impactData.impact ? 'Impact Detected' : 'No'}</span>
        `;
        impactList.appendChild(impactItem);
    });

    // Ensure only one Impact Data card exists
    const existingImpactCard = document.querySelector('.data-item .fas.fa-car-crash');
    if (existingImpactCard) {
        existingImpactCard.parentElement.remove();
    }

    document.getElementById('dataContainer').appendChild(impactContainer);
}

async function fetchLatestData() {
    const latestDataSnapshot = await db.collection("Helmet-data-3in1")
        .orderBy("timestamp", "desc")
        .limit(1)
        .get();

    if (!latestDataSnapshot.empty) {
        const data = latestDataSnapshot.docs[0].data();
        const dataContainer = document.getElementById('dataContainer');
        const timestampCard = document.getElementById('timestamp-card');
        dataContainer.innerHTML = ''; // Clear previous data

        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/test-tube.png', 'C2H5OH', data.C2H5OH, data.C2H5OHalarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/gas.png', 'CH4', data.CH4, data.CH4alarm);
        addDataItem(dataContainer, 'https://img.icons8.com/fluency/48/000000/poison-bottle.png', 'CO', data.CO, data.COalarm);
        addDataItemSimple(dataContainer, '<i class="fas fa-hard-hat"></i>', 'FSR', data.fsr);

        // Update heart rate data
        updateHeartRateData(data.avgbpm);

        // Set the date picker to the latest date and fetch timestamps
        const latestTimestamp = data.timestamp.seconds * 1000;
        const datePicker = document.getElementById('datePicker');
        if (datePicker) {
            datePicker._flatpickr.setDate(new Date(latestTimestamp));
            await fetchTimestamps();
        }

        // Re-add the timestamp card at the end
        dataContainer.appendChild(timestampCard);
    }

    // Fetch and display impact data
    fetchAndDisplayImpactData();
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
    const datePicker = document.getElementById('datePicker');
    const timePicker = document.getElementById('timePicker');
    let selectedTimestamp = '';

    if (datePicker && timePicker && datePicker.value && timePicker.value) {
        const selectedDate = datePicker.value;
        const selectedTime = timePicker.value;
        selectedTimestamp = new Date(selectedTime * 1000).toLocaleString('en-US');
    }

    ctx.font = '16px Arial';
    ctx.fillStyle = 'black';
    ctx.fillText(`Last upload: ${selectedTimestamp}`, 10, 30);
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
