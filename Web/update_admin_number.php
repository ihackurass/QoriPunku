<?php
// Credenciales de la base de datos
require_once 'config.php';

// Conectar a la base de datos
$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]));
}

if (!isset($_POST['admin_number']) || empty($_POST['admin_number'])) {
    die(json_encode(['status' => 'error', 'message' => 'El número del administrador es requerido']));
}

$admin_number = $_POST['admin_number'];

$query = "INSERT INTO admin_config (config_key, config_value) 
          VALUES ('admin_number', ?) 
          ON DUPLICATE KEY UPDATE config_value = VALUES(config_value)";

$stmt = $conn->prepare($query);

if (!$stmt) {
    die(json_encode(['status' => 'error', 'message' => 'Error preparando la consulta: ' . $conn->error]));
}

$stmt->bind_param("s", $admin_number);

if ($stmt->execute()) {
    echo json_encode(['status' => 'success', 'message' => 'Número del administrador actualizado correctamente']);
} else {
    echo json_encode(['status' => 'error', 'message' => 'Error al actualizar el número del administrador: ' . $stmt->error]);
}

$stmt->close();
$conn->close();
?>
