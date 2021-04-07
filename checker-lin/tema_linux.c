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
	/* se goleste buffer-ul in care am scris pana in acest moment, daca avem date in acesta */
	int ret1 = so_fflush(stream);

	if (ret1 == SO_EOF) {
		int ret2 = close(stream->fd);

		free(stream);

		if (ret2 < 0)
			return SO_EOF;

		return SO_EOF;
	}

	/* se inchide file descriptorul care indica spre structura de fisier deschis */
	int ret2 = close(stream->fd);

	free(stream);

	if (ret2 < 0)
		return SO_EOF;

	return 0;
}


int syscall_read(SO_FILE *stream)
{
	size_t bytes_read = 0;
	int count = BUFFER_SIZE;

	/* daca s-a ajuns la sfarsitul fisierului, nu mai apelez read */
	if (stream->end_of_file == 1)
		return 0;

	/* se citesc mai multi bytes in avans din fisier pana se umple buffer-ul intern sau s-a ajuns la sfarsitul fisierului */
	int crt_bytes_read = read(stream->fd, stream->read_buffer + bytes_read, count - bytes_read);

	if (crt_bytes_read == 0) {
		/* sfarsitul fisierului */
		stream->end_of_file = 1;
		stream->end_of_file_pos = stream->cursor + crt_bytes_read;
		bytes_read += crt_bytes_read;
		stream->crt_read_buf_size = bytes_read;
		return SO_EOF;
	}
	if (crt_bytes_read < 0) {
		/* eroare */
		stream->found_error = 1;
		return SO_EOF;
	}

	bytes_read += crt_bytes_read;
	stream->crt_read_buf_size = bytes_read;

	stream->read_offset = 0;

	return 0;
}


int syscall_write(SO_FILE *stream)
{
	size_t bytes_written = stream->write_offset;
	int count = stream->crt_write_buf_size;

	/* se scriu datele din buffer in fisier pana cand totul a fost scris cu succes */
	while (bytes_written < count) {
		int crt_bytes_written = write(stream->fd, stream->write_buffer + bytes_written, count - bytes_written);

		if (crt_bytes_written <= 0) {
			stream->found_error = 1;
			return SO_EOF;
		}

		bytes_written += crt_bytes_written;
		stream->write_offset = bytes_written;
	}
	/* se reseteaza dimensiunea buffer-ului la 0 */
	stream->crt_write_buf_size = 0;
	stream->write_offset = 0;

	return 0;
}


int so_fgetc(SO_FILE *stream)
{
	stream->last_op = 0;

	if (so_feof(stream) == 1)
		return SO_EOF;

	/* daca buffer-ul intern de read este gol sau am citit tot ce se afla in buffer, se incearca umplerea buffer-ului */
	if (stream->crt_read_buf_size == 0 || stream->read_offset == stream->crt_read_buf_size) {
		int ret = syscall_read(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	/* se preia un caracter din buffer */
	int pos = stream->read_offset;
	int character = (int)stream->read_buffer[pos];

	stream->read_offset += 1;
	stream->cursor += 1;

	return character;
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