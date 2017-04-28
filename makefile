!include "../global.mak"

ALL : "$(OUTDIR)\MQ2AutoAccept.dll"

CLEAN :
	-@erase "$(INTDIR)\MQ2AutoAccept.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\MQ2AutoAccept.dll"
	-@erase "$(OUTDIR)\MQ2AutoAccept.exp"
	-@erase "$(OUTDIR)\MQ2AutoAccept.lib"
	-@erase "$(OUTDIR)\MQ2AutoAccept.pdb"


LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib $(DETLIB) ..\Release\MQ2Main.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\MQ2AutoAccept.pdb" /debug /machine:I386 /out:"$(OUTDIR)\MQ2AutoAccept.dll" /implib:"$(OUTDIR)\MQ2AutoAccept.lib" /OPT:NOICF /OPT:NOREF 
LINK32_OBJS= \
	"$(INTDIR)\MQ2AutoAccept.obj" \
	"$(OUTDIR)\MQ2Main.lib"

"$(OUTDIR)\MQ2AutoAccept.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) $(LINK32_FLAGS) $(LINK32_OBJS)


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("MQ2AutoAccept.dep")
!INCLUDE "MQ2AutoAccept.dep"
!ELSE 
!MESSAGE Warning: cannot find "MQ2AutoAccept.dep"
!ENDIF 
!ENDIF 


SOURCE=.\MQ2AutoAccept.cpp

"$(INTDIR)\MQ2AutoAccept.obj" : $(SOURCE) "$(INTDIR)"

