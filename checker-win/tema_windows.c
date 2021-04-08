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