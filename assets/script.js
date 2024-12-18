const ledStates = {};
    const chart = new Chart(
      document.getElementById("tempHumidityChart").getContext("2d"), {
        type: "line",
        data: {
          labels: [],
          datasets: [{
              label: "Temperature (°C)",
              data: [],
              borderColor: "rgba(255, 99, 132, 1)",
              backgroundColor: "rgba(255, 99, 132, 0.2)",
              borderWidth: 2,
              fill: true,
            },
            {
              label: "Humidity (%)",
              data: [],
              borderColor: "rgba(54, 162, 235, 1)",
              backgroundColor: "rgba(54, 162, 235, 0.2)",
              borderWidth: 2,
              fill: true,
            },
          ],
        },
        options: {
          responsive: true,
          plugins: {
            legend: {
              display: true,
              position: "top",
            },
            tooltip: {
              enabled: true,
            },
          },
          scales: {
            x: {
              title: {
                display: true,
                text: "Time",
              },
            },
            y: {
              title: {
                display: true,
                text: "Value",
              },
              beginAtZero: true,
            },
          },
        },
      }
    );

    // done
    function publishMessage(topic, payload) {
      $.ajax({
        url: "publish.php",
        type: "POST",
        data: {
          topic: topic,
          payload: payload
        },
        success: function(response) {
          console.log("Message published successfully:", response);
        },
        error: function(xhr, status, error) {
          console.error("Error publishing message:", error);
        }
      });
    }

    function toggleLED(ledNumber) {
      const ledButton = document.getElementById(`led${ledNumber}`);
      const isOn = ledButton.classList.contains('btn-success');
      const newState = isOn ? 0 : 1;

      // Update UI
      ledButton.classList.toggle('btn-success', !isOn);
      ledButton.classList.toggle('btn-danger', isOn);
      ledButton.textContent = `LED ${ledNumber} ${newState === 1 ? 'ON' : 'OFF'}`;

      // Update status lampu dalam objek
      ledStates[`led${ledNumber}`] = newState;

      // Buat payload JSON
      const payload = JSON.stringify({
        leds: ledStates,
      });


      // Send the message to the server to control the LEDs
      publishMessage("g231220004/control", payload);
    }


    async function fetchSensorData() {
      const response = await fetch('subscribe.php');
      const data = await response.json();
      return data;
    }

    async function displaySensorData() {
      const data = await fetchSensorData();
      handleTemperature(data.temperature);
      handleHumidity(data.humidity);
      setHumd(data.humidity);
      setTemp(data.temperature);
      updateChart(data.sensors_data);
      updateTable(data.sensors_data);
      const leds = data.control?.leds || {};
      for (let [key, value] of Object.entries(leds)) {
        const ledNumber = key.replace("led", ""); // Ambil nomor LED dari key
        if (value === 1) {
          ledOn(ledNumber); // Fungsi untuk menghidupkan LED di UI
        } else if (value === 0) {
          ledOff(ledNumber); // Fungsi untuk mematikan LED di UI
        }
      }
    }

    function ledOn(ledNumber) {
      const ledButton = document.getElementById(`led${ledNumber}`);
      ledButton.classList.add('btn-success');
      ledButton.classList.remove('btn-danger');
      ledButton.textContent = `LED ${ledNumber} ON`;
    }

    function ledOff(ledNumber) {
      const ledButton = document.getElementById(`led${ledNumber}`);
      ledButton.classList.add('btn-danger');
      ledButton.classList.remove('btn-success');
      ledButton.textContent = `LED ${ledNumber} OFF`;
    }



    function handleTemperature(temperature) {
      temperature = parseFloat(temperature);
      if (temperature < 29) {
        turnLedUiOff(1);
        turnLedUiOff(2);

      } else if (temperature >= 29 && temperature < 30) {
        turnLedUiOn(1);
        turnLedUiOff(2);
        document.getElementById("beep1").play();
      } else if (temperature >= 30 && temperature <= 31) {
        turnLedUiOff(1);
        turnLedUiOn(2);
        document.getElementById("beep2").play();
      } else {
        turnLedUiOn(1);
        turnLedUiOn(2);
        document.getElementById("beep3").play();
      }
    }

    function handleHumidity(humidity) {
      humidity = parseFloat(humidity);
      let humidityLevel;
      if (humidity >= 30 && humidity < 60) {
        humidityLevel = 'Kering/Aman'
      } else if (humidity >= 60 && humidity < 70) {
        humidityLevel = 'Mulai banyak uap air/Normal'
      } else if (humidity >= 70) {
        humidityLevel = 'Terdapat banyak uap air'
      }
      document.getElementById('humidity-level').textContent = humidityLevel
    }

    function turnLedUiOn(led) {
      document.getElementById("led-" + led).classList.add("led-on");
    }

    function turnLedUiOff(led) {
      document.getElementById("led-" + led).classList.remove("led-on");
    }

    function setTemp(value) {
      document.getElementById("temp-value").innerHTML = value + " °C";
    }

    function setHumd(value) {
      document.getElementById("humd-value").innerHTML = value + " %";
    }

    document.addEventListener("DOMContentLoaded", function() {
      const playButton = document.getElementById("playAudioButton");
      
      const modal = new bootstrap.Modal(document.getElementById("audioModal"));
      modal.show();

      playButton.addEventListener("click", () => {
        setInterval(displaySensorData, 10000);
        displaySensorData();
        modal.hide();
      });
    });

    const updateChart = (sensorsData) => {
      const sensorsDataChart = {
        temperatures: sensorsData.map((sensor) => sensor.temperature),
        humidities: sensorsData.map((sensor) => sensor.humidity),
        timestamps: sensorsData.map((sensor) => sensor.created_at),
      };

       // Reverse the data once to maintain consistency
      sensorsDataChart.temperatures.reverse();
      sensorsDataChart.humidities.reverse();
      sensorsDataChart.timestamps.reverse();
      
      chart.data.labels = sensorsDataChart.timestamps;
      chart.data.datasets[0].data = sensorsDataChart.temperatures;
      chart.data.datasets[1].data = sensorsDataChart.humidities;
      chart.update();
    };

    const updateTable = (sensorsData) => {
      const tableBody = document.getElementById("table-body");
      const tableRows = sensorsData
        .map((data, index) => {
          return `
        <tr>
          <td>${data["id"]}</td>
          <td>${data["temperature"]}</td>
          <td>${data["humidity"]}</td>
          <td>${data["created_at"]}</td>
        </tr>`;
        })
        .join("\n");
      tableBody.innerHTML = tableRows;
    };