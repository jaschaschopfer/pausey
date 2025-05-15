<?php
// load.php - receive JSON from ESP32 and insert into MySQL 'scores' table

// pull in database connection from config.php
require_once 'config.php';  // provides $pdo (PDO instance)

// Read raw POST JSON
$raw = file_get_contents('php://input');
$data = json_decode($raw, true);

header('Content-Type: application/json');

if (!is_array($data) || !isset($data['device_id'], $data['game_type'], $data['score'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid or missing fields']);
    exit;
}

try {
    $stmt = $pdo->prepare("INSERT INTO scores (device_id, game_type, score) VALUES (?, ?, ?)");
    $stmt->execute([
        $data['device_id'],
        $data['game_type'],
        $data['score'],
    ]);
    echo json_encode(['status' => 'ok']);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['error' => $e->getMessage()]);
}
