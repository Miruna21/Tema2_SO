#include "so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
    int fd = -1;

	/* se deschide fisierul in modul dorit cu permisiunile necesare */
	if (strcmp(mode, "r") == 0)
		fd = open(pathname, O_RDONLY, 0644);
	else if (strcmp(mode, "r+") == 0)
		fd = open(pathname, O_RDWR, 0644);
	else if (strcmp(mode, "w") == 0)
		fd = open(pathname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	else if (strcmp(mode, "w+") == 0)
		fd = open(pathname, O_RDWR | O_CREAT | O_TRUNC, 0644);
	else if (strcmp(mode, "a") == 0)
		fd = open(pathname, O_WRONLY | O_CREAT | O_APPEND, 0644);
	else if (strcmp(mode, "a+") == 0)
		fd = open(pathname, O_RDWR | O_CREAT | O_APPEND, 0644);
	else
		return NULL;

	if (fd < 0)
		return NULL;

	/* se populeaza structura SO_FILE cu informatiile necesare lucrului cu fisiere si procese */
	SO_FILE *so_file = (SO_FILE *)malloc(sizeof(SO_FILE));

	so_file->fd = fd;
	so_file->crt_read_buf_size = 0;
	so_file->crt_write_buf_size = 0;
	so_file->write_offset = 0;
	so_file->read_offset = 0;
	so_file->found_error = 0;
	so_file->end_of_file = 0;
	so_file->last_op = -1;
	so_file->cursor = 0;
	so_file->end_of_file_pos = -2;
	so_file->child_pid = -1;

	return so_file;
}

int so_fclose(SO_FILE *stream)
{
    return -1;
}

int so_fgetc(SO_FILE *stream)
{
    return -1;
}

int so_fputc(int c, SO_FILE *stream)
{
    return -1;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    return 0;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    return 0;
}

int so_fileno(SO_FILE *stream)
{
	return -1;
}

int so_fflush(SO_FILE *stream)
{
    return -1;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
    return -1;
}

long so_ftell(SO_FILE *stream)
{
    return 0;
}

int so_feof(SO_FILE *stream)
{
    return 0;
}

int so_ferror(SO_FILE *stream)
{
    return 0;
}


SO_FILE *so_popen(const char *command, const char *type)
{
    return NULL;
}


int so_pclose(SO_FILE *stream)
{
    return -1;
}