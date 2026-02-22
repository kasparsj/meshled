#pragma once

// File Upload Variables
File uploadFile;
bool uploadSuccess = false;
String uploadFilename = "";

void streamSpiffs(WiFiClient& client) {
  client.println("<div style='margin: 20px 0; padding: 15px; background: #555; border-radius: 5px; position: relative;'>");

// Display messages based on query parameters
  if (server.hasArg("success")) {
    String filename = server.hasArg("filename") ? server.arg("filename") : "file";
    client.println("<div style='background-color: #285;color: white;padding: 15px;margin-bottom: 20px;border-radius: 5px;'>");
    client.print("<strong>Success!</strong> ");
    
    // Special case for SPIFFS format
    if (filename == "SPIFFS") {
      client.println("SPIFFS was formatted successfully.");
    } else {
      client.print(filename);
      client.println(" was uploaded successfully.");
    }
    
    client.println("</div>");
  } else if (server.hasArg("deleted")) {
    String filename = server.arg("deleted");
    client.println("<div style='background-color: #285;color: white;padding: 15px;margin-bottom: 20px;border-radius: 5px;'>");
    client.print("<strong>Success!</strong> ");
    client.print(filename);
    client.println(" was deleted successfully.");
    client.println("</div>");
  } else if (server.hasArg("error")) {
    String error = server.arg("error");
    String errorMsg = "An error occurred.";
    
    if (error == "no_file_specified") {
      errorMsg = "No file was specified for deletion.";
    } else if (error == "file_not_found") {
      errorMsg = "The file could not be found.";
    } else if (error == "delete_failed") {
      String filename = server.hasArg("file") ? server.arg("file") : "file";
      errorMsg = "Failed to delete " + filename + ".";
    } else if (error == "format_failed") {
      errorMsg = "Failed to format SPIFFS.";
    }
    
    client.println("<div style='background-color: #852;color: white;padding: 15px;margin-bottom: 20px;border-radius: 5px;'>");
    client.print("<strong>Error:</strong> ");
    client.print(errorMsg);
    client.println("</div>");
  }

  client.println("<h3>SPIFFS Info:</h3>");
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  float percentUsed = 100.0 * usedBytes / totalBytes;
  
  // Create a visual usage bar
  client.println("<div style='background: #222; height: 20px; border-radius: 10px; margin: 10px 0;'>");
  
  // Determine bar color based on percentage
  String barColor;
  if (percentUsed > 90) {
    barColor = "#d44"; // Red for high usage
  } else if (percentUsed > 70) {
    barColor = "#da4"; // Orange/yellow for medium usage
  } else {
    barColor = "#4a4"; // Green for low usage
  }
  
  // Create the progress bar
  client.print("<div style='background: ");
  client.print(barColor);
  client.print("; width: ");
  client.print(String(percentUsed));
  client.println("%; height: 100%; border-radius: 10px;'></div>");
  
  client.println("</div>");
  
  // Display storage usage information
  client.print("<p>Used: <span class='value'>");
  client.print(String(usedBytes / 1024.0, 1));
  client.print(" KB</span> of <span class='value'>");
  client.print(String(totalBytes / 1024.0, 1));
  client.print(" KB</span> (<span class='value'>");
  client.print(String(percentUsed, 1));
  client.println("%</span>)</p>");
  
  // Get uploaded filename from success parameter or default to scripts.js
  String viewFilename = server.hasArg("filename") ? server.arg("filename") : "";
  
  // If a specific file is requested to be viewed
  if (server.hasArg("view")) {
    viewFilename = server.arg("view");
  }
  
  // Display file content if it exists
  client.println("<h2>SPIFFS Files</h2>");
  client.println("<div style='background: #333; padding: 10px; border-radius: 5px; margin-bottom: 20px;'>");
  
  // List all files in SPIFFS
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  bool hasFiles = false;
  
  while (file) {
    hasFiles = true;
    String filename = String(file.name());
    #ifndef DEV_ENABLED
    if (filename == "credentials.json") {
      continue;
    }
    #endif
    
    // Get file size
    size_t fileSize = file.size();
    
    // Create a link to view each file with delete button
    String bgColor = (filename == viewFilename ? "#444" : "#3a3a3a");
    client.print("<div style='margin: 5px 0; padding: 8px; background: ");
    client.print(bgColor);
    client.print("; border-radius: 3px; display: flex; align-items: center;'>");
    
    // File link (takes most of the space)
    String textColor = (filename == viewFilename ? "#fff" : "#ccc");
    client.print("<a href='/spiffs?view=");
    client.print(filename);
    client.print("' style='color: ");
    client.print(textColor);
    client.print("; text-decoration: none; flex-grow: 1;'>");
    client.print(filename);
    client.print(" <span style='float:right; color: #999;'>");
    
    // File size display
    if (fileSize < 1024) {
      client.print(String(fileSize) + " B");
    } else {
      client.print(String(fileSize / 1024.0, 1) + " KB");
    }
    
    client.print("</span></a>");
    
    #ifdef SPIFFS_DELETE
    client.print("<a href='/delete?file=");
    client.print(filename);
    client.print("' onclick=\"return confirm('Are you sure you want to delete ");
    client.print(filename);
    client.print("?')\" style='background: #922; color: white; text-decoration: none; ");
    client.print("padding: 2px 8px; border-radius: 3px; margin-left: 10px; font-size: 12px;");
    client.print("'>Delete</a>");
    #endif
    client.print("</div>");
    
    file = root.openNextFile();
    yield(); // Allow system to process other tasks
  }
  
  if (!hasFiles) {
    client.println("<p style='color: #999;'><em>No files found in SPIFFS</em></p>");
  }
  
  client.println("</div>");
  
  // Display currently viewing file content
  if (viewFilename != "" && SPIFFS.exists("/" + viewFilename)) {
    client.println("<h3>Content of " + viewFilename + ":</h3>");
    client.println("<div style='background: #222; padding: 10px; border-radius: 5px; max-height: 400px; overflow-y: auto;'>");
    client.println("<pre style='margin: 0; color: #ddd; white-space: pre-wrap; word-break: break-all;'>");
    
    File viewFile = SPIFFS.open("/" + viewFilename, "r");
    if (viewFile) {
      // Get file size for progress tracking
      size_t fileSize = viewFile.size();
      size_t bytesRead = 0;
      
      while (viewFile.available()) {
        String line = viewFile.readStringUntil('\n');
        // Escape HTML characters to display properly
        line.replace("&", "&amp;");
        line.replace("<", "&lt;");
        line.replace(">", "&gt;");
        client.println(line);
        
        // Track progress for larger files
        bytesRead += line.length() + 1; // +1 for newline
        
        // Yield more frequently for larger files
        if (fileSize > 10000 && bytesRead % 1000 == 0) {
          yield();
        } else {
          yield(); // Always yield at least once per line
        }
      }
      viewFile.close();
    } else {
      client.println("Error opening file");
    }
    
    client.println("</pre>");
    client.println("</div>");
  }

  #ifdef SPIFFS_UPLOAD
  client.println("<h2>Upload to SPIFFS</h2>");
  client.println("<form method='POST' action='/spiffs' enctype='multipart/form-data'>");
  client.println("<div><label for='file'>Select file:</label>");
  client.println("<input type='file' name='file' id='file' required='required'></div>");
  // Add a hidden CSRF token using the device hostname as the token
  client.println("<input type='hidden' name='csrf_token' value='" + deviceHostname + "'>");
  // Display allowed file types and size limits
  client.println("<p style='font-size: 12px; color: #aaa;'>Allowed file types: .js, .json, .txt, .csv, .html, .htm, .css</p>");
  client.println("<p style='font-size: 12px; color: #aaa;'>Maximum file size: 1MB</p>");
  client.println("<div><button type='submit'>Upload File</button></div>");
  client.println("</form>");
  #endif

  #ifdef SPIFFS_FORMAT
  // Add Format SPIFFS button with confirmation dialog
  client.println("<div style='margin-top: 15px;'>");
  client.println("<form method='POST' action='/format-spiffs' onsubmit=\"return confirm('WARNING: Formatting will delete ALL files in SPIFFS. This cannot be undone! Continue?');\">");
  client.println("<button type='submit' style='background: #922; color: white; padding: 5px 10px; border: none; border-radius: 3px; cursor: pointer;'>Format SPIFFS</button>");
  client.println("</form>");
  client.println("</div>");
  #endif
  
  client.println("</div>");
}

// Define allowed file extensions and maximum file size
const char* ALLOWED_EXTENSIONS[] = {".js", ".json", ".txt", ".csv", ".html", ".htm", ".css"};
const int NUM_ALLOWED_EXTENSIONS = 7;
const size_t MAX_UPLOAD_SIZE = 1048576; // 1MB max file size

// Helper function to check if a file extension is allowed
bool isExtensionAllowed(const String& filename) {
  String lowerFilename = filename;
  lowerFilename.toLowerCase(); // Case-insensitive comparison
  
  for (int i = 0; i < NUM_ALLOWED_EXTENSIONS; i++) {
    if (lowerFilename.endsWith(ALLOWED_EXTENSIONS[i])) {
      return true;
    }
  }
  return false;
}

// Handler for file upload process
void handleFileUpload() {
  #ifdef SPIFFS_ENABLED
  HTTPUpload& upload = server.upload();
  static size_t totalBytes = 0;
  
  if (upload.status == UPLOAD_FILE_START) {
    // Reset tracking variables
    uploadSuccess = false;
    totalBytes = 0;
    String filename = upload.filename;
    
    // todo: temp disabled, server.arg is not reliable with file uploads, perhaps need to send it by header
    // Verify CSRF token (enabled when form submission is from our page)
    // if (!server.hasHeader("X-Requested-With")) {
    //   if (!server.hasArg("csrf_token") || server.arg("csrf_token") != deviceHostname) {
    //     LP_LOGF("CSRF validation failed for upload %s != %s", server.arg("csrf_token"), deviceHostname);
    //     return; // Fail silently - this prevents CSRF attacks
    //   }
    // }
    
    // Check for empty filename
    if (filename.length() == 0) {
      LP_LOGLN("Empty filename - upload rejected");
      return;
    }
    
    // More thorough filename sanitization to prevent directory traversal
    // Remove any path components, control characters, and special characters
    String sanitizedFilename = "";
    for (size_t i = 0; i < filename.length(); i++) {
      char c = filename[i];
      // Allow alphanumeric, '.', '-', and '_' characters only
      if (isalnum(c) || c == '.' || c == '-' || c == '_') {
        sanitizedFilename += c;
      }
    }
    
    // Ensure filename doesn't start with dots or have multiple extensions
    if (sanitizedFilename.startsWith(".") || sanitizedFilename.indexOf("..") >= 0) {
      LP_LOGLN("Invalid filename pattern detected: " + sanitizedFilename);
      return;
    }
    
    // File extension validation
    if (!isExtensionAllowed(sanitizedFilename)) {
      LP_LOGLN("Unsupported file type: " + sanitizedFilename);
      return;
    }
    
    if (sanitizedFilename.length() > 0) {
      // Store the filename for the completion handler
      uploadFilename = sanitizedFilename;
      LP_LOGLN("File upload started: " + sanitizedFilename);
      
      // Open the file for writing
      uploadFile = SPIFFS.open("/" + sanitizedFilename, "w");
      
      if (!uploadFile) {
        LP_LOGLN("Failed to open file for writing: " + sanitizedFilename);
        uploadSuccess = false;
        return;
      }
    } else {
      LP_LOGLN("Invalid filename after sanitization - upload rejected");
      return;
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    // Enforce maximum file size
    totalBytes += upload.currentSize;
    if (totalBytes > MAX_UPLOAD_SIZE) {
      LP_LOGLN("Upload exceeds maximum file size (" + String(MAX_UPLOAD_SIZE / 1024) + "KB)");
      if (uploadFile) {
        uploadFile.close();
        // Delete the partial file
        SPIFFS.remove("/" + uploadFilename);
      }
      uploadSuccess = false;
      return;
    }
    
    // Write the received bytes to the file
    if (uploadFile) {
      size_t written = uploadFile.write(upload.buf, upload.currentSize);
      if (written != upload.currentSize) {
        LP_LOGLN("Error writing file: " + String(written) + "/" + String(upload.currentSize) + " bytes");
      } else {
        LP_LOGLN("Writing file: " + String(upload.currentSize) + " bytes");
      }
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    // Close the file when upload is complete
    if (uploadFile) {
      uploadFile.close();
      
      // Verify file size doesn't exceed limit
      if (upload.totalSize > MAX_UPLOAD_SIZE) {
        LP_LOGLN("Upload size exceeds maximum allowed (" + String(MAX_UPLOAD_SIZE / 1024) + "KB)");
        SPIFFS.remove("/" + uploadFilename);
        uploadSuccess = false;
      } else {
        uploadSuccess = true;
        LP_LOGLN("Upload complete: " + String(upload.totalSize) + " bytes");
      }
    }
  } else if (upload.status == UPLOAD_FILE_ABORTED) {
    LP_LOGLN("Upload aborted");
    if (uploadFile) {
      uploadFile.close();
      // Remove partial file
      SPIFFS.remove("/" + uploadFilename);
    }
    uploadSuccess = false;
  }
  #endif
}

// Handler for upload completion
void handleUploadComplete() {
  #ifdef SPIFFS_ENABLED
  if (uploadSuccess) {
    // Sanitize filename parameter for the redirect URL
    String safeFilename = uploadFilename;
    safeFilename.replace("&", "&amp;");
    safeFilename.replace("<", "&lt;");
    safeFilename.replace(">", "&gt;");
    safeFilename.replace("\"", "&quot;");
    safeFilename.replace("'", "&#x27;");
    
    // Redirect with success flag and the uploaded filename
    server.sendHeader("Location", "/spiffs?success=1&filename=" + safeFilename + "&view=" + safeFilename, true);
    server.send(303, "text/plain", "");
  } else {
    // Provide a more descriptive error message in the 500 response with debug info
    String debugInfo = "Upload failed. ";
    
    // Add more specific debug information based on known checks
    if (!server.hasArg("csrf_token") || server.arg("csrf_token") != deviceHostname) {
      debugInfo += "CSRF token validation failed. ";
      LP_LOGLN("Upload failed: CSRF token validation failed");
    }
    
    if (uploadFilename.length() == 0) {
      debugInfo += "Empty or invalid filename. ";
      LP_LOGLN("Upload failed: Empty or invalid filename");
    }
    
    if (!isExtensionAllowed(uploadFilename) && uploadFilename.length() > 0) {
      debugInfo += "Unsupported file type (allowed: .js, .json, .txt, .csv, .html, .htm, .css). ";
      LP_LOGLN("Upload failed: Unsupported file type: " + uploadFilename);
    }
    
    // Log some system information that might help diagnose the issue
    LP_LOGLN("SPIFFS info - Total: " + String(SPIFFS.totalBytes() / 1024) + "KB, Used: " + String(SPIFFS.usedBytes() / 1024) + "KB");
    LP_LOGLN("Device hostname: " + deviceHostname);
    
    // Send the detailed error message
    server.send(500, "text/plain", debugInfo + "Check the device logs for more information.");
  }
  #else
  server.send(501, "text/plain", "SPIFFS not enabled");
  #endif
}

// Handler for deleting files
void handleDeleteFile() {
  #ifdef SPIFFS_ENABLED
  if (!server.hasArg("file")) {
    server.sendHeader("Location", "/spiffs?error=no_file_specified", true);
    server.send(303, "text/plain", "");
    return;
  }
  
  String filename = server.arg("file");
  
  // Basic security check - prevent directory traversal
  if (filename.indexOf('/') >= 0 || filename.indexOf('\\') >= 0) {
    filename = filename.substring(filename.lastIndexOf('/') + 1);
    filename = filename.substring(filename.lastIndexOf('\\') + 1);
  }
  
  String fullPath = "/" + filename;
  
  if (!SPIFFS.exists(fullPath)) {
    server.sendHeader("Location", "/spiffs?error=file_not_found", true);
    server.send(303, "text/plain", "");
    return;
  }
  
  if (SPIFFS.remove(fullPath)) {
    LP_LOGLN("Deleted file: " + fullPath);
    server.sendHeader("Location", "/spiffs?deleted=" + filename, true);
    server.send(303, "text/plain", "");
  } else {
    LP_LOGLN("Failed to delete file: " + fullPath);
    server.sendHeader("Location", "/spiffs?error=delete_failed&file=" + filename, true);
    server.send(303, "text/plain", "");
  }
  #else
  server.send(501, "text/plain", "SPIFFS not enabled");
  #endif
}

#ifdef SPIFFS_FORMAT
// Handler for formatting SPIFFS
void handleFormatSpiffs() {
  #ifdef SPIFFS_ENABLED
  LP_LOGLN("Formatting SPIFFS...");
  
  // Attempt to format SPIFFS
  bool formatSuccess = SPIFFS.format();
  
  if (formatSuccess) {
    LP_LOGLN("SPIFFS formatted successfully");
    server.sendHeader("Location", "/spiffs?success=1&filename=SPIFFS", true);
    server.send(303, "text/plain", "");
  } else {
    LP_LOGLN("Failed to format SPIFFS");
    server.sendHeader("Location", "/spiffs?error=format_failed", true);
    server.send(303, "text/plain", "");
  }
  #else
  server.send(501, "text/plain", "SPIFFS not enabled");
  #endif
}
#endif