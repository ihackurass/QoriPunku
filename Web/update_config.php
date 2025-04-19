<?php
require_once 'config.php';

// Conectar a la base de datos
$conn = new mysqli($servername, $username, $password, $dbname);

// Verificar conexión
if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]));
}

$response = ["status" => "error", "message" => "Operación no válida"];
$password = $_POST['password'] ?? '';
$numero = $_POST['numero'] ?? '';

if ($password) {
    if (preg_match('/^\d{5}$/', $password)) {  // Verificar que sea un número de 5 dígitos
        $stmt = $conn->prepare("UPDATE admin_password SET password = ? WHERE id = 1");
        $stmt->bind_param("s", $password);
        if ($stmt->execute()) {
            $response = ["status" => "success", "message" => "Contraseña actualizada exitosamente"];
        } else {
            $response = ["status" => "error", "message" => "Error al actualizar contraseña"];
        }
        $stmt->close();
    } else {
        $response = ["status" => "error", "message" => "La contraseña debe tener exactamente 5 dígitos"];
    }
} elseif ($numero) {
    if (preg_match('/^\d{9}$/', $numero)) {  // Verificar que sea un número de 9 dígitos
        $stmt = $conn->prepare("UPDATE admin_config SET config_value = ? WHERE config_key = 'admin_number'");
        $stmt->bind_param("s", $numero);
        if ($stmt->execute()) {
            $response = ["status" => "success", "message" => "Número de teléfono actualizado exitosamente"];
        } else {
            $response = ["status" => "error", "message" => "Error al actualizar número de teléfono"];
        }
        $stmt->close();
    } else {
        $response = ["status" => "error", "message" => "El número debe tener exactamente 9 dígitos"];
    }
}

$conn->close();
echo json_encode($response);
?>
