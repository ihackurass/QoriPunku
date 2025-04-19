<?php
header('Content-Type: application/json');

// Configuración de la base de datos
require_once 'config.php';

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    echo json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]);
    exit();
}

$sql = "SELECT uuid, status, created_at FROM allowed_uuids ORDER BY created_at DESC";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $uuids = [];
    while ($row = $result->fetch_assoc()) {
        $uuids[] = [
            "uuid" => $row["uuid"],
            "status" => $row["status"],
            "created_at" => $row["created_at"]
        ];
    }
    echo json_encode(["status" => "success", "uuids" => $uuids]);
} else {
    echo json_encode(["status" => "success", "uuids" => []]); // Devuelve un array vacío si no hay resultados
}

// Cerrar conexión
$conn->close();
?>
