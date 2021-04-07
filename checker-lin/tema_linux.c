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
    stream->last_op = 1;
	int ret = c;
	int crt_pos = stream->crt_write_buf_size;

	/* se scrie un caracter in buffer */
	stream->write_buffer[crt_pos] = (unsigned char)c;
	stream->crt_write_buf_size += 1;
	stream->cursor += 1;

	/* daca buffer-ul de write este plin, se vor scrie datele in fisier */
	if (stream->crt_write_buf_size == BUFFER_SIZE)
		ret = syscall_write(stream);

	if (ret == 0 || ret == c)
		return c;

	return ret;
}

size_t so_fread(void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    unsigned char *memory_zone = (unsigned char *)ptr;
	int nr_elements_read = 0;

	/* se vor scrie la adresa de memorie mentionata byte cu byte datele intoarse de fgetc */
	for (int i = 0; i < nmemb; i++) {
		for (int j = 0; j < size; j++) {
			int ret = so_fgetc(stream);

			if (so_feof(stream) == 1 && nr_elements_read < nmemb)
				return nr_elements_read;
			else if (so_feof(stream) == 1)
				return 0;

			if (ret == SO_EOF)
				return 0;

			memory_zone[i * size + j] = (unsigned char)ret;
		}
		nr_elements_read++;
	}

	return nr_elements_read;
}

size_t so_fwrite(const void *ptr, size_t size, size_t nmemb, SO_FILE *stream)
{
    unsigned char *memory_zone = (unsigned char *)ptr;
	int nr_elements_written = 0;

	/* datele din zona de memorie mentionata se vor scrie in buffer-ul intern cu ajutorul functiei fputc */
	for (int i = 0; i < nmemb; i++) {
		for (int j = 0; j < size; j++) {
			unsigned char byte = memory_zone[i * size + j];
			int ret = so_fputc(byte, stream);

			if (ret == SO_EOF)
				return 0;
		}
		nr_elements_written++;
	}

	return nr_elements_written;
}

int so_fileno(SO_FILE *stream)
{
	return stream->fd;;
}

int so_fflush(SO_FILE *stream)
{
    /* se scriu datele din buffer in fisier, daca e cazul */
	if (stream->last_op == 1) {
		int ret = syscall_write(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	return 0;
}

int so_fseek(SO_FILE *stream, long offset, int whence)
{
    if (stream->last_op == 0) {
		/* operatie de citire -> read buffer invalid */
		stream->crt_read_buf_size = 0;
	} else if (stream->last_op == 1) {
		/* operatie de scriere -> e nevoie de fflush */
		int ret = so_fflush(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	/* se schimba pozitia cursorului */
	off_t pos = lseek(stream->fd, offset, whence);

	if (pos < 0) {
		stream->found_error = 1;
		return SO_EOF;
	}
	stream->cursor = pos;

	return 0;
}

long so_ftell(SO_FILE *stream)
{
    return stream->cursor;
}

int so_feof(SO_FILE *stream)
{
    return (stream->end_of_file == 1);
}

int so_ferror(SO_FILE *stream)
{
    /* intoarce 1 daca a avut loc o eroare pe parcurs in urma apelurile de sistem */
	return stream->found_error;
}


SO_FILE *so_popen(const char *command, const char *type)
{
    pid_t pid;
	int fd[2];
	SO_FILE *so_file = malloc(sizeof(SO_FILE));

	/* se creeaza pipe-ul necesar comunicarii dintre procesul parinte si procesul copil */
	int ret = pipe(fd);

	if (ret < 0)
		return NULL;

	/* se creeaza procesul copil */
	pid = fork();
	switch (pid) {
	case -1:
		/* eroare la crearea noului proces */
		close(fd[0]);
		close(fd[1]);
		free(so_file);
		return NULL;
	case 0:
		/* procesul copil */
		if (strcmp(type, "r") == 0) {
			/* se inchide STDIN */
			close(fd[0]);
			/* se redirecteaza STDOUT-ul copilului in capatul fd[1] al pipe-ului */
			dup2(fd[1], STDOUT_FILENO);
		} else if (strcmp(type, "w") == 0) {
			/* se inchide STDOUT */
			close(fd[1]);
			/* se redirecteaza STDIN-ul copilului in capatul fd[0] al pipe-ului */
			dup2(fd[0], STDIN_FILENO);
		}

		/* se schimba imaginea procesului copil cu a comenzii de mai jos */
		execlp("sh", "sh", "-c", (const char *)command, NULL);

		exit(EXIT_FAILURE);
	default:
		/* procesul parinte */
		if (so_file == NULL)
			return NULL;

		if (strcmp(type, "r") == 0) {
			/* se inchide STDOUT */
			close(fd[1]);
			/* se salveaza capatul pipe-ului cu care comunica parintele cu copilul */
			so_file->fd = fd[0];
		} else if (strcmp(type, "w") == 0) {
			/* se inchide STDIN */
			close(fd[0]);
			/* se salveaza capatul pipe-ului cu care comunica parintele cu copilul */
			so_file->fd = fd[1];
		}

		break;
	}

	so_file->crt_read_buf_size = 0;
	so_file->crt_write_buf_size = 0;
	so_file->write_offset = 0;
	so_file->read_offset = 0;
	so_file->found_error = 0;
	so_file->end_of_file = 0;
	so_file->last_op = -1;
	so_file->cursor = 0;
	so_file->end_of_file_pos = -2;
	so_file->child_pid = pid;

	return so_file;
}


int so_pclose(SO_FILE *stream)
{
    return -1;
}