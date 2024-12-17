<?php
header('Content-Type: application/json'); // Set header untuk JSON response

require("vendor/bluerhinos/phpmqtt/phpMQTT.php"); // Pastikan path sesuai dengan lokasi file

// Konfigurasi MQTT
$host = "x2.revolusi-it.com"; // Broker MQTT
$port = 1883; // Port MQTT
$topik = "iot/kendali/0406";
$topic_suhu = "iot/suhu/0406"; // Topik untuk suhu
$clientID = "ClientID" . rand(); // ID unik untuk client MQTT
$username = "usm";   // Your MQTT username
$password = "usmjaya001";   // Your MQTT password
// Database configuration
$host_db = "localhost"; // Host database
$user_db = "root";      // Username database
$pass_db = "";          // Password database
$db_name = "mqtt_data"; // Nama database

// Create a database connection
$conn = new mysqli($host_db, $user_db, $pass_db, $db_name);
if ($conn->connect_error) {
    die(json_encode([
        'status' => 'error',
        'message' => 'Database connection failed: ' . $conn->connect_error,
    ]));
}

// Variabel untuk menyimpan data suhu
$receivedSuhuData = false;
$suhu = null;
$kelembaban = null;

// Mengecek apakah permintaan valid
$type = $_GET['type'] ?? ''; // Mendapatkan tipe dari parameter GET

if ($type === 'suhu') {
    $mqtt = new Bluerhinos\phpMQTT($host, $port, $clientID);

    if ($mqtt->connect(true, NULL, $username, $password)) {
        // Publish permintaan suhu ke topik suhu
        $mqtt->publish($topic_suhu, 0, 0);

        // Subscribe ke topik suhu dan menangani callback
        $mqtt->subscribe([$topic_suhu => ['qos' => 0, 'function' => 'procCallback']], 0);

        // Waktu timeout
        $timeout = 10; // Dalam detik
        $startTime = time();

        // Loop untuk memproses pesan yang diterima
        while (time() - $startTime < $timeout) {
            $mqtt->proc();
            if ($receivedSuhuData) {
                break;
            }
            usleep(100000); // Delay 100ms untuk mengurangi beban loop
        }

        // Jika tidak ada data yang diterima dalam waktu timeout
        if (!$receivedSuhuData) {
            echo json_encode([
                'status' => 'error',
                'message' => 'Timeout: Data suhu tidak diterima.'
            ]);
        }

        $mqtt->close();
    } else {
        echo json_encode([
            'status' => 'error',
            'message' => 'Gagal menghubungkan ke broker MQTT.'
        ]);
    }
} else {
    echo json_encode([
        'status' => 'error',
        'message' => 'Tipe permintaan tidak valid.'
    ]);
}

// Fungsi callback untuk menangani pesan dari MQTT
function procCallback($topic, $msg) {
    global $receivedSuhuData, $suhu, $kelembaban, $conn;

    if (strpos($topic, 'iot/suhu') !== false) {
        // Misalkan format pesan adalah "Suhu: XX.XXC, Kelembaban: YY.YY%"
        if (preg_match('/Suhu: (\d+\.\d+)C, Kelembaban: (\d+\.\d+)%/', $msg, $matches)) {
            $suhu = $matches[1];
            $kelembaban = $matches[2];

            // Insert the data into the database
            $stmt = $conn->prepare("INSERT INTO sensor_data (suhu, kelembaban) VALUES (?, ?)");
            $stmt->bind_param("dd", $suhu, $kelembaban);

            if ($stmt->execute()) {
                $data = [
                    'status' => 'success',
                    'message' => "Suhu: {$suhu}C, Kelembaban: {$kelembaban}%",
                    'db_status' => 'Data inserted successfully',
                ];
            } else {
                $data = [
                    'status' => 'success',
                    'message' => "Suhu: {$suhu}C, Kelembaban: {$kelembaban}%",
                    'db_status' => 'Failed to insert data: ' . $stmt->error,
                ];
            }

            $stmt->close();
            echo json_encode($data);
            $receivedSuhuData = true; // Tandai bahwa data telah diterima
        }
    }
}

$conn->close(); // Close the database connection
?>
