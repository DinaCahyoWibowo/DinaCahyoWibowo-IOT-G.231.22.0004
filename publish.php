<?php

use Bluerhinos\phpMQTT;

require("vendor/bluerhinos/phpmqtt/phpMQTT.php");

$server     = 'x2.revolusi-it.com';
$port       = 1883;
$username   = 'usm';
$password   = 'usmjaya001';
$client_id  = 'G.231.22.0004_publish';

$mqtt = new phpMQTT($server, $port, $client_id);

if (!$mqtt->connect(true, NULL, $username, $password)) {
    exit(1);
}

// Validasi data dari POST
$topic = isset($_POST['topic']) ? $_POST['topic'] : null;
$message = isset($_POST['payload']) ? $_POST['payload'] : null;
$payload = json_encode($message);

if ($topic && $message) {
    if ($mqtt->connect(true, NULL, $username, $password)) {
        $mqtt->publish($topic, $message, 0, true);
        $mqtt->close();
        echo "Message published successfully.";
    } else {
        echo "Failed to connect or timed out.";
    }
} else {
    echo "Invalid topic or payload.";
}
