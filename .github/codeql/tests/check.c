#include <stdio.h>
#include <stddef.h>
extern char *g_strdup(const char *);
extern size_t g_strlcpy(char *, const char *, size_t);
extern char *g_strdup_printf(const char *, ...) __attribute__((format(printf, 1, 2)));
typedef struct _GError GError;
extern char *g_locale_to_utf8(const char *, ptrdiff_t, size_t *, size_t *, GError **);
static void wrapper(char *format, ...) {
  char *message = g_strdup_printf("failed: %s", format);
  printf(format); /* SAFE: callers pass a literal */
  (void)message;
}
int main(int argc, char **argv) {
  char safe[100], unsafe[100];
  char *copy = g_strdup("literal %d");
  printf(copy, 1); /* SAFE */
  g_strlcpy(safe, "literal %d", sizeof(safe));
  printf(safe, 1); /* SAFE */
  wrapper("literal"); /* SAFE */
  char *utf8 = g_locale_to_utf8("literal %d", -1, NULL, NULL, NULL);
  printf(utf8, 1); /* SAFE */
  char converted[100];
  g_strlcpy(converted, utf8, sizeof(converted));
  printf(converted, 1); /* SAFE: conversion followed by bounded copy */
  if (argc > 1) {
    char *untrusted = g_strdup(argv[1]);
    printf(untrusted); /* ALERT */
    g_strlcpy(unsafe, argv[1], sizeof(unsafe));
    printf(unsafe); /* ALERT */
    char *formatted = g_strdup_printf("%s", argv[1]);
    printf(formatted); /* ALERT */
    char written[100];
    g_strdup_printf("%hhn", written);
    printf(written); /* ALERT: %n output is still a source */
    g_strdup_printf(argv[1], unsafe); /* ALERT: untrusted format */
    printf(unsafe); /* ALERT: unknown format may write via %n */
    char *untrusted_utf8 = g_locale_to_utf8(argv[1], -1, NULL, NULL, NULL);
    printf(untrusted_utf8); /* ALERT: encoding conversion is not sanitization */
    char untrusted_converted[100];
    g_strlcpy(untrusted_converted, untrusted_utf8, sizeof(untrusted_converted));
    printf(untrusted_converted); /* ALERT: conversion followed by bounded copy */
  }
  return 0;
}
