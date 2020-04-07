#include <config.h>
#include "i3status.h"
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>

#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <wordexp.h>

#define STRING_SIZE 16
#define MAIL_DIR "~/mail/inbox/new"

void print_mail(yajl_gen json_gen, char *buffer, const char *format) {
    char *outwalk = buffer;
    static DIR *d;

    if (!d) {
        wordexp_t we;
        int rc;

        rc = wordexp(MAIL_DIR, &we, WRDE_NOCMD|WRDE_SHOWERR|WRDE_UNDEF);
        if (rc != 0) {
            OUTPUT_FULL_TEXT("wordexp error");
            return;
        }

        if (we.we_wordc == 0) {
            OUTPUT_FULL_TEXT("MAIL_DIR expansion failed");
            return;
        } else if (we.we_wordc > 1) {
            OUTPUT_FULL_TEXT("MAIL_DIR expansion is ambiguous");
            return;
        }

        char *expanded_dirname = we.we_wordv[0];

        if (!(d = opendir(expanded_dirname))) {
            OUTPUT_FULL_TEXT("MAIL_DIR not found");
            return;
        }

        wordfree(&we);
    } else {
        rewinddir(d);
    }

    int nb_mail = -2;
    while (readdir(d)) nb_mail++;

    char *plural = "";
    if (nb_mail > 1) plural = "s";

    char string_nb_mail[STRING_SIZE];
    if (nb_mail > 0) {
        snprintf(string_nb_mail, sizeof string_nb_mail, "%d mail%s", nb_mail,
                plural);
    } else {
        string_nb_mail[0] = '\0';
    }

    placeholder_t placeholders[] = {
        { .name = "%nb_mail", .value = string_nb_mail },
    };

    {
        char *buffer;
        size_t num = sizeof placeholders / sizeof(placeholder_t);
        if (!format) format = "%nb_mail";
        buffer = format_placeholders(format, &placeholders[0], num);
        OUTPUT_FULL_TEXT(buffer);
        free(buffer);
    }
}
