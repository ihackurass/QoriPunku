<?php
require_once 'config.php';

$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Error de conexión: " . $conn->connect_error]));
}

$contrasenaIngresada = $_POST['password'] ?? '';

if (empty($contrasenaIngresada)) {
    die(json_encode(["status" => "error", "message" => "Contraseña no proporcionada."]));
}

$sql = "SELECT password FROM admin_password WHERE id = 1";
$result = $conn->query($sql);

if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    $contrasenaAlmacenada = $row['password'];

    if ($contrasenaIngresada === $contrasenaAlmacenada) {
        echo json_encode(["status" => "valid", "message" => "Contraseña válida."]);
    } else {
        echo json_encode(["status" => "invalid", "message" => "Contraseña inválida."]);
    }
} else {
    echo json_encode(["status" => "error", "message" => "No se encontró la contraseña."]);
}

// Cerrar conexión
$conn->close();
?>
