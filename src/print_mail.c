#include "config.h"
#include "i3status.h"
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <wordexp.h>
#include <linux/limits.h>

#define STRING_SIZE 16

void print_mail(yajl_gen json_gen, char *buffer, const char *format,
                const char *maildir) {
    char *outwalk = buffer;

    char raw_dirname[PATH_MAX];
    strncpy(raw_dirname, maildir, sizeof raw_dirname);
    strncpy(raw_dirname + strlen(raw_dirname), "/new",
            sizeof raw_dirname - strlen(raw_dirname));

    static DIR **dirs;
    static int nb_dir;

    if (!dirs) {
        wordexp_t we;
        int rc;

        rc = wordexp(raw_dirname, &we, WRDE_NOCMD|WRDE_SHOWERR|WRDE_UNDEF);
        if (rc != 0) {
            OUTPUT_FULL_TEXT("wordexp error");
            return;
        }

        if (we.we_wordc == 0) {
            OUTPUT_FULL_TEXT("MAIL_DIR expansion failed");
            return;
        }

        nb_dir = we.we_wordc;
        dirs = (DIR **) calloc(nb_dir, sizeof (DIR *));

        if (dirs == NULL) {
            perror("calloc");
            exit(EXIT_FAILURE);
        }

        for (int i=0 ; i<nb_dir ; i++) {
            char *expanded_dirname = we.we_wordv[i];
            DIR *d = opendir(expanded_dirname);
            if (!d) {
                OUTPUT_FULL_TEXT("MAIL_DIR not found");
                return;
            }
            dirs[i] = d;
        }
        wordfree(&we);
    } else {
        for (int i=0 ; i<nb_dir ; i++) rewinddir(dirs[i]);
    }

    int nb_mail = 0;
    for (int i=0 ; i<nb_dir ; i++) {
        struct dirent *dirent_ptr;
        while ((dirent_ptr = readdir(dirs[i]))) {
            if (dirent_ptr->d_type == DT_REG) nb_mail++;
        }
    }

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
