#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LCDI2C_Generic.h>
#include <Keypad.h>
#include <Servo.h>

Servo miServo;  // Crea un objeto servo para controlar un servomotor

// Configuración del teclado matricial
const byte FILAS = 4; 
const byte COLUMNAS = 4; 

char keys[FILAS][COLUMNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte pinesFilas[FILAS] = {6, 7, 8, 9};
byte pinesColumnas[COLUMNAS] = {2, 3, 4, 5};  

Keypad teclado = Keypad(makeKeymap(keys), pinesColumnas, pinesFilas, COLUMNAS, FILAS);

// Configuración del RC522 y LCD
#define SS_PIN 53
#define RST_PIN 49
MFRC522 rfid(SS_PIN, RST_PIN);
LCDI2C_Generic lcd(0x27, 16, 2); // Dirección I2C de la pantalla LCD

// Pines para el sensor ultrasónico
#define TRIG_PIN 11
#define ECHO_PIN 10
// Pin para el sensor infrarrojo
#define IR_SENSOR_PIN 22

#define SERVO_PIN A8

int distanciaSensor = 50;  

String claveIngresada = "";

bool modoConfiguracion = false;

int language = 0;
String messages[2][3] = {
  {"Acceso concedido", "Acceso denegado", "Bienvenido!"},
  {"Access granted", "Access denied", "Welcome!"}
};

void setup() {
  Serial.begin(115200);    // Comunicación con la computadora
  Serial1.begin(115200);   // Comunicación Serial1 con el ESP32-CAM
  SPI.begin();
  rfid.PCD_Init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  miServo.attach(SERVO_PIN);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(IR_SENSOR_PIN, INPUT);
  
  lcd.setCursor(2, 0);
  lcd.print("Bienvenido.");
  lcd.setCursor(0, 1);
  lcd.print("Ingrese IDCard..");
  delay(2000);
}

// ESPERA FOTO + ACCESO
bool esperandoFotoAcceso = false;
unsigned long tiempoInicioEspera;
const unsigned long tiempoEsperaMax = 120000; // 120 segundos

void loop() {
  if (Serial1.available()) {
    String mensajeESP32 = Serial1.readStringUntil('\n');
    if (mensajeESP32.indexOf("Foto: OK") >= 0) {
      Serial.println(mensajeESP32);
      Serial.println("Foto: OK.");
      lcd.setCursor(3, 1);
      lcd.print("Foto OK ");
      esperandoFotoAcceso = true;
      delay(2000);
    }else if (mensajeESP32.indexOf("Autorizando") >= 0) {
      lcd.clear();
      Serial.println("Verificando acceso...");
      lcd.setCursor(1, 0);
      lcd.print("Autorizando...");
      delay(2000);
    }else if (mensajeESP32.indexOf("APermitido") >= 0) {
      Serial.println("Acceso Permitido.");
      lcd.setCursor(1, 1);
      lcd.print("A. Concedido");
      esperandoFotoAcceso = false;
      miServo.write(20);
      delay(10000);
      miServo.write(90);
      lcd.clear();
    } else if (mensajeESP32.indexOf("ADenegado") >= 0) {
      Serial.println("Acceso Denegado.");
      lcd.setCursor(1, 1);
      lcd.print("A. Denegado");
      esperandoFotoAcceso = false;
      delay(2000);
      lcd.clear();
    } else if (mensajeESP32.indexOf("Foto: Error") >= 0) {
      Serial.println("Error al enviar foto.");
      lcd.setCursor(2, 1);
      lcd.print("Error Foto");
      esperandoFotoAcceso = false;
      delay(2000);
      lcd.clear();
    }

    // Opcional: Leer desde el monitor serie y enviar al ESP32-CAM
    if (Serial.available()) {
        String mensajePC = Serial.readStringUntil('\n');
        Serial1.println(mensajePC);  // Envía el mensaje al ESP32-CAM
    }

  }

  verificarModoConfiguracion();

  if (modoConfiguracion) {
    mostrarOpciones();
  } else if (!esperandoFotoAcceso) {
    
    lcd.setCursor(2, 0);
    lcd.print("Bienvenido.");
    lcd.setCursor(0, 1);
    lcd.print("Ingrese IDCard..");

    if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
      return;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Leyendo tarjeta");
    lcd.setCursor(5, 1);
    lcd.print("...");
    delay(3000);

    String readUIDStr = convertUIDToString(rfid.uid.uidByte, rfid.uid.size);

    long distance = getUltrasonicDistance();
    
    if ( verificarTarjetaUUID(readUIDStr) == "I") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(messages[language][1]); // Acceso denegado
      delay(2000);

      // Verifica si hay un objeto
      if (distance < distanciaSensor) { 
        Serial.print("Objeto detectado a ");
        Serial.print(distance);
        Serial.println(" cm");

        // Verifica el sensor infrarrojo
        bool isObjectDetected = digitalRead(IR_SENSOR_PIN) == LOW;
        if (isObjectDetected) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Tomando foto...");
          Serial1.print("1"); // Envía el comando para capturar foto al ESP32-CAM
          
          esperandoFotoAcceso = true;
          tiempoInicioEspera = millis();
        } else {
          lcd.clear();
          lcd.setCursor(2, 0);
          lcd.print("Sensor IR: ");
          lcd.setCursor(1, 1);
          lcd.print("Sin deteccion");
          delay(3000);
          lcd.clear();
        }
      } else {
        lcd.clear();
        lcd.setCursor(2, 0);
        lcd.print("Sensor US: ");
        lcd.setCursor(1, 1);
        lcd.print("Sin deteccion");
        delay(3000);
        lcd.clear();
      }
      
      Serial.print("Acceso denegado -> UID: ");
      Serial.println(readUIDStr); // Imprime el UID en el monitor serie
    } else if (verificarTarjetaUUID(readUIDStr) == "A") {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(messages[language][0]); // Acceso concedido
      miServo.write(20);
      delay(10000);
      miServo.write(90);
      delay(3000);
      lcd.clear();
    }

    //delay(3000);
    //lcd.clear();

    rfid.PICC_HaltA();
  }

  // Verifica si el tiempo de espera ha excedido el límite de 10 segundos
  if (esperandoFotoAcceso && (millis() - tiempoInicioEspera >= tiempoEsperaMax)) {
    Serial.println("FotoAcceso: Tiempo de espera agotado");
    lcd.setCursor(3, 1);
    lcd.print("Sin respuesta");
    delay(3000);
    esperandoFotoAcceso = false;  // Termina la espera después del tiempo límite
    lcd.clear();
  }
}


// ESPERA CONTRASEÑA
unsigned long tiempoInicioUUIDEspera;
const unsigned long tiempoEsperaUUIDMax = 20000; // 20 segundos

String verificarTarjetaUUID(String uuidIngresado) {
    String validarBool = "";
    bool responseUUID = false;

    if (uuidIngresado.length() > 0) {
        //Serial.println("UUID leído: " + uuidIngresado);
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Validando");
        lcd.setCursor(4, 1);
        lcd.print("UUID...");
        delay(2000);

        // Enviar UUID al ESP32-CAM para validación
        Serial1.println("5:" + uuidIngresado);
        unsigned long tiempoInicioUUIDEspera = millis();

        while (millis() - tiempoInicioUUIDEspera < tiempoEsperaUUIDMax) {  // 20 segundos de espera
            if (Serial1.available()) {
                String validarmensajeUUID = Serial1.readStringUntil('\n');
                if (validarmensajeUUID.indexOf("UUID válido") >= 0) {
                    //Serial.println("UUID: OK");
                    responseUUID = true;
                    validarBool = "A";
                    lcd.clear();
                    break;
                } else if (validarmensajeUUID.indexOf("UUID inválido") >= 0) {
                    //Serial.println("UUID inválido");
                    responseUUID = true;
                    validarBool = "I";
                    lcd.clear();
                    break;
                }
            }
        }

        if (!responseUUID && validarBool == "") {
          Serial.println("UUID: Tiempo de espera agotado");
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Error");
          lcd.setCursor(2, 1);
          lcd.print("Sin respuesta");
          delay(3000);
          validarBool = "SR";
          lcd.clear();  // Limpia el LCD al finalizar
        }

    }

    return validarBool;
}

// ESPERA CONTRASEÑA
unsigned long tiempoInicioValidarEspera;
const unsigned long tiempoEsperaValidarMax = 20000; // 20 segundos

void verificarModoConfiguracion() {
    bool responseValidar = false;
    char tecla = teclado.getKey();
    if (tecla) {
      claveIngresada += tecla;   
      if (claveIngresada.length() == 5) {
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print("Verificando...");
        delay(2000);
        Serial1.println("4:"+claveIngresada);
        tiempoInicioValidarEspera = millis();
        while (millis() - tiempoInicioValidarEspera < tiempoEsperaValidarMax) {
            if (Serial1.available()) {
                String validarmensajeContrasena = Serial1.readStringUntil('\n');
                if (validarmensajeContrasena.indexOf("ValidarPassword: OK") >= 0) {
                    Serial.println("Validar Password OK");
                    modoConfiguracion = true;
                    responseValidar = true;
                    lcd.clear();
                    lcd.setCursor(4, 0);
                    lcd.print("Config");
                    lcd.setCursor(5, 1);
                    lcd.print("Mode");
                    delay(2000);
                    claveIngresada = "";
                    break;
                } else if (validarmensajeContrasena.indexOf("Contraseña inválida") >= 0) {
                    Serial.println("Validar Password Invalido");
                    lcd.clear();
                    responseValidar = true;
                    lcd.setCursor(5, 0);
                    lcd.print("Clave");
                    lcd.setCursor(3, 1);
                    lcd.print("Incorrecta");
                    delay(2000);
                    claveIngresada = "";
                    break;
                }
            }
        }

        if (!responseValidar) {
          Serial.println("Error Validar Pass");
          lcd.clear();
          lcd.setCursor(5, 0);
          lcd.print("Error");
          lcd.setCursor(3, 1);
          lcd.print("Respuesta");
          delay(2000);
          claveIngresada = "";
          lcd.clear();
        }

      }
    } 
}

// ESPERA CONTRASEÑA

unsigned long tiempoInicioContrasenaEspera;
const unsigned long tiempoEsperaContrasenaMax = 20000; // 20 segundos

void cambiarContrasena() {
  bool esperandoContrasena = false;
  bool responseContrasena = false;
  lcd.clear();
  lcd.print("Nueva Pass:");
  String nuevaContrasena = "";
  while (true) {
    if (Serial1.available()) {
      String mensajeContrasena = Serial1.readStringUntil('\n');
      if (mensajeContrasena.indexOf("Password: OK") >= 0) {
        Serial.println("Contraseña Actualizada Correctamente");
        lcd.setCursor(1, 0);
        lcd.print("P. Actualizado");
        esperandoContrasena = false;
        responseContrasena = true;
        delay(2000);
        lcd.clear();
      } else if (mensajeContrasena.indexOf("Password: Error") >= 0) {
        Serial.println("Error Actualizacion Pass");
        lcd.setCursor(1, 1);
        lcd.print("Pass Error");
        esperandoContrasena = false;
        responseContrasena = true;
        delay(2000);
        lcd.clear();
      }
    }

    if (!esperandoContrasena) {
      if (responseContrasena) {
        responseContrasena = false;
        return;
      }
      char tecla = teclado.getKey();
      if (tecla) {
        if (tecla == '#') {  // Confirmar nueva contraseña
          if (nuevaContrasena.length() == 5) {
            lcd.clear();
            lcd.print("Confirmar Pass:");
            String confirmarContrasena = leerContrasena();
            if (nuevaContrasena == confirmarContrasena) {
              lcd.clear();
              lcd.setCursor(1, 0);
              lcd.print("Actualizando...");
              Serial1.print("3:" + nuevaContrasena);  // Comando para actualizar contraseña
              tiempoInicioContrasenaEspera = millis();
              esperandoContrasena = true;
            } else {
              lcd.clear();
              lcd.print("No coinciden!");
              delay(2000);
              lcd.clear();
              lcd.print("Nueva Pass:");
              nuevaContrasena = "";  // Reiniciar si no coincide
            }
          } else {
            lcd.clear();
            lcd.print("Nueva Pass:");
            nuevaContrasena = "";  // Reiniciar si menos de 5 caracteres
          }
        } else if (tecla == '*') {  // Borrar último dígito
          if (nuevaContrasena.length() > 0) {
            nuevaContrasena.remove(nuevaContrasena.length() - 1);
            lcd.setCursor(0, 1);
            lcd.print(nuevaContrasena + " ");
          }
        } else if (tecla == 'C') {  // Cancelar operación
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Cancelado");
          delay(2000);
          return;
        } else if (tecla >= '0' && tecla <= '9') {  // Solo aceptar dígitos
          if (nuevaContrasena.length() < 9) {  // Limitar longitud máxima de la contraseña
            nuevaContrasena += tecla;
            lcd.setCursor(0, 1);
            lcd.print(nuevaContrasena);
          }
        }
      }
    } else {
      if (esperandoContrasena && (millis() - tiempoInicioContrasenaEspera >= tiempoEsperaContrasenaMax)) {
        Serial.println("Pass: Tiempo de espera agotado");
        lcd.setCursor(1, 1);
        lcd.print("Sin respuesta");
        delay(3000);
        esperandoContrasena = false;  // Termina la espera después del tiempo límite
        responseContrasena = true;
        lcd.clear();
      }
    }
  }
}

String leerContrasena() {
  String contrasena = "";
  while (true) {
    char tecla = teclado.getKey();
    if (tecla) {
      if (tecla == '#') {
        return contrasena;
      } else if (tecla == '*') {
        if (contrasena.length() > 0) {
          contrasena.remove(contrasena.length() - 1);
          lcd.setCursor(0, 1);
          lcd.print(contrasena + " ");
        }
      } else if (tecla >= '0' && tecla <= '9') {
        if (contrasena.length() < 9) {  // Limitar longitud máxima de la contraseña
          contrasena += tecla;
          lcd.setCursor(0, 1);
          lcd.print(contrasena);
        }
      }
    }
  }
}

const int numOpciones = 4;
String opcionesMenu[] = {
  "1: Cambiar Num",
  "2: Distancia",
  "3: Cambiar Pass",
  "4: Salir"
};

int opcionSeleccionada = 0;

void mostrarOpciones() {
  lcd.clear();
  mostrarOpcionActual();  // Muestra las opciones iniciales

  while (true) {
    char tecla = teclado.getKey();

    if (tecla) {
      if (tecla == 'A') {  // Tecla para subir
        if (opcionSeleccionada > 0) {
          opcionSeleccionada--;
          mostrarOpcionActual();
        }
      } else if (tecla == 'B') {  // Tecla para bajar
        if (opcionSeleccionada < numOpciones - 1) {
          opcionSeleccionada++;
          mostrarOpcionActual();
        }
      } else if (tecla >= '1' && tecla <= '4') {  // Selección directa por número
        int opcion = tecla - '1';  // Convierte char a int
        ejecutarOpcion(opcion);
        break;
      }
    }
  }
}

void mostrarOpcionActual() {
  lcd.clear();
  lcd.print(opcionesMenu[opcionSeleccionada]);
  if (opcionSeleccionada < numOpciones - 1) {
    lcd.setCursor(0, 1);
    lcd.print(opcionesMenu[opcionSeleccionada + 1]);
  }
}

void ejecutarOpcion(int opcion) {
  lcd.clear();
  switch (opcion) {
    case 0:
      cambiarNumeroTelefono();
      break;
    case 1:
      cambiarDistanciaSensor();
      break;
    case 2:
      // Acción para la opción tres
      cambiarContrasena();
      break;
    case 3:
      modoConfiguracion = false;
      lcd.print("Saliendo...");
      delay(2000);
      lcd.clear();
      break;
  }
}

// ESPERA NUMERO

unsigned long tiempoInicioNumeroEspera;
const unsigned long tiempoEsperaNumeroMax = 20000; // 20 segundos

// Función para cambiar el número de teléfono
void cambiarNumeroTelefono() {
  bool esperandoNumero = false;
  bool responseNumero = false;
  lcd.clear();
  lcd.print("Nuevo Numero:");
  String numeroTelefono = "";  
  while (true) {
    if (Serial1.available()) {
        String mensajeNumero = Serial1.readStringUntil('\n');  // Asegura leer hasta el fin de línea
        if (mensajeNumero.indexOf("Actualización: OK") >= 0) {
          Serial.println("Numero Actualizado Correctamente");
          lcd.setCursor(1, 1);
          lcd.print("N. Actualizado");
          esperandoNumero = false;
          responseNumero = true;
          delay(2000);
          lcd.clear();
        }
        else if (mensajeNumero.indexOf("Actualización: OK") >= 0) {
          Serial.println("Error Actualizacion Numero");
          lcd.setCursor(2, 1);
          lcd.print("N. Error");
          esperandoNumero = false;
          responseNumero = true;
          delay(2000);
          lcd.clear();
        }
    }

  // Opcional: Enviar datos desde la computadora al ESP32-CAM
    if (Serial.available()) {
      String mensajePC = Serial.readString();
      Serial1.println(mensajePC);      // Envía el mensaje de la computadora al ESP32-CAM
    }

    if (!esperandoNumero){
      if (responseNumero){
        responseNumero = false;
        return;
      }
      char tecla = teclado.getKey();
      if (tecla) {
        if (tecla == '#') {  // # para confirmar
          if (numeroTelefono.length() == 9) {
            lcd.clear();
            lcd.setCursor(1, 0);
            lcd.print("Actualizando...");
            Serial1.print("2:"+numeroTelefono);
            tiempoInicioNumeroEspera = millis();
            esperandoNumero = true;
          } else {
            // Si el número no tiene 9 dígitos, mostrar error
            lcd.clear();
            lcd.print("Numero invalido");
            delay(2000);
            lcd.clear();
            lcd.print("Nuevo Numero:");
            numeroTelefono = "";  // Reinicia la entrada del número
          }
        } else if (tecla == '*') {  // * para borrar el último dígito
          if (numeroTelefono.length() > 0) {
            numeroTelefono.remove(numeroTelefono.length() - 1);
            lcd.setCursor(0, 1);
            lcd.print(numeroTelefono + " ");  // Limpia el dígito borrado en el LCD
          }
        } else if (tecla == 'C') {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Cancelado");
          delay(2000);
          numeroTelefono = "";
          return;
        } else if (tecla >= '0' && tecla <= '9') {  // Solo acepta dígitos
          if (numeroTelefono.length() < 9) {
            numeroTelefono += tecla;  
            lcd.setCursor(0, 1);
            lcd.print(numeroTelefono);
          }
        }
      }
    }else{
      if (esperandoNumero && (millis() - tiempoInicioNumeroEspera >= tiempoEsperaNumeroMax)) {
        Serial.println("Numero: Tiempo de espera agotado");
        lcd.setCursor(2, 1);
        lcd.print("Sin respuesta");
        delay(3000);
        esperandoNumero = false;  // Termina la espera después del tiempo límite
        responseNumero = true;
        lcd.clear();
      }
    }
  }
}

// Función para cambiar la distancia del sensor
void cambiarDistanciaSensor() {
  lcd.clear();
  lcd.print("Nueva Dist:");

  String nuevaDistancia = "";
  while (true) {
    char tecla = teclado.getKey();
    if (tecla) {
      Serial.print("Tecla para distancia: ");
      Serial.println(tecla); // Imprime la tecla presionada en el monitor serie

      if (tecla == 'C') {  // Tecla para salir
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("Cancelado");
        delay(2000);
        return;
      } else if (tecla == '#') {  // Confirmar
        if (nuevaDistancia.length() > 0) {
          distanciaSensor = nuevaDistancia.toInt();  
          lcd.clear();
          lcd.setCursor(4, 0);
          lcd.print("Distancia");
          lcd.setCursor(4, 1);
          lcd.print("Cambiada");
          delay(2000);
          lcd.clear();
          return;
        } else {
          lcd.clear();
          lcd.print("Valor invalido");
          delay(2000);
          lcd.clear();
          lcd.print("Nueva Dist:");
          nuevaDistancia = "";  // Reinicia la entrada
        }
      } else if (tecla == '*') {  // Borrar
        if (nuevaDistancia.length() > 0) {
          nuevaDistancia.remove(nuevaDistancia.length() - 1);
          lcd.setCursor(0, 1);
          lcd.print(nuevaDistancia + " ");  // Limpia el dígito borrado en el LCD
        }
      } else if (tecla >= '0' && tecla <= '9') {  // Solo acepta dígitos
        if (nuevaDistancia.length() < 3) {  // Limitar a 3 dígitos para una distancia razonable
          nuevaDistancia += tecla;  
          lcd.setCursor(0, 1);
          lcd.print(nuevaDistancia);
        }
      }
    }
  }
  lcd.clear();
}


// Función para convertir el UID a una cadena en formato hexadecimal
String convertUIDToString(byte *uid, byte uidSize) {
  String uidStr = "";
  for (byte i = 0; i < uidSize; i++) {
    if (uid[i] < 0x10) uidStr += "0"; // Añadir un '0' si el byte es menor de 16 para mantener formato hexadecimal
    uidStr += String(uid[i], HEX);     // Convertir byte a HEX y concatenar
  }
  uidStr.toUpperCase(); // Convertir a mayúsculas (opcional)
  return uidStr;
}


// Función para obtener la distancia con el sensor ultrasónico
long getUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  long distance = duration * 0.034 / 2; // Convertir a centímetros
  return distance;
}
