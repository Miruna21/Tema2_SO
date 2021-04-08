build: so_stdio.dll

so_stdio.dll: tema_windows.obj
	link /nologo /dll /out:so_stdio.dll tema_windows.obj

tema_windows.obj: tema_windows.c
	cl /nologo /W3 /D_CRT_SECURE_NO_DEPRECATE /DDLL_IMPORTS /Fotema_windows.obj /c tema_windows.c

clean:
	del /Q /F *.obj *.lib *.dll *.exe  *.exp
