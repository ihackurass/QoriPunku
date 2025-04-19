//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> ESP32_CAM_Send_Photo_to_Server
//======================================== Including the libraries.
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_camera.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
//======================================== 

//======================================== CAMERA_MODEL_AI_THINKER GPIO.
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
//======================================== 

// LED Flash PIN (GPIO 4)
#define FLASH_LED_PIN 4             

//========================================
const char* ssid = "MI_WIFI 2.4G"; // NOMBRE DE LA RED WIFI
const char* password = "MI_CLAVE"; // PASSWORD
//======================================== 

String serverName = "tu-dominio.com";
const int serverPort = 80;

// Variable to set capture photo with LED Flash.
// Set to "false", then the Flash LED will not light up when capturing a photo.
// Set to "true", then the Flash LED lights up when capturing a photo.
bool LED_Flash_ON = true;

WiFiClient client;


bool validarUUID(const String& uuidIngresado) {
    int maxAttempts = 3;
    int attempt = 0;
    bool success = false;

    while (attempt < maxAttempts && !success) {
        attempt++;
        Serial.println("Intento " + String(attempt) + " de validar el UUID...");

        HTTPClient http;
        http.begin("http://" + serverName + "/CA/validate_uuid_list.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String postData = "uuid=" + uuidIngresado;
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);

            StaticJsonDocument<200> doc;
            DeserializationError error = deserializeJson(doc, response);

            if (error) {
                Serial.println("Error al parsear JSON: ");
                return false;
            }

            const char* status = doc["status"];
            if (String(status) == "valid") {
                Serial.println("UUID válido.");
                success = true;
            } else {
                Serial.println("UUID inválido.");
                return false;
            }
        } else {
            Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
        }

        http.end();

        if (!success && attempt < maxAttempts) {
            Serial.println("Reintentando en 2 segundos...");
            delay(2000);  // Espera 2 segundos antes de reintentar
        }
    }

    return success;
}


bool validarContrasena(const String& contrasenaIngresada) {
    int maxAttempts = 3;
    int attempt = 0;
    bool success = false;

    while (attempt < maxAttempts && !success) {
        attempt++;
        Serial.println("Intento " + String(attempt) + " de validar la contraseña...");

        HTTPClient http;
        http.begin("http://" + serverName + "/CA/validate_password.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String postData = "password=" + contrasenaIngresada;
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);

            if (response.indexOf("\"status\":\"valid\"") >= 0) {
                Serial.println("Contraseña válida.");
                success = true;
            } else {
                Serial.println("Contraseña inválida.");
                return false;
            }
        } else {
            Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
        }

        http.end();

        if (!success && attempt < maxAttempts) {
            Serial.println("Reintentando en 2 segundos...");
            delay(2000);  // Espera 2 segundos antes de reintentar
        }
    }

    return success;
}

bool updatePassword(const String& nuevaContrasena) {
    int maxAttempts = 3;
    int attempt = 0;
    bool success = false;

    while (attempt < maxAttempts && !success) {
        attempt++;
        Serial.println("Intento " + String(attempt) + " de actualizar la contraseña...");

        HTTPClient http;
        http.begin("http://" + serverName + "/CA/update_password.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String postData = "password=" + nuevaContrasena;
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);

            if (response.indexOf("\"status\":\"success\"") >= 0) {
                Serial.println("Contraseña actualizada correctamente.");
                success = true;
            } else {
                Serial.println("Error en la actualización de la contraseña.");
            }
        } else {
            Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
        }

        http.end();

        if (!success && attempt < maxAttempts) {
            Serial.println("Reintentando en 2 segundos...");
            delay(2000);
        }
    }

    return success;
}


bool updateAdminNumber(const String& adminNumber) {
    int maxAttempts = 3;
    int attempt = 0;
    bool success = false;

    while (attempt < maxAttempts && !success) {
        attempt++;
        Serial.println("Intento " + String(attempt) + " de actualizar el número del administrador...");

        HTTPClient http;
        http.begin("http://" + serverName + "/CA/update_admin_number.php");
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");

        String postData = "admin_number=" + adminNumber;
        int httpResponseCode = http.POST(postData);

        if (httpResponseCode > 0) {
            String response = http.getString();
            Serial.println("Respuesta del servidor: " + response);

            if (response.indexOf("\"status\":\"success\"") >= 0) {
                Serial.println("Número del administrador actualizado correctamente.");
                success = true;
            } else {
                Serial.println("Error en la actualización del número del administrador.");
            }
        } else {
            Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
        }

        http.end();

        if (!success && attempt < maxAttempts) {
            Serial.println("Reintentando en 2 segundos...");
            delay(2000);  // Espera 2 segundos antes de reintentar
        }
    }

    return success;
}


bool checkAccessStatus(String photoId) {
  if (photoId.isEmpty()) {
    Serial.println("Photo ID está vacío. No se puede consultar el estado.");
    return false;
  }

  Serial.println("Consultando el estado del acceso para Photo ID: " + photoId);

  unsigned long startTime = millis();
  const unsigned long timeout = 60000;  // Tiempo de espera máximo de 20 segundos
  bool accessApproved = false;

  HTTPClient http;
  http.begin("http://" + serverName + "/CA/check_access.php");
  http.addHeader("Content-Type", "application/json");

  String requestData = "{\"photo_id\":\"" + photoId + "\"}";

  while ((millis() - startTime) < timeout) {
    int httpResponseCode = http.POST(requestData);

    if (httpResponseCode > 0) {
      String response = http.getString();
      //Serial.println("Respuesta del servidor: " + response);

      StaticJsonDocument<256> doc;
      DeserializationError error = deserializeJson(doc, response);

      if (error) {
        Serial.println("Error al parsear JSON: " + String(error.c_str()));
      } else {
        String status = doc["access_status"] | "error";

        if (status == "approved") {
          Serial.println("Acceso aprobado.");
          accessApproved = true;
          break;
        } else if (status == "denied") {
          Serial.println("Acceso denegado.");
          break;
        } else {
          Serial.println("Estado del acceso: " + status);
        }
      }
    } else {
      Serial.println("Error en la solicitud HTTP: " + String(httpResponseCode));
    }

    delay(2000);  // Esperar 2 segundos antes de la siguiente verificación
  }

  http.end();
  return accessApproved;
}

String getServerResponse(WiFiClient& client) {
  String response = "";
  long startTimer = millis();
  const int timeoutTimer = 10000;  // Tiempo de espera de 10 segundos

  // Leer respuesta completa con timeout
  while ((startTimer + timeoutTimer) > millis()) {
    while (client.available()) {
      response += (char)client.read();
    }
  }

  // Separar encabezados del cuerpo
  int bodyIndex = response.indexOf("\r\n\r\n");
  String body;
  if (bodyIndex != -1) {
    body = response.substring(bodyIndex + 4);  // +4 para omitir "\r\n\r\n"

    int jsonStart = body.indexOf('{');
    int jsonEnd = body.lastIndexOf('}');

    if (jsonStart != -1 && jsonEnd != -1 && jsonEnd > jsonStart) {
      return body.substring(jsonStart, jsonEnd + 1);
    } else {
      Serial.println("No se pudo encontrar JSON válido en el cuerpo.");
      return "";  // Salir de la función si no se encuentra JSON válido
    }
  } else {
    Serial.println("No se pudieron encontrar los encabezados.");
    body = response;  // Si no se encuentran los encabezados, trabaja con la respuesta completa
  }
  return body;

}


int maxIntentos = 2; // Máximo de intentos

RTC_DATA_ATTR int intentos = 0;  // Almacena el número de intentos

String photoId;

// Función para enviar foto al servidor
bool sendPhotoToServer() {
  bool envioExitoso = false;

  while (intentos < maxIntentos && !envioExitoso) {
    intentos++;
    Serial.print("Intento ");
    Serial.print(intentos);
    Serial.println(" de captura y envío de foto...");

    String AllData;
    String DataBody;

    Serial.println("-----------");
    Serial.println("Tomando una foto...");

    // Capturas preliminares para ajuste de exposición
    for (int i = 0; i < 3; i++) { 
      camera_fb_t *prelim_fb = esp_camera_fb_get();
      if (prelim_fb) {
        esp_camera_fb_return(prelim_fb); // Libera el buffer de la imagen preliminar
      }
      delay(100); // Da tiempo para que la cámara ajuste la exposición
    }

    // Captura final con flash (si está activado)
    if (LED_Flash_ON) {
      digitalWrite(FLASH_LED_PIN, HIGH);
      delay(1000);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Error al capturar la foto");
      Serial.println("Reiniciando ESP32 CAM");
      delay(1000);  // Pausa antes de reiniciar
      ESP.restart();
      return false;
    }

    if (LED_Flash_ON) digitalWrite(FLASH_LED_PIN, LOW);
    
    Serial.println("Foto capturada exitosamente.");
    Serial.println("Conectando al servidor: " + serverName);

    // Conectar al servidor con reintentos
    int retryCount = 0;
    const int maxRetries = 5;
    while (!client.connect(serverName.c_str(), serverPort) && retryCount < maxRetries) {
      Serial.println("Conexión fallida, reintentando...");
      retryCount++;
      delay(2000);
    }

    if (retryCount == maxRetries) {
      DataBody = "Conexión a " + serverName + " fallida.";
      Serial.println(DataBody);
      Serial.println("-----------");
    } else {
      Serial.println("¡Conexión exitosa!");

      String post_data = "--dataMarker\r\nContent-Disposition: form-data; name=\"imageFile\"; filename=\"ESP32CAMCap.jpg\"\r\nContent-Type: image/jpeg\r\n\r\n";
      String head = post_data;
      String boundary = "\r\n--dataMarker--\r\n";
      
      uint32_t imageLen = fb->len;
      uint32_t dataLen = head.length() + boundary.length();
      uint32_t totalLen = imageLen + dataLen;
      
      client.println("POST /CA/upload_img.php HTTP/1.1");
      client.println("Host: " + serverName);
      client.println("Content-Length: " + String(totalLen));
      client.println("Content-Type: multipart/form-data; boundary=dataMarker");
      client.println();
      client.print(head);
    
      uint8_t *fbBuf = fb->buf;
      size_t fbLen = fb->len;
      for (size_t n = 0; n < fbLen; n += 1024) {
        if (n + 1024 < fbLen) {
          client.write(fbBuf, 1024);
          fbBuf += 1024;
        } else if (fbLen % 1024 > 0) {
          size_t remainder = fbLen % 1024;
          client.write(fbBuf, remainder);
        }
      }
      client.print(boundary);
      
      esp_camera_fb_return(fb); // Libera el buffer de la imagen actual

      // Confirmación de respuesta del servidor
      String response = getServerResponse(client);

    if (response.length() == 0) {
      Serial.println("No se recibió respuesta del cuerpo del servidor.");
      return false;
    }

      //Serial.println("Respuesta del servidor: " + response);
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, response);
      if (error) {
        Serial.println("Error al parsear JSON: " + String(error.c_str()));
        continue;
      }

      photoId = doc["photo_id"] | "";
      String status = doc["status"] | "error";

      if (status == "success" && !photoId.isEmpty()) {
        envioExitoso = true;
        Serial.println("Foto enviada exitosamente. Photo ID: " + photoId);
      } else {
        Serial.println("Error en el envío:");
      }
    }
  }

  if (!envioExitoso) {
    Serial.println("Error: No se pudo enviar la foto después de " + String(maxIntentos) + " intentos.");
    intentos = 0;
  } else {
    intentos = 0;  // Reiniciar el contador si fue exitoso
  }

  return envioExitoso;
}


void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Deshabilitar el detector de bajo voltaje
  Serial.begin(115200);
  pinMode(FLASH_LED_PIN, OUTPUT);

  // Conectar a WiFi
  WiFi.mode(WIFI_STA);
  Serial.print("Conectando a WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int timeout = 20000;
  unsigned long startAttemptTime = millis();
  
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    Serial.print(".");
    delay(500);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println("Error de conexión, reiniciando...");
    ESP.restart();
  }

  Serial.println();
  Serial.println("Conectado a WiFi");
  
  // Configuración de la cámara
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Error al inicializar la cámara");
    ESP.restart();
  }

  Serial.println("Cámara inicializada correctamente.");
}

void loop() {
  // Verifica si hay datos en el puerto serial
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    
    // Si el valor es "1", toma y envía una foto
    if (input == "1") {
      //Serial.println("Recibido '1' en Serial. Capturando y enviando foto...");
      if (sendPhotoToServer()) {
        Serial.println("Foto: OK");
        Serial.println("");
        Serial.println("Autorizando");
        if(checkAccessStatus(photoId)){
          Serial.println("APermitido");
        }else{
          Serial.println("ADenegado");
        }
      } else {
        if (intentos >= maxIntentos){
          Serial.println("Foto: Error");
        }
      }

      delay(5000);  // Espera antes del próximo intento
    }
    else if (input.startsWith("2:")) {
      String adminNumber = input.substring(String("2:").length());
      adminNumber.trim();  // Remueve cualquier espacio o nueva línea
      Serial.println("Recibido número de administrador: " + adminNumber);

      if (updateAdminNumber(adminNumber)) {
        Serial.println("Actualización: OK");
      } else {
        Serial.println("Actualización: Error");
      }
    }else if (input.startsWith("3:")) {
      String password = input.substring(String("3:").length());
      password.trim();  // Remueve cualquier espacio o nueva línea
      Serial.println("Recibido Contrasena: " + password);

      if (updatePassword(password)) {
        Serial.println("Password: OK");
      } else {
        Serial.println("Password: Error");
      }
    }
    else if (input.startsWith("4:")) {
      String password = input.substring(String("4:").length());
      password.trim();  // Remueve cualquier espacio o nueva línea
      Serial.println("Recibido Validar: " + password);

      if (validarContrasena(password)) {
        Serial.println("ValidarPassword: OK");
      } else {
        Serial.println("ValidarPassword: Error");
      }
    }
    else if (input.startsWith("5:")) {
      String uuid = input.substring(String("5:").length());
      uuid.trim();  // Remueve cualquier espacio o nueva línea
      Serial.println("Recibido UUID: " + uuid);

      if (validarUUID(uuid)) {
        Serial.println("UUID: OK");
      } else {
        Serial.println("UUID: Error");
      }
    }
  }
}