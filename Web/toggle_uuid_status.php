<?php
header('Content-Type: application/json');

require_once 'config.php';

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    echo json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]);
    exit();
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
    $newStatus = ($row['status'] === 'denied') ? 'allowed' : 'denied';

    $updateStmt = $conn->prepare("UPDATE allowed_uuids SET status = ? WHERE uuid = ?");
    $updateStmt->bind_param("ss", $newStatus, $uuid);
    if ($updateStmt->execute()) {
        echo json_encode(["status" => "success", "new_status" => $newStatus]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error al actualizar el estado."]);
    }
    $updateStmt->close();
} else {
    echo json_encode(["status" => "error", "message" => "UUID no encontrado."]);
}

$stmt->close();
$conn->close();
?>
