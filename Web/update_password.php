<?php
require_once 'config.php';

// Conectar a la base de datos
$conn = new mysqli($servername, $username, $password, $dbname);

// Verificar conexión
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]));
}

// Obtener la contraseña desde la solicitud POST
$nuevaContrasena = $_POST['password'] ?? '';

if (empty($nuevaContrasena)) {
    die(json_encode(["status" => "error", "message" => "Contraseña no proporcionada."]));
}

// Preparar y ejecutar la consulta
$sql = "INSERT INTO admin_password (id, password) VALUES (1, ?) 
        ON DUPLICATE KEY UPDATE password = ?";
$stmt = $conn->prepare($sql);
$stmt->bind_param("ss", $nuevaContrasena, $nuevaContrasena);

if ($stmt->execute()) {
    echo json_encode(["status" => "success", "message" => "Contraseña actualizada correctamente."]);
} else {
    echo json_encode(["status" => "error", "message" => "Error al actualizar la contraseña: " . $stmt->error]);
}

// Cerrar conexión
$stmt->close();
$conn->close();
?>
