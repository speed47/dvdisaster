#include <stdio.h>
#include <stddef.h>
extern char *g_strdup(const char *);
extern size_t g_strlcpy(char *, const char *, size_t);
extern char *g_strdup_printf(const char *, ...) __attribute__((format(printf, 1, 2)));
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
  }
  return 0;
}
