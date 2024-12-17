<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>MQTT Temperature and Humidity Graph</title>
    <style>
        .container {
            max-width: 600px;
            margin: 0 auto;
            padding: 20px;
            font-family: Arial, sans-serif;
        }
        .sensor-display {
            text-align: center;
            margin-bottom: 20px;
        }
        .sensor-display p {
            font-size: 1.2em;
        }
        .icon {
            font-size: 1.5em;
            margin-right: 10px;
        }
    </style>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        let tempData = [];
        let humData = [];
        let labels = [];

        function updateGraph(temp, hum) {
            const now = new Date().toLocaleTimeString();
            labels.push(now);
            tempData.push(temp);
            humData.push(hum);

            if (labels.length > 20) {
                labels.shift();
                tempData.shift();
                humData.shift();
            }

            tempHumidityChart.update();
        }

        function fetchSensorData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    try {
                        var response = JSON.parse(this.responseText);
                        if (response.status === "success") {
                            var message = response.message;
                            var suhu = parseFloat(message.match(/Suhu: (\d+\.\d+)C/)[1]);
                            var kelembaban = parseFloat(message.match(/Kelembaban: (\d+\.\d+)%/)[1]);

                            updateGraph(suhu, kelembaban);

                            document.getElementById("temp").innerText = suhu;
                            document.getElementById("humidity").innerText = kelembaban;
                        }
                    } catch (e) {
                        console.error("Error parsing JSON response: ", e);
                    }
                }
            };
            xhttp.open("GET", "kirim.php?type=suhu", true);
            xhttp.send();
        }

        let ctx = null;
        let tempHumidityChart = null;
        document.addEventListener("DOMContentLoaded", () => {
            ctx = document.getElementById("tempHumidityGraph").getContext("2d");
            tempHumidityChart = new Chart(ctx, {
                type: 'line',
                data: {
                    labels: labels,
                    datasets: [
                        {
                            label: 'Temperature (°C)',
                            data: tempData,
                            borderColor: '#ff6384',
                            backgroundColor: 'rgba(255, 99, 132, 0.2)',
                            tension: 0.1,
                        },
                        {
                            label: 'Humidity (%)',
                            data: humData,
                            borderColor: '#36a2eb',
                            backgroundColor: 'rgba(54, 162, 235, 0.2)',
                            tension: 0.1,
                        }
                    ]
                },
                options: {
                    scales: {
                        x: {
                            title: {
                                display: true,
                                text: 'Time',
                            },
                        },
                        y: {
                            beginAtZero: true,
                            title: {
                                display: true,
                                text: 'Value',
                            },
                        },
                    },
                    plugins: {
                        legend: {
                            display: true,
                        },
                    },
                },
            });

            setInterval(fetchSensorData, 5000);
        });
    </script>
</head>
<body>
    <div class="container">
        <h1>Monitoring Suhu dan Kelembapan</h1>

        <div id="sensorData" class="sensor-display">
            <p><span class="icon">🌡️</span>Temperatur: <span id="temp">Memuat...</span> °C</p>
            <p><span class="icon">💧</span>Kelembapan: <span id="humidity">Memuat...</span> %</p>
        </div>

        <canvas id="tempHumidityGraph" width="400" height="200"></canvas>
    </div>
</body>
</html>
