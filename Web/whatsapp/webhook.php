<?php
if ($_SERVER['REQUEST_METHOD'] == 'POST') {
    $input = file_get_contents('php://input');
    parse_str($input, $data);

    // Guardar toda la respuesta en un archivo de registro
    $logFile = 'webhook_full_response.txt';
    file_put_contents($logFile, "Webhook Received:\n" . print_r($data, true) . "\n", FILE_APPEND | LOCK_EX);

    $message_body = strtolower(trim($data['Body'] ?? ''));
    $msid_user_response = $data['MessageSid'] ?? '';
    $from = $data['From'] ?? '';  // Número del administrador

    file_put_contents($logFile, "\n" . $from . "\n", FILE_APPEND | LOCK_EX);
    file_put_contents($logFile, "\n" . $msid_user_response . "\n", FILE_APPEND | LOCK_EX);
    file_put_contents($logFile, "\n" . $message_body . "\n", FILE_APPEND | LOCK_EX);
    // Determinar el estado basado en la respuesta
    $status = ($message_body === 'si') ? 'approved' : (($message_body === 'no') ? 'denied' : 'unknown');

    // Conectar a la base de datos
    $servername = "localhost";
    $username = "root";
    $password = "";
    $dbname = "mi_bd";

    $conn = new mysqli($servername, $username, $password, $dbname);
    if ($conn->connect_error) {
        error_log("Fallo en la conexión a la base de datos.");
        echo json_encode(['status' => 'error', 'message' => 'Database connection failed']);
        exit();
    }

    // Buscar el último photo_id pendiente para este administrador
    $stmt = $conn->prepare("
        SELECT photo_id 
        FROM uploaded_images 
        WHERE status = 'pending' AND from_number = ? 
        ORDER BY created_at DESC LIMIT 1
    ");
    $stmt->bind_param("s", $from);
    $stmt->execute();
    $stmt->bind_result($photo_id);
    $stmt->fetch();
    $stmt->close();

    if ($photo_id) {
        // Actualizar el estado en la base de datos
        $stmt = $conn->prepare("UPDATE uploaded_images SET status = ?, msid_user_response = ? WHERE photo_id = ?");
        $stmt->bind_param("sss", $status, $msid_user_response, $photo_id);

        if ($stmt->execute()) {
            $response = [
                'status' => 'success',
                'photo_id' => $photo_id,
                'new_status' => $status,
                'message_sid' => $msid_user_response
            ];
            file_put_contents($logFile, "Database Update Successful:\n" . print_r($response, true) . "\n", FILE_APPEND | LOCK_EX);
        } else {
            $response = ['status' => 'error', 'message' => 'Failed to update status'];
            file_put_contents($logFile, "Database Update Failed:\n" . $stmt->error . "\n", FILE_APPEND | LOCK_EX);
        }

        $stmt->close();
    } else {
        $response = ['status' => 'error', 'message' => 'No pending photo_id found for this admin'];
        file_put_contents($logFile, "No Pending photo_id Found:\n" . print_r($response, true) . "\n", FILE_APPEND | LOCK_EX);
    }

    $conn->close();
    header('Content-Type: application/json');
    echo json_encode($response);

} else {
    header('Content-Type: application/json');
    echo json_encode(['status' => 'error', 'message' => 'Unsupported request method']);
}
?>
