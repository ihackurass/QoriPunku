<?php
require_once 'config.php';

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]));
}

$uuid = $_POST['uuid'] ?? '';

if (!$uuid) {
    echo json_encode(["status" => "error", "message" => "UUID no proporcionado."]);
    exit();
}

$stmt = $conn->prepare("SELECT status FROM allowed_uuids WHERE uuid = ?");
$stmt->bind_param("s", $uuid);
$stmt->execute();
$result = $stmt->get_result();

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $updateStmt = $conn->prepare("UPDATE allowed_uuids SET created_at = CURRENT_TIMESTAMP WHERE uuid = ?");
    $updateStmt->bind_param("s", $uuid);
    $updateStmt->execute();
    $updateStmt->close();
    if ($row['status'] === 'allowed') {
        echo json_encode(["status" => "valid"]);
    } else {
        echo json_encode(["status" => "invalid"]);
    }
} else {
    $insertStmt = $conn->prepare("INSERT INTO allowed_uuids (uuid, status, created_at) VALUES (?, 'denied', CURRENT_TIMESTAMP)");
    $insertStmt->bind_param("s", $uuid);
    if ($insertStmt->execute()) {
        echo json_encode(["status" => "invalid"]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error al insertar UUID: " . $conn->error]);
    }
    $insertStmt->close();
}

// Cerrar conexión
$stmt->close();
$conn->close();
?>
