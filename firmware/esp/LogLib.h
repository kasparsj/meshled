#pragma once

#define LOG_PRUNE_SIZE 24576 // Size to prune log file down to when it exceeds maximum (24KB)

#include <stdarg.h>

// Maximum size of a formatted string buffer
#define MAX_LOG_BUFFER_SIZE 256

// Function to check if a file needs pruning
bool needsPruning(const char* filePath, size_t maxSize) {
  if (!SPIFFS.exists(filePath)) {
    return false;
  }

  File file = SPIFFS.open(filePath, FILE_READ);
  if (!file) {
    return false;
  }

  size_t size = file.size();
  file.close();

  return (size > maxSize);
}

// Function to prune a file to a target size
// Keeps the most recent entries and discards older ones
void pruneFile(const char* filePath, size_t targetSize) {
  if (!SPIFFS.exists(filePath)) {
    return;
  }

  File file = SPIFFS.open(filePath, FILE_READ);
  if (!file) {
    return;
  }

  size_t totalSize = file.size();

  // If the file is smaller than the target, no need to prune
  if (totalSize <= targetSize) {
    file.close();
    return;
  }

  // Calculate how much content to skip (from the beginning)
  size_t skipSize = totalSize - targetSize;

  // Read the file, skipping the first skipSize bytes
  file.seek(skipSize, SeekSet);
  String remainingContent = file.readString();
  file.close();

  // Find the first line break to ensure we start at a line boundary
  int firstNewline = remainingContent.indexOf('\n');
  if (firstNewline != -1) {
    remainingContent = remainingContent.substring(firstNewline + 1);
  }

  // Create a temporary file and write the pruned content
  File tempFile = SPIFFS.open("/temp.txt", FILE_WRITE);
  if (tempFile) {
    tempFile.print(remainingContent);
    tempFile.close();

    // Replace the original file with the pruned content
    SPIFFS.remove(filePath);
    SPIFFS.rename("/temp.txt", filePath);
  }
}

// Function to append to a file with size management
void appendToLogFile(const char* filePath, const String& content, size_t maxSize, size_t pruneSize) {
  // First check if file needs pruning
  if (needsPruning(filePath, maxSize)) {
    pruneFile(filePath, pruneSize);
  }

  // Now append the new content
  File file = SPIFFS.open(filePath, FILE_APPEND);
  if (file) {
    file.print(content);
    file.close();
  }
}

#ifdef LOG_FILE

#define MAX_LOG_SIZE 32768 // Maximum size for log file (32KB)

void logMessage(const String& message) {
  // Get timestamp for log entry
  unsigned long timestamp = millis();

  // Format log entry with timestamp
  String logEntry = String(timestamp) + ": " + message + "\n";
  appendToLogFile(LOG_FILE, logEntry, MAX_LOG_SIZE, LOG_PRUNE_SIZE);
}

// Function to log a formatted message
void logMessageF(const char* format, ...) {
  char buffer[MAX_LOG_BUFFER_SIZE];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, MAX_LOG_BUFFER_SIZE, format, args);
  va_end(args);

  logMessage(String(buffer));
}

#endif

#ifdef CRASH_LOG_FILE

#define MAX_CRASH_LOG_SIZE 32768 // Maximum size for crash log file (32KB)

// ESP32 crash handler libraries
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_partition.h"

// Crash handling
struct CrashInfo {
  uint32_t crashTime;     // Timestamp when the crash occurred
  uint32_t restartReason; // Reason for the restart
  uint32_t freeMem;       // Free memory at crash time
  uint32_t crashAddr;     // Address where the crash occurred
  uint32_t gpRegister[16]; // GP registers contents at crash time
  char crashMessage[128]; // Additional crash information
  bool crashDetected;     // Flag to indicate if a crash was detected
};

// Function to save crash info to file in SPIFFS
void saveCrashLog(const CrashInfo& crash) {
  // Create a formatted crash log entry
  String crashLog = "=== CRASH LOG ENTRY ===\n";
  crashLog += "Timestamp: " + String(crash.crashTime) + " ms\n";

  // Interpret restart reason
  String reasonStr = "Unknown";
  switch (crash.restartReason) {
    case 1:  reasonStr = "Power on"; break;
    case 3:  reasonStr = "Software reset"; break;
    case 4:  reasonStr = "Hardware watchdog"; break;
    case 5:  reasonStr = "Fatal exception"; break;
    case 6:  reasonStr = "Software watchdog"; break;
    case 7:  reasonStr = "Other"; break;
    case 8:  reasonStr = "Brownout"; break;
    case 9:  reasonStr = "SDIO"; break;
    case 10: reasonStr = "External reset"; break;
    case 12: reasonStr = "In deep sleep"; break;
    default: reasonStr = "Unknown reason: " + String(crash.restartReason); break;
  }

  crashLog += "Restart reason: " + reasonStr + "\n";

  if (crash.crashDetected) {
    crashLog += "Crash address: 0x" + String(crash.crashAddr, HEX) + "\n";
  }

  // Add the crash message if available
  if (strlen(crash.crashMessage) > 0) {
    crashLog += "Message: " + String(crash.crashMessage) + "\n";
  }

  // Add a separator at the end
  crashLog += "====================\n\n";

  // Use our file size management system to append to the crash log
  appendToLogFile(CRASH_LOG_FILE, crashLog, MAX_CRASH_LOG_SIZE, LOG_PRUNE_SIZE);
}

// Check if the system has restarted due to a crash
void checkForCrash() {
  esp_reset_reason_t reason = esp_reset_reason();

  CrashInfo crash;
  crash.crashTime = millis();
  crash.restartReason = (uint32_t)reason;
  crash.freeMem = ESP.getFreeHeap();
  crash.crashAddr = 0;
  crash.crashDetected = false;

  // Check if this was a crash restart (watchdog, exception, etc.)
  if (reason == ESP_RST_PANIC || reason == ESP_RST_WDT || reason == ESP_RST_BROWNOUT) {
    crash.crashDetected = true;

    if (reason == ESP_RST_PANIC) {
      snprintf(crash.crashMessage, sizeof(crash.crashMessage),
               "System crashed due to a panic. This could be caused by an unhandled exception, stack overflow, or other critical error.");
    } else if (reason == ESP_RST_WDT) {
      snprintf(crash.crashMessage, sizeof(crash.crashMessage),
               "System crashed due to watchdog timer. This indicates the system stopped responding for too long, possibly due to an infinite loop or deadlock.");
    } else if (reason == ESP_RST_BROWNOUT) {
      snprintf(crash.crashMessage, sizeof(crash.crashMessage),
               "System reset due to brownout. This indicates the voltage dropped below the safe threshold, possibly due to power supply issues or high current draw.");
    }
  } else {
    // Normal restart, just log the reason with more details
    switch (reason) {
      case ESP_RST_POWERON:
        snprintf(crash.crashMessage, sizeof(crash.crashMessage), "System powered on (normal startup)");
        break;
      case ESP_RST_SW:
        snprintf(crash.crashMessage, sizeof(crash.crashMessage), "Software reset via ESP.restart()");
        break;
      case ESP_RST_EXT:
        snprintf(crash.crashMessage, sizeof(crash.crashMessage), "External reset via reset pin");
        break;
      case ESP_RST_DEEPSLEEP:
        snprintf(crash.crashMessage, sizeof(crash.crashMessage), "Reset after exit from deep sleep");
        break;
      default:
        snprintf(crash.crashMessage, sizeof(crash.crashMessage), "System restarted with reason code %d", (int)reason);
    }
  }

  saveCrashLog(crash);
}

// Function to manually trigger a crash (for testing)
void triggerCrash() {
  delay(1000);

  // Trigger a null pointer exception
  int *p = NULL;
  *p = 42; // This will cause a crash
}

#endif