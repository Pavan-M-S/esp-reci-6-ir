#include <WiFi.h>
#include <FirebaseESP32.h>

// Firebase credentials
#define FIREBASE_PROJECT_API_KEY "AIzaSyDAtbzOawekOm8xMliLVjuwJ-U_N9vfRm8"
#define FIREBASE_DATABASE_URL "https://new-one-eef62-default-rtdb.firebaseio.com/"
#define FIREBASE_EMAIL "esp32@espres.com"
#define FIREBASE_PASSWORD "Pavan#310512"

// Wi-Fi credentials
#define WIFI_SSID "AKCNB-VIIT-A"
#define WIFI_PASSWORD "AKCNB@1234"

// Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Traffic variables
float R1, R1T, R2, R2T, R3, R3T, R4, R4T, Mode;

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400, SERIAL_8N1, 22, 21); // GPIO22=TX, GPIO21=RX

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

  // Firebase setup
  config.api_key = FIREBASE_PROJECT_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;
  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Ready to receive from PC or Nano...");
  Firebase.setFloat(fbdo, "/Mode", 0);
}

void loop() {
  // Emergency trigger from Firebase
  if (Firebase.getFloat(fbdo, "/Set") == 2) {
    for (int i = 0; i < 35; i++) {
      Serial1.println("Emergency");
      delay(300);
    }
    Firebase.setFloat(fbdo, "/Set", 0);
  }

  // Read from Nano
  if (Serial1.available()) {
    String input = Serial1.readStringUntil('\n');
    input.trim();
    Serial.println(input);

    int start = 0;
    while (start < input.length()) {
      int sep = input.indexOf(';', start);
      if (sep == -1) sep = input.length();

      String pair = input.substring(start, sep);
      int colon = pair.indexOf(':');
      if (colon != -1) {
        String key = pair.substring(0, colon);
        String value = pair.substring(colon + 1);
        float val = value.toFloat();

        if (key == "R1") R1 = val;
        else if (key == "R1T") R1T = val;
        else if (key == "R2") R2 = val;
        else if (key == "R2T") R2T = val;
        else if (key == "R3") R3 = val;
        else if (key == "R3T") R3T = val;
        else if (key == "R4") R4 = val;
        else if (key == "R4T") R4T = val;
        else if (key == "Mode") Mode = val;
      }

      start = sep + 1;
    }

    // Debug print
    Serial.println("------------------------");
    Serial.printf("Road 1 = %.2f | Timer = %.2f\n", R1, R1T);
    Serial.printf("Road 2 = %.2f | Timer = %.2f\n", R2, R2T);
    Serial.printf("Road 3 = %.2f | Timer = %.2f\n", R3, R3T);
    Serial.printf("Road 4 = %.2f | Timer = %.2f\n", R4, R4T);
    Serial.printf("Mode = %.2f\n", Mode);
    Serial.println("------------------------");

    // Send to Firebase using JSON
    if (Firebase.ready()) {
      FirebaseJson json;
      json.set("/R1C", R1);
      json.set("/R1T", R1T);
      json.set("/R2C", R2);
      json.set("/R2T", R2T);
      json.set("/R3C", R3);
      json.set("/R3T", R3T);
      json.set("/R4C", R4);
      json.set("/R4T", R4T);
      json.set("/Mode", Mode);

      if (Firebase.set(fbdo, "/", json)) {
        Serial.println("Batch sent to Firebase!");
      } else {
        Serial.println("Failed to send data to Firebase");
        Serial.println(fbdo.errorReason());
      }
    }
  }else {
    delay(2500);
    FirebaseJson json;
      json.set("/R1C", 0);
      json.set("/R1T", 0);
      json.set("/R2C", 0);
      json.set("/R2T", 0);
      json.set("/R3C", 0);
      json.set("/R3T", 0);
      json.set("/R4C", 0);
      json.set("/R4T", 0);
      json.set("/Mode", 1);
  }
}