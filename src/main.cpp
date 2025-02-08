#include <Arduino.h>
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "mbedtls/md.h"

String hashPin(const String &pin)
{
  byte shaResult[32];
  mbedtls_md_context_t ctx;
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 0);
  mbedtls_md_starts(&ctx);
  mbedtls_md_update(&ctx, (const unsigned char *)pin.c_str(), pin.length());
  mbedtls_md_finish(&ctx, shaResult);
  mbedtls_md_free(&ctx);

  String hashString = "";
  for (int i = 0; i < sizeof(shaResult); i++)
  {
    char str[3];
    sprintf(str, "%02x", (int)shaResult[i]);
    hashString += str;
  }
  return hashString;
}

bool isPinSet()
{
  return SPIFFS.exists("/pin.txt");
}

void createPin(const String &pin)
{
  String hashedPin = hashPin(pin);
  File file = SPIFFS.open("/pin.txt", "w");
  if (!file)
    return;
  file.print(hashedPin);
  file.close();
}

bool verifyPin(const String &pin)
{
  File file = SPIFFS.open("/pin.txt", "r");
  if (!file)
    return false;
  String storedHash = file.readString();
  file.close();
  return storedHash == hashPin(pin);
}

bool isPasswordExists(const String &name)
{
  if (!SPIFFS.exists("/passwords.json"))
    return false;
  File file = SPIFFS.open("/passwords.json", "r");
  if (!file)
    return false;
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  if (error)
    return false;
  return doc.containsKey(name);
}

void savePassword(const String &name, const String &password)
{
  const int MAX_NAME_LENGTH = 32;
  const int MAX_PASSWORD_LENGTH = 8000;

  if (name.length() > MAX_NAME_LENGTH)
  {
    Serial.println("\n✗ Error: Password name too long (max 32 characters)");
    return;
  }

  if (password.length() > MAX_PASSWORD_LENGTH)
  {
    Serial.println("\n✗ Error: Password too long (max 8000 characters)");
    return;
  }

  if (isPasswordExists(name))
  {
    Serial.println("\n✗ Error: Password name already exists!");
    return;
  }

  DynamicJsonDocument doc(131072);
  if (SPIFFS.exists("/passwords.json"))
  {
    File file = SPIFFS.open("/passwords.json", "r");
    if (file)
    {
      DeserializationError error = deserializeJson(doc, file);
      file.close();
      if (error)
      {
        Serial.println("\n✗ Error: Failed to read file");
        return;
      }
    }
  }

  JsonObject obj = doc.as<JsonObject>();
  size_t passwordCount = obj.size();

  if (passwordCount >= 1000)
  {
    Serial.println("\n✗ Error: Maximum number of passwords (1000) reached");
    return;
  }

  doc[name] = password;

  String output;
  serializeJson(doc, output);
  if (output.length() > 102400)
  {
    Serial.println("\n✗ Error: Storage full");
    return;
  }

  File file = SPIFFS.open("/passwords.json", "w");
  if (!file)
  {
    Serial.println("\n✗ Error: Failed to create file");
    return;
  }
  if (serializeJson(doc, file) == 0)
  {
    Serial.println("\n✗ Error: Failed to write file");
  }
  file.close();
}

String getPasswordFromStorage(const String &name)
{
  File file = SPIFFS.open("/passwords.json", "r");
  if (!file)
  {
    Serial.println("\n✗ No passwords found");
    return "";
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error)
  {
    Serial.println("\n✗ Error: Failed to read file");
    return "";
  }

  if (!doc.containsKey(name))
    return "";
  return doc[name].as<String>();
}

bool deletePassword(const String &name)
{
  if (!SPIFFS.exists("/passwords.json"))
    return false;

  File file = SPIFFS.open("/passwords.json", "r");
  if (!file)
    return false;

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error)
    return false;

  if (!doc.containsKey(name))
    return false;

  doc.remove(name);

  file = SPIFFS.open("/passwords.json", "w");
  if (!file)
    return false;

  if (serializeJson(doc, file) == 0)
  {
    file.close();
    return false;
  }

  file.close();
  return true;
}

void listPasswords()
{
  if (!SPIFFS.exists("/passwords.json"))
  {
    Serial.println("\n[Empty List]");
    return;
  }

  File file = SPIFFS.open("/passwords.json", "r");
  if (!file)
  {
    Serial.println("\n✗ Error: Could not open password file");
    return;
  }

  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();

  if (error)
  {
    Serial.println("\n✗ Error: Could not read passwords");
    return;
  }

  Serial.println("\n=== Stored Passwords ===");
  JsonObject obj = doc.as<JsonObject>();
  int count = 0;
  for (JsonPair pair : obj)
  {
    count++;
    Serial.print(count);
    Serial.print(". ");
    Serial.println(pair.key().c_str());
  }

  if (count == 0)
  {
    Serial.println("(No passwords stored)");
  }
  Serial.println("========================");
}

void showStorageInfo()
{
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  size_t freeBytes = totalBytes - usedBytes;

  int passwordCount = 0;
  size_t totalPasswordLength = 0;
  float averagePasswordLength = 0;

  if (SPIFFS.exists("/passwords.json"))
  {
    File file = SPIFFS.open("/passwords.json", "r");
    if (file)
    {
      DynamicJsonDocument doc(131072);
      DeserializationError error = deserializeJson(doc, file);
      file.close();

      if (!error)
      {
        JsonObject obj = doc.as<JsonObject>();
        for (JsonPair pair : obj)
        {
          passwordCount++;
          totalPasswordLength += String(pair.value().as<const char *>()).length();
        }
        if (passwordCount > 0)
        {
          averagePasswordLength = (float)totalPasswordLength / passwordCount;
        }
      }
    }
  }

  Serial.println("\n=== Storage Information ===");
  Serial.print("Total Storage: ");
  Serial.print(totalBytes);
  Serial.println(" bytes");
  Serial.print("Used Storage: ");
  Serial.print(usedBytes);
  Serial.println(" bytes");
  Serial.print("Free Storage: ");
  Serial.print(freeBytes);
  Serial.println(" bytes");
  Serial.print("Storage Usage: ");
  Serial.print((float)usedBytes / totalBytes * 100);
  Serial.println("%");
  Serial.println("\n=== Password Statistics ===");
  Serial.print("Total Passwords: ");
  Serial.println(passwordCount);
  Serial.print("Total Characters: ");
  Serial.println(totalPasswordLength);
  Serial.print("Average Length: ");
  Serial.println(averagePasswordLength);
  Serial.println("=========================");
}

void showWelcomeMessage()
{
  Serial.println("\n====================================");
  Serial.println("   ESP32Pass - Password Manager");
  Serial.println("====================================");
}

void showCommands()
{
  Serial.println("\nAvailable commands:");
  Serial.println("- create : Create new password");
  Serial.println("- get    : Retrieve password");
  Serial.println("- delete : Remove password");
  Serial.println("- list   : Show all passwords");
  Serial.println("- info   : System information");
  Serial.println("------------------------------------------------");
}

void setup()
{
  Serial.begin(115200);
  Serial.setRxBufferSize(10000);

  if (!SPIFFS.begin(true))
  {
    Serial.println("Error: SPIFFS Mount Failed!");
    return;
  }

  if (!isPinSet())
  {
    Serial.println("\n[No PIN] Please create a PIN to protect your data");
    Serial.print("Enter new PIN > ");
  }
  else
  {
    Serial.println("\n[Authentication Required] Please enter your PIN to continue");
    Serial.print("PIN > ");
  }
}

bool firstAuthenticated = true;
void loop()
{
  static bool authenticated = false;

  if (Serial.available())
  {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (!isPinSet())
    {
      if (input == "create" || input == "get" || input == "delete" || input == "list" || input == "info")
      {
        Serial.println("\n[Security] You need to create a PIN first");
        Serial.print("Enter new PIN > ");
      }
      else
      {
        createPin(input);
        Serial.println("\n✓ PIN created successfully!");
        Serial.println("[Authentication Required] Please enter your PIN to continue");
        Serial.print("PIN > ");
      }
      return;
    }

    if (!authenticated)
    {
      if (verifyPin(input))
      {
        authenticated = true;
        Serial.println("\n✓ Authentication successful!");
        showCommands();
      }
      else
      {
        if (firstAuthenticated)
        {
          firstAuthenticated = false;
          showWelcomeMessage();
          Serial.println("\n[Security] You need to enter a PIN first");
          Serial.print("Enter new PIN > ");
        }
        else
        {
          Serial.println("\n✗ Invalid PIN!");
          Serial.print("Try again > ");
        }
      }
      return;
    }

    if (input == "create")
    {
      Serial.print("\nEnter password name > ");
      while (!Serial.available())
        delay(100);
      String passName = Serial.readStringUntil('\n');
      passName.trim();

      if (isPasswordExists(passName))
      {
        Serial.println("\n✗ Error: Password name already exists!");
        return;
      }

      Serial.print("Enter password for '");
      Serial.print(passName);
      Serial.print("' > ");
      while (!Serial.available())
        delay(100);
      String password = Serial.readStringUntil('\n');
      password.trim();

      savePassword(passName, password);
      Serial.println("\n✓ Password saved successfully!");
    }
    else if (input == "get")
    {
      Serial.print("\nEnter password name > ");
      while (!Serial.available())
        delay(100);
      String passName = Serial.readStringUntil('\n');
      passName.trim();

      String password = getPasswordFromStorage(passName);
      if (password.length() > 0)
      {
        Serial.print("\nPassword: ");
        Serial.println(password);
      }
      else
      {
        Serial.println("\n✗ Password not found");
      }
    }
    else if (input == "delete")
    {
      Serial.print("\nEnter password name to delete > ");
      while (!Serial.available())
        delay(100);
      String passName = Serial.readStringUntil('\n');
      passName.trim();

      if (deletePassword(passName))
      {
        Serial.println("\n✓ Password deleted successfully!");
      }
      else
      {
        Serial.println("\n✗ Failed to delete or password not found");
      }
    }
    else if (input == "list")
    {
      listPasswords();
    }
    else if (input == "info")
    {
      showStorageInfo();
    }
    else
    {
      Serial.println("\n✗ Invalid command!");
      showCommands();
    }
    Serial.println("------------------------------------------------");
  }
}