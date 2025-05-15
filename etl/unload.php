<?php
// unload.php – returns leaderboard data as JSON
require_once 'config.php'; // provides $pdo
header('Content-Type: application/json');

$period = $_GET['period'] ?? 'today';
$where  = '';
switch ($period) {
    case 'week':
        $where = 'WHERE YEARWEEK(timestamp, 1) = YEARWEEK(NOW(), 1)';
        break;
    case 'all':
        $where = ''; // no filter
        break;
    default: // today
        $where = 'WHERE DATE(timestamp) = CURDATE()';
}

$sql = "SELECT device_id, SUM(score) AS total FROM scores $where GROUP BY device_id ORDER BY total DESC";

try {
    $stmt = $pdo->query($sql);
    $rows = $stmt->fetchAll(PDO::FETCH_ASSOC);
    // add rank field
    $rank = 1;
    foreach ($rows as &$row) {
        $row['rank'] = $rank++;
    }
    echo json_encode($rows);
} catch (PDOException $e) {
    http_response_code(500);
    echo json_encode(['error' => $e->getMessage()]);
}
