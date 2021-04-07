Nume: Orzata Miruna-Narcisa,
Grupă: 331CA

# Tema 2 SO

Organizare
-
* Ideea: Implementarea functiilor de baza pentru lucrul cu fisiere din biblioteca **stdio**, tinand cont si de procesul de buffering.
* Solutia se bazeaza pe dezvoltarea unui wrapper peste apelurile de sistem din **UNIX / WINDOWS** puse la dispozitie de sistemul de operare pentru a exploata lucrul cu fisiere si procese.
* Pentru a imita cat mai bine comportamentul bibliotecii stdio, am creat o versiune personalizata a structurii **FILE**, in care am salvat informatiile necesare implementarii solutiei. De asemenea, din dorinta de a avea o performanta marita(apelurile de sistem sunt costisitoare), am preluat ideea de **buffering** prezenta in stdio in cazul unei citiri din fisier sau a unei scrieri in fisier. Astfel, apelurile efective ale functiilor read/ReadFile, respectiv write/WriteFile se fac numai in cazul in care buffer-ele interne nu pot indeplini o cerere de citire sau scriere.


Implementare
-
* Primul pas in deschiderea unui fisier este realizat de catre functia **so_fopen**, care are rolul de a deschide fisierul in modul dorit si de a popula structura SO_FILE.
* Functia **so_fread** va scrie in spatiul de memorie dat ca parametru caracterele intoarse de functia **so_fgetc**. So_fgetc va prelua din buffer-ul intern un singur caracter si va putea apela functia syscall_read, in cazul in care buffer-ul este gol sau a citit tot ce se afla in buffer(syscall_read se va ocupa de umplerea buffer-ului read_buffer).
* Functia **so_fwrite** va scrie datele din zona de memorie mentionata in buffer-ul write_buffer prin intermediul functiei **so_fputc**, care va scrie in buffer un singur caracter la un moment dat. In cazul in care buffer-ul este plin, atunci so_fputc va apela syscall_write pentru a scrie datele in fisier printr-un apel de sistem.
* Functia **so_fseek** are obligatia ca, inainte de a schimba pozitia cursorului, sa invalideze read_buffer, daca ultima operatie facuta a fost de citire, sau sa apeleze fflush pentru a scrie datele din write_buffer in fisier, daca ultima operatie a fost de scriere.
* Functia **so_fopen** se ocupa de crearea unui canal de comunicatie intre un proces parinte si procesul copil. Parintele creeaza un pipe cu 2 capete: pentru read si pentru write, apoi porneste un nou proces. Procesul copil va trebui sa transmita date spre procesul parinte sau invers, in functie de modul specificat in parametru.
In acest sens,
    * pentru type == 'r' --> copilul isi va redirecta STDOUT in capatul de scriere al pipe-ului si va inchide capatul de citire;
    * pentru type == 'w' --> copilul isi va redirecta STDIN in capatul de citire al pipe-ului si va inchide capatul de scriere.

    La randul lui, parintele va inchide capatul de pipe nefolosit si va retine doar capatul care ii permite comunicarea cu copilul.
* Functia **so_fclose** are rolul de a inchide file descriptorul care indica spre structura de fisier deschis si de a elibera memoria ocupata de SO_FILE.
* Functia **so_pclose** este necesara pentru asteptarea procesului copil de catre parinte si pentru eliberarea resurselor folosite.
    
* Alte detalii:
    * citirea din read_buffer sau din fisier se face numai daca nu s-a ajuns la sfarsitul fisierului. Apelul de sistem read/ReadFile va indica eof daca numarul de caractere citite a fost 0.

Cum se compilează și cum se rulează?
-
* In urma comenzii make / nmake, se genereaza biblioteca dinamica libso_stdio.so / so_stdio.dll.
* Fiecare test se ruleaza separat astfel: "./_test/run_test.sh <nr_test>"
* Toate testele se ruleaza prin: "./run_all.sh"

Resurse utilizate
-
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-01
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-02
* https://ocw.cs.pub.ro/courses/so/laboratoare/laborator-03
* https://docs.microsoft.com/en-us/windows/win32/procthread/creating-a-child-process-with-redirected-input-and-output
* https://www.mkssoftware.com/docs/man3/popen.3.asp

Git
-
* https://github.com/Miruna21/Tema2_SO