<?php
$directory = 'captured_images/';
$protocol = isset($_SERVER['HTTPS']) && $_SERVER['HTTPS'] === 'on' ? "https://" : "http://";
$domain = $_SERVER['HTTP_HOST'];
$baseUrl = $protocol . $domain . '/CA/captured_images/';

// Escanea el directorio
$files = array_diff(scandir($directory), array('.', '..'));
$images = [];

foreach ($files as $file) {
    if (is_file($directory . $file) && preg_match('/\.(jpg|jpeg|png|gif)$/i', $file)) {
        $images[] = $baseUrl . $file;
    }
}

header('Content-Type: application/json');
echo json_encode(["status" => "success", "images" => $images]);