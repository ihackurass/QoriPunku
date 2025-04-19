<?php

// Conectar a la base de datos
require_once 'config.php';

$conn = new mysqli($servername, $username, $password, $dbname);

// Verificar la conexión
if ($conn->connect_error) {
    echo json_encode(['status' => 'error', 'message' => 'Fallo en la conexión a la base de datos.']);
    exit();
}

// Verificar si la solicitud es POST y contiene el photo_id
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $input = file_get_contents('php://input');
    $data = json_decode($input, true);

    $photo_id = $data['photo_id'] ?? '';

    if (empty($photo_id)) {
        echo json_encode(['status' => 'error', 'message' => 'photo_id es obligatorio.']);
        exit();
    }

    // Consultar el estado en la base de datos
    $stmt = $conn->prepare("SELECT status FROM uploaded_images WHERE photo_id = ?");
    $stmt->bind_param("s", $photo_id);

    if ($stmt->execute()) {
        $result = $stmt->get_result();
        if ($result->num_rows > 0) {
            $row = $result->fetch_assoc();
            echo json_encode(['status' => 'success', 'photo_id' => $photo_id, 'access_status' => $row['status']]);
        } else {
            echo json_encode(['status' => 'error', 'message' => 'photo_id no encontrado.']);
        }
    } else {
        echo json_encode(['status' => 'error', 'message' => 'Error al consultar la base de datos.']);
    }

    $stmt->close();
} else {
    echo json_encode(['status' => 'error', 'message' => 'Método no soportado.']);
}

$conn->close();
?>