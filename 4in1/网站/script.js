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

async function fetchData() {
    const dataContainer = document.getElementById('dataContainer');
    dataContainer.innerHTML = ''; // Clear previous data

    const heartRateSnapshot = await db.collection("Helmet-data")
        .orderBy("timestamp", "desc")
        .limit(1)
        .get();
    heartRateSnapshot.forEach((doc) => {
        const data = doc.data();
        const heartRateItem = document.createElement('div');
        heartRateItem.className = 'data-item';
        heartRateItem.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/heart-with-pulse.png" alt="Heart Rate Icon">
            <span>Heart Rate: ${data.Heartrate} bpm</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(heartRateItem);
    });

    const environmentSnapshot = await db.collection("Helmet-data-3in1")
        .orderBy("timestamp", "desc")
        .limit(1)
        .get();
    environmentSnapshot.forEach((doc) => {
        const data = doc.data();

        const c2h5ohItem = document.createElement('div');
        c2h5ohItem.className = 'data-item';
        c2h5ohItem.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/chemistry.png" alt="C2H5OH Icon">
            <span>C2H5OH: ${data.C2H5OH}</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(c2h5ohItem);

        const ch4Item = document.createElement('div');
        ch4Item.className = 'data-item';
        ch4Item.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/methane.png" alt="CH4 Icon">
            <span>CH4: ${data.CH4}</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(ch4Item);

        const coItem = document.createElement('div');
        coItem.className = 'data-item';
        coItem.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/co.png" alt="CO Icon">
            <span>CO: ${data.CO}</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(coItem);

        const fsrItem = document.createElement('div');
        fsrItem.className = 'data-item';
        fsrItem.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/helmet.png" alt="Helmet Icon">
            <span>FSR: ${data.fsr}</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(fsrItem);

        const impactItem = document.createElement('div');
        impactItem.className = 'data-item';
        impactItem.innerHTML = `
            <img src="https://img.icons8.com/color/48/000000/crash.png" alt="Impact Icon">
            <span>Impact: ${data.impact}</span>
            <span>${new Date(data.timestamp.seconds * 1000).toLocaleString()}</span>
        `;
        dataContainer.appendChild(impactItem);
    });
}

// Fetch latest data initially and set interval to update every minute
fetchData();
setInterval(fetchData, 60000); // Update every minute
