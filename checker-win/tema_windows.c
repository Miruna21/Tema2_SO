#include "so_stdio.h"

SO_FILE *so_fopen(const char *pathname, const char *mode)
{
	HANDLE hFile;
	SO_FILE *so_file;
	int append_mode = 0;

	/* se deschide fisierul in modul dorit cu permisiunile necesare */
	if (strcmp(mode, "r") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	} else if (strcmp(mode, "r+") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						OPEN_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	}  else if (strcmp(mode, "w") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	}  else if (strcmp(mode, "w+") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						CREATE_ALWAYS | TRUNCATE_EXISTING,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
	}  else if (strcmp(mode, "a") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		append_mode = 1;
	}  else if (strcmp(mode, "a+") == 0) {
		hFile = CreateFile(pathname,
						GENERIC_READ | GENERIC_WRITE | FILE_APPEND_DATA,
						FILE_SHARE_READ | FILE_SHARE_WRITE,
						NULL,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_NORMAL,
						NULL);
		append_mode = 1;
	} else {
		return NULL;
	}

	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	/* se populeaza structura SO_FILE */
	so_file = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (so_file == NULL)
		return NULL;

	so_file->fd = hFile;
	so_file->crt_read_buf_size = 0;
	so_file->crt_write_buf_size = 0;
	so_file->write_offset = 0;
	so_file->read_offset = 0;
	so_file->found_error = 0;
	so_file->end_of_file = 0;
	so_file->last_op = -1;
	so_file->cursor = 0;
	so_file->end_of_file_pos = -2;
	so_file->child_process = 0;
	so_file->hProcess = NULL;
	so_file->hThread = NULL;
	so_file->append_mode = append_mode;

	return so_file;
}

int so_fclose(SO_FILE *stream)
{	int ret1;
	BOOL ret2;

	/* se goleste buffer-ul in care am scris, daca avem date in acesta */
	ret1 = so_fflush(stream);

	if (ret1 == SO_EOF) {
		ret2 = CloseHandle(stream->fd);

		free(stream);

		if (ret2 == FALSE)
			return SO_EOF;

		return SO_EOF;
	}

	/* se inchide file descriptorul care indica spre structura de fisier deschis */
	ret2 = CloseHandle(stream->fd);

	free(stream);

	if (ret2 == FALSE)
		return SO_EOF;

	return 0;
}

int syscall_read(SO_FILE *stream)
{
	DWORD bytes_read = 0;
	DWORD count = BUFFER_SIZE;
	DWORD crt_bytes_read;
	BOOL ret;

	/* daca s-a ajuns la sfarsitul fisierului, nu mai apelez read */
	if (stream->end_of_file == 1)
		return 0;

	/* se citesc mai multi bytes in avans din fisier pana se umple buffer-ul intern sau s-a ajuns la sfarsitul fisierului */
	ret = ReadFile(stream->fd, stream->read_buffer + bytes_read, count - bytes_read, &crt_bytes_read, NULL);

	if (ret == FALSE || (ret == FALSE && stream->child_process == 1 && GetLastError() == ERROR_BROKEN_PIPE)) {
		/* eroare */
		stream->found_error = 1;
		return SO_EOF;
	}

	if (crt_bytes_read == 0) {
		/* sfarsitul fisierului */
		stream->end_of_file = 1;
		stream->end_of_file_pos = stream->cursor + crt_bytes_read;
		bytes_read += crt_bytes_read;
		stream->crt_read_buf_size = bytes_read;
		return SO_EOF;
	}

	bytes_read += crt_bytes_read;
	stream->crt_read_buf_size = bytes_read;

	stream->read_offset = 0;

	return 0;
}


int syscall_write(SO_FILE *stream)
{
	DWORD bytes_written = stream->write_offset;
	DWORD count = stream->crt_write_buf_size;
	DWORD crt_bytes_written;
	BOOL ret;
	DWORD pos;

	if (stream->append_mode == 1) {
		pos = SetFilePointer(stream->fd, 0l, NULL, FILE_END);

		if (pos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
			return SO_EOF;
	}

	/* se scriu datele din buffer in fisier pana cand totul a fost scris cu succes */
	while (bytes_written < count) {
		ret = WriteFile(stream->fd, stream->write_buffer + bytes_written, count - bytes_written, &crt_bytes_written, NULL);

		if (ret == FALSE) {
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
	int pos;
	int character;
	int ret;

	stream->last_op = 0;

	if (so_feof(stream) == 1)
		return SO_EOF;

	/* daca buffer-ul intern de read este gol sau am citit tot ce se afla in buffer, se incearca umplerea buffer-ului */
	if (stream->crt_read_buf_size == 0 || stream->read_offset == stream->crt_read_buf_size) {
		ret = syscall_read(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	/* se preia un caracter din buffer */
	pos = stream->read_offset;
	character = (int)stream->read_buffer[pos];

	stream->read_offset += 1;
	stream->cursor += 1;

	return character;
}


int so_fputc(int c, SO_FILE *stream)
{
	int ret = c;
	int crt_pos = stream->crt_write_buf_size;

	stream->last_op = 1;

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
	size_t nr_elements_read = 0;
	size_t i, j;
	int ret;

	/* se vor scrie la adresa de memorie mentionata byte cu byte datele intoarse de fgetc */
	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			ret = so_fgetc(stream);

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
	size_t nr_elements_written = 0;
	int ret;
	unsigned char byte;
	size_t i, j;

	/* datele din zona de memorie mentionata se vor scrie in buffer-ul intern cu ajutorul functiei fputc */
	for (i = 0; i < nmemb; i++) {
		for (j = 0; j < size; j++) {
			byte = memory_zone[i * size + j];
			ret = so_fputc(byte, stream);

			if (ret == SO_EOF)
				return 0;
		}
		nr_elements_written++;
	}

	return nr_elements_written;
}


HANDLE so_fileno(SO_FILE *stream)
{
	return stream->fd;
}

int so_fflush(SO_FILE *stream)
{
	int ret;

	/* se scriu datele din buffer in fisier, daca e cazul */
	if (stream->last_op == 1) {
		ret = syscall_write(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	return 0;
}


int so_fseek(SO_FILE *stream, long offset, int whence)
{
	DWORD pos;
	int ret;

	if (stream->last_op == 0) {
		/* operatie de citire -> read buffer invalid */
		stream->crt_read_buf_size = 0;
	} else if (stream->last_op == 1) {
		/* operatie de scriere -> e nevoie de fflush */
		ret = so_fflush(stream);

		if (ret == SO_EOF)
			return SO_EOF;
	}

	/* se schimba pozitia cursorului */
	pos = SetFilePointer(stream->fd, offset, NULL, whence);

	if (pos == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR) {
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

static VOID RedirectHandle(STARTUPINFO *psi, HANDLE hFile, INT opt)
{
	if (hFile == INVALID_HANDLE_VALUE)
		return;

	ZeroMemory(psi, sizeof(*psi));
	psi->cb = sizeof(*psi);

	psi->hStdInput = GetStdHandle(STD_INPUT_HANDLE);
	psi->hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	psi->hStdError = GetStdHandle(STD_ERROR_HANDLE);

	psi->dwFlags |= STARTF_USESTDHANDLES;

	switch (opt) {
	case STD_INPUT_HANDLE:
		psi->hStdInput = hFile;
		break;
	case STD_OUTPUT_HANDLE:
		psi->hStdOutput = hFile;
		break;
	default:
		break;
	}
}

SO_FILE *so_popen(const char *command, const char *type)
{
	HANDLE fd_read, fd_write;
	BOOL ret;
	SECURITY_ATTRIBUTES sa;
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	SO_FILE *so_file;
	const char *aux = "cmd /C ";
	char *final_command = malloc(50 * sizeof(char));

	strcpy(final_command, aux);
	strcat(final_command, command);

	ZeroMemory(&sa, sizeof(sa));
	sa.bInheritHandle = TRUE;
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));

	/* se creeaza pipe-ul necesar comunicarii dintre procesul parinte si procesul copil */
	ret = CreatePipe(&fd_read, &fd_write, &sa, 0);
	if (ret == NULL)
		return NULL;


	/* pentru procesul copil */
	if (strcmp(type, "r") == 0) {
		/* se redirecteaza STDOUT-ul copilului in capatul fd_write al pipe-ului */
		RedirectHandle(&si, fd_write, STD_OUTPUT_HANDLE);
		/* setez nemostenbil fd_read */
		SetHandleInformation(fd_read, HANDLE_FLAG_INHERIT, 0);
	} else if (strcmp(type, "w") == 0) {
		/* se redirecteaza STDIN-ul copilului in capatul fd_read al pipe-ului */
		RedirectHandle(&si, fd_read, STD_INPUT_HANDLE);
		/* setez nemostenbil fd_write */
		SetHandleInformation(fd_write, HANDLE_FLAG_INHERIT, 0);
	}

	/* se creeaza procesul copil */
	ret = CreateProcess(
		NULL,
		final_command,
		NULL,
		NULL,
		TRUE,
		0,
		NULL,
		NULL,
		&si,
		&pi);

	free(final_command);

	if (ret == FALSE)
		return NULL;
	
	so_file = (SO_FILE *)malloc(sizeof(SO_FILE));

	if (so_file == NULL)
		return NULL;


	if (strcmp(type, "r") == 0) {
		/* se inchide STDOUT */
		CloseHandle(fd_write);
		/* se salveaza capatul pipe-ului cu care comunica parintele cu copilul */
		so_file->fd = fd_read;
	} else if (strcmp(type, "w") == 0) {
		/* se inchide STDIN */
		CloseHandle(fd_read);
		/* se salveaza capatul pipe-ului cu care comunica parintele cu copilul */
		so_file->fd = fd_write;
	} else
		return NULL;

	so_file->crt_read_buf_size = 0;
	so_file->crt_write_buf_size = 0;
	so_file->write_offset = 0;
	so_file->read_offset = 0;
	so_file->found_error = 0;
	so_file->end_of_file = 0;
	so_file->last_op = -1;
	so_file->cursor = 0;
	so_file->end_of_file_pos = -2;
	so_file->child_process = 1;
	so_file->hProcess = pi.hProcess;
	so_file->hThread = pi.hThread;
	so_file->append_mode = 0;

	return so_file;
}

int so_pclose(SO_FILE *stream)
{
	/* parintele ajunge aici */
	DWORD ret;
	DWORD exit_code;
	HANDLE hProcess = stream->hProcess;
	HANDLE hThread = stream->hThread;

	/* se elibereaza structura SO_FILE si se inchide file discriptorul parintelui ramas deschis */
	ret = so_fclose(stream);

	if (ret == SO_EOF)
		return SO_EOF;

	/* se asteapta procesul copil */
	ret = WaitForSingleObject(hProcess, INFINITE);

	if (ret == WAIT_FAILED)
		return SO_EOF;

	ret = GetExitCodeProcess(hProcess, &exit_code);

	if (ret == FALSE)
		return SO_EOF;

	CloseHandle(hThread);
	CloseHandle(hProcess);

	return exit_code;
}
