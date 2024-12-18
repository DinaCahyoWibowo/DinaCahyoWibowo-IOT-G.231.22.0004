<?php

class DB
{
    public $servername = "localhost";
    public $username = "root";
    public $password = "";
    public $dbname = "mqtt_data";
    public $conn;

    public function __construct()
    {
        $this->conn = new mysqli(
            $this->servername,
            $this->username,
            $this->password,
            $this->dbname
        );

        if ($this->conn->connect_error) {
            die("Connection failed: " . $this->conn->connect_error);
        }
    }

    public function insert($temperature, $humidity)
    {
        date_default_timezone_set('Asia/Jakarta');
        $timestamp = date('Y-m-d H:i:s');
        if ($temperature !== null && $humidity !== null && $timestamp !== null) {
            $sql = "INSERT INTO dht_data (temperature, humidity, created_at) VALUES (?, ?, ?)";

            $stmt = $this->conn->prepare($sql);

            if ($stmt === false) {
                die("Error preparing statement: " . $this->conn->error);
            }

            $stmt->bind_param("dds", $temperature, $humidity, $timestamp);

            if ($stmt->execute()) {
                return "New record created successfully!";
            } else {
                return "Error: " . $stmt->error;
            }

            $stmt->close();
        } else {
            return "Missing parameters in the URL!";
        }
    }

    public function getSensorsData()
    {
        $sql = "SELECT * FROM dht_data ORDER BY id DESC LIMIT 10";
        $result = $this->conn->query($sql);

        if ($result === false) {
            return "Error: " . $this->conn->error;
        }

        $data = [];
        while ($row = $result->fetch_assoc()) {
            $data[] = $row;
        }

        return $data;
    }
}
