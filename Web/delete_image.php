<?php
$directory = 'captured_images/';  // Especifica el directorio de las imágenes

// Obtener el nombre de la imagen desde el POST
$image = $_POST['image'] ?? '';

if (!$image) {
    echo json_encode(["status" => "error", "message" => "Imagen no proporcionada."]);
    exit();
}

$imagePath = $directory . basename($image);  // Obtener el nombre del archivo en el directorio

if (file_exists($imagePath)) {
    if (unlink($imagePath)) {
        echo json_encode(["status" => "success", "message" => "Imagen eliminada con éxito."]);
    } else {
        echo json_encode(["status" => "error", "message" => "Error al eliminar la imagen."]);
    }
} else {
    echo json_encode(["status" => "error", "message" => "Imagen no encontrada."]);
}
