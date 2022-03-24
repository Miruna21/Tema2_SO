Name: Orzata Miruna-Narcisa,
Group: 331CA

# Topic 2 SO

Organization
-
* Idea: Implementing the basic functions for working with files from the **stdio** library, taking into account the buffering process.
* The solution is based on developing a wrapper over **UNIX / WINDOWS** system calls made available by the operating system to exploit working with files and processes.
* In order to better imitate the behavior of the stdio library, we created a customized version of the **FILE** structure, in which we saved the information necessary to implement the solution. Also, out of a desire to have a higher performance (system calls are expensive), I took the idea of **buffering** present in the studio in case of a read from file or write to file. Thus, the actual calls of the read / ReadFile, respectively write / WriteFile functions are made only in case the internal buffers cannot fulfill a read or write request.


Implementation
-
* The first step in opening a file is done by the **so_fopen** function, which has the role of opening the file in the desired way and populating the SO_FILE structure.
* The **so_fread** function will write in the given memory space as a parameter the characters returned by the **so_fgetc** function. So_fgetc will take only one character from the internal buffer and will be able to call the syscall_read function, if the buffer is empty or has read everything in the buffer (syscall_read will fill in the read_buffer buffer).
* The **so_fwrite** function will write the data from the memory area mentioned in the write_buffer buffer via the **so_fputc** function, which will buffer only one character at a time. If the buffer is full, then so_fputc will call syscall_write to write the data to the file via a system call.
* The function **so_fseek** must, before changing the cursor position, invalidate read_buffer, if the last operation was read, or call fflush to write the data from write_buffer to the file, if the last operation was writing.
* The function **so_fopen** deals with the creation of a communication channel between a parent process and the child process. The parent creates a pipe with 2 ends: for read and for write, then starts a new process. The child process will have to transmit data to the parent process or vice versa, depending on the module specified in the parameter.
In this regard,
    * for type == 'r' -> the child will redirect his STDOUT to the writing end of the pipe and will close the reading end;
    * for type == 'w' -> the child will redirect his STDIN to the read end of the pipe and will close the write end.

    In turn, the parent will close the end of the pipe unused and will retain only the end that allows him to communicate with the child.
* The function **so_fclose** has the role of closing the tab of the descriptor that points to the open file structure and to free the memory occupied by SO_FILE.
* The **so_pclose** function is required for the parent to wait for the child's process and to release the resources used.
    
* Other details:
    * read from read_buffer or file is done only if the end of the file has not been reached. The read / ReadFile system call will indicate eof if the number of characters read was 0.

How is it compiled and run?
-
* Following the make / nmake command, the dynamic library libso_stdio.so / so_stdio.dll is generated.
* Each test is run separately as follows: "./_test/run_test.sh <nr_test>"
* All tests are run through: "./run_all.sh"

Resources used
-
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-03
* https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
* https://www.mkssoftware.com/docs/man3/popen.3.asp

Git
-
* https://github.com/Miruna21/Tema2_SO 
