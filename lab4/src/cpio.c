#include "cpio.h"
#include "uart.h"
#include "lib.h"


#ifndef NULL
#define NULL ((void *)0)
#endif

void *CPIO_BASE;
void *CPIO_END;


static unsigned long align_up(unsigned long n, unsigned long align)
{
    return (n + align - 1) & (~(align - 1));
}

static unsigned long parse_hex_str(char *s, unsigned int max_len)
{
    unsigned long r = 0;
    unsigned long i;

    for (i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
        continue;
    }
    return r;
}


int cpio_strncmp(const char *a, const char *b, unsigned long n)
{
    unsigned long i;
    for (i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return a[i] - b[i];
        }
        if (a[i] == 0) {
            return 0;
        }
    }
    return 0;
}


// static char* cpio_strcpy(char *to, const char *from) {
//     char *save = to;
//     while (*from != 0) {
//         *to = *from;
//         to++;
//         from++;
//     }
//     return save;
// }


unsigned int cpio_strlen(const char *str) {
    const char *s;
    for (s = str; *s; ++s) {}
    return (s - str);
}


int cpio_parse_header(struct cpio_header *archive,
        char **filename, unsigned long *_filesize, void **data,
        struct cpio_header **next)
{
    unsigned long filesize;

    if (cpio_strncmp(archive->c_magic, CPIO_HEADER_MAGIC, sizeof(archive->c_magic)) != 0)
        return -1;

    filesize = parse_hex_str(archive->c_filesize, sizeof(archive->c_filesize));
    *filename = ((char *)archive) + sizeof(struct cpio_header);

    if (cpio_strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)) == 0)
        return 1;

    unsigned long filename_length = parse_hex_str(archive->c_namesize, sizeof(archive->c_namesize));
    *data = (void *)align_up(
        ((unsigned long)archive) 
            + sizeof(struct cpio_header) 
            + filename_length, 
        CPIO_ALIGNMENT
        );

    *next = (struct cpio_header *)align_up(
        ((unsigned long)*data) 
            + filesize, 
        CPIO_ALIGNMENT
        );

    if(_filesize){
        *_filesize = filesize;
    }

    return 0;
}


void *cpio_get_entry(void *archive, int n, char **name, unsigned long *size)
{
    int i;
    struct cpio_header *header = archive;
    void *result = NULL;

    for (i = 0; i <= n; i++) {
        struct cpio_header *next;
        int error = cpio_parse_header(header, name, size, &result, &next);
        if (error)
            return NULL;
        header = next;
    }

    return result;
}


void *cpio_get_file(void *archive, const char *name, unsigned long *size)
{
    struct cpio_header *header = archive;

    while (1) {
        struct cpio_header *next;
        void *result;
        char *current_filename;

        int error = cpio_parse_header(header, &current_filename,
                size, &result, &next);
        if (error)
            return NULL;
        if (cpio_strncmp(current_filename, name, -1) == 0)
            return result;
        header = next;
    }
}

int cpio_info(void *archive, struct cpio_info *info) {
    struct cpio_header *header, *next;
    char *current_filename;
    void *result;
    int error;
    unsigned long size, current_path_sz;

    if (info == NULL) return 1;
    info->file_count = 0;
    info->max_path_sz = 0;

    header = archive;
    while (1) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        if (error == -1) {
            return error;
        } else if (error == 1) {
            return 0;
        }
        info->file_count++;
        header = next;

        current_path_sz = cpio_strlen(current_filename);
        if (current_path_sz > info->max_path_sz) {
            info->max_path_sz = current_path_sz;
        }
    }

    return 0;
}


void cpio_ls(void *archive) {
    char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long i, size;

    header = archive;
    for (i = 0; i < 128; i++) {
        error = cpio_parse_header(header, &current_filename, &size, &result, &next);
        if (error) break;
        uart_async_puts(current_filename);
        // uart_async_hex((unsigned long long)result);
        // uart_async_puts(" ");
        // uart_async_hex(result);
        uart_async_puts("\n");
        header = next;
    }
}

void cpio_cat(void *archive, char *filename) {
    char *current_filename, *tmp;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long size;
    int exist = 0;

    tmp = filename;
    header = archive;
    while(header != 0) {
        error = cpio_parse_header(header, &current_filename, &size, &result, &next);
        if (error == 1) {
            break;
        }
        if (cpio_strncmp( tmp, current_filename, cpio_strlen(current_filename)) == 0) {
            exist = 1;
            uart_async_puts_len(result, size);
            uart_async_puts("\n");
            break;
        }
        header = next;
    }
    if (exist == 0) {
        uart_async_puts("File does not exists.\n");
    }
}