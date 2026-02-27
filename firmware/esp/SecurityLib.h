#pragma once

#include <mbedtls/sha256.h>

String normalizeAuthToken(String token) {
  token.trim();
  const String bearerPrefix = "Bearer ";
  if (token.startsWith(bearerPrefix)) {
    token = token.substring(bearerPrefix.length());
  }
  token.trim();
  return token;
}

String sha256Hex(const String& input) {
  uint8_t digest[32];
  mbedtls_sha256_context context;
  mbedtls_sha256_init(&context);
  mbedtls_sha256_starts_ret(&context, 0);
  mbedtls_sha256_update_ret(
      &context,
      reinterpret_cast<const unsigned char*>(input.c_str()),
      input.length());
  mbedtls_sha256_finish_ret(&context, digest);
  mbedtls_sha256_free(&context);

  char hex[65];
  for (size_t i = 0; i < sizeof(digest); i++) {
    sprintf(&hex[i * 2], "%02x", digest[i]);
  }
  hex[64] = '\0';
  return String(hex);
}

bool hasApiAuthTokenConfigured() {
  return apiAuthTokenHash.length() == 64;
}

void clearApiAuthToken() {
  apiAuthTokenHash = "";
}

void setApiAuthToken(const String& rawToken) {
  String token = normalizeAuthToken(rawToken);
  if (token.length() == 0) {
    clearApiAuthToken();
    return;
  }
  apiAuthTokenHash = sha256Hex(token);
}

bool isApiAuthTokenAuthorized(const String& rawToken) {
  if (!hasApiAuthTokenConfigured()) {
    return false;
  }
  String token = normalizeAuthToken(rawToken);
  if (token.length() == 0) {
    return false;
  }
  return sha256Hex(token) == apiAuthTokenHash;
}
