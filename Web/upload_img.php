<?php
require 'vendor/autoload.php'; // Asegúrate de que Twilio PHP SDK esté instalado

use Twilio\Rest\Client;

date_default_timezone_set('America/Lima');

require_once 'config.php';

function getAdminNumber($defaultNumber) {
    global $servername, $username, $password, $dbname;

    $conn = new mysqli($servername, $username, $password, $dbname);

    if ($conn->connect_error) {
        file_put_contents('upload_log.txt', "Fallo en la conexión a la base de datos para obtener el número del administrador.\n", FILE_APPEND | LOCK_EX);
        return $defaultNumber;
    }

    $stmt = $conn->prepare("SELECT config_value FROM admin_config WHERE config_key = 'admin_number'");
    $stmt->execute();
    $stmt->bind_result($admin_number);
    $stmt->fetch();
    $stmt->close();
    $conn->close();

    return !empty($admin_number) ? $admin_number : $defaultNumber;
}

$target_dir = "captured_images/";
$date = new DateTime();
$date_string = $date->format('Y-m-d_His_');
$target_file = $target_dir . $date_string . basename($_FILES["imageFile"]["name"]);
$uploadOk = 1;
$imageFileType = strtolower(pathinfo($target_file, PATHINFO_EXTENSION));
$file_name = pathinfo($target_file, PATHINFO_BASENAME);

// Comprobar si el archivo es una imagen
if(isset($_FILES["imageFile"])) {  
    $check = getimagesize($_FILES["imageFile"]["tmp_name"]);
    if($check !== false) {
        $uploadOk = 1;
    } else {
        $uploadOk = 0;
    }
}

if (file_exists($target_file)) {
    $uploadOk = 0;
}

if ($_FILES["imageFile"]["size"] > 500000) {
    $uploadOk = 0;
}

if($imageFileType != "jpg" && $imageFileType != "png" && $imageFileType != "jpeg" && $imageFileType != "gif" ) {
    $uploadOk = 0;
}

// Subir archivo
if ($uploadOk == 0) {
    echo json_encode(['status' => 'error', 'message' => 'File upload failed']);
} else {
    if (move_uploaded_file($_FILES["imageFile"]["tmp_name"], $target_file)) {
        $photo_id = uniqid();
        // Configurar Twilio
        $admin_number = getAdminNumber($defaultAdminNumber);
        $to_whatsapp_number = 'whatsapp:+51' . $admin_number;
        
        $protocol = isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https://" : "http://";
        $domain = $_SERVER['HTTP_HOST'];
        
        $media_url = $protocol . $domain . '/CA/' . $target_file;

        $client = new Client($account_sid, $auth_token);

        // Enviar mensaje con la foto
        $message = $client->messages->create(
            $to_whatsapp_number,
            [
                'from' => $from_whatsapp_number,
                'body' => "🔒 *Control de Acceso*\n\nHemos detectado un intento de acceso. Responde con 'Sí' o 'No'.",
                'mediaUrl' => [$media_url]
            ]
        );

        $msid = $message->sid;

        $conn = new mysqli($servername, $username, $password, $dbname);

        if ($conn->connect_error) {
            file_put_contents('upload_log.txt', "Fallo en la conexión a la base de datos.\n", FILE_APPEND | LOCK_EX);
            exit();
        }

        // Insertar detalles en la base de datos
        $stmt = $conn->prepare("INSERT INTO uploaded_images (photo_id, msid, status, from_number) VALUES (?, ?, 'pending', ?)");
        $stmt->bind_param("sss", $photo_id, $msid, $to_whatsapp_number);

        if ($stmt->execute()) {
            file_put_contents('upload_log.txt', "Foto subida con éxito. photo_id: $photo_id, msid: $msid\n", FILE_APPEND | LOCK_EX);
            echo json_encode(['status' => 'success', 'photo_id' => $photo_id, 'message' => 'File uploaded and message sent']);
        } else {
            file_put_contents('upload_log.txt', "Error al insertar en la base de datos: " . $stmt->error . "\n", FILE_APPEND | LOCK_EX);
            echo json_encode(['status' => 'error', 'message' => 'Database insertion failed']);
        }

        $stmt->close();
        $conn->close();
    } else {
        echo json_encode(['status' => 'error', 'message' => 'File upload process failed']);
    }
}
?>
