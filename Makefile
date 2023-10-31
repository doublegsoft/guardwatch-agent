CC=x86_64-w64-mingw32-gcc
RES=x86_64-w64-mingw32-windres

CFLAGS = -I./include 																		\
				 -I../guardwatch-thirdparty/mingw64/include 		\
				 -DWIN32_LEAN_AND_MEAN													\
				 -D_WIN32_WINNT=0x0600													\
				 -Wno-pointer-sign															\
				 -Wno-incompatible-pointer-types								\
				 -Wno-implicit-function-declaration

LFLAGS = -g	-DTRAY_WINAPI=1															\
				 -fno-stack-protector														\
				 -L../guardwatch-thirdparty/mingw64/lib 				\
				 -L/opt/local/x86_64-w64-mingw32/lib

LIBS =  -ltds -lreplacements -ltdsutils -llibct 				\
				-lsqlite3 -luser32 -ljson-c											\
			 	-lnetsnmpagent -lnetsnmpmibs -lnetsnmp					\
			 	-lws2_32 -luser32 -lpsapi -lshell32							\
			 	-liphlpapi																			\
			 	-lgfc -lgtdb

SRC=./src

all: resource guardwatch-agent.exe guardwatch-registry.exe test-suite

mongoose.o:
	$(CC) $(CFLAGS) -c $(SRC)/mongoose.c -o obj/$@

gw_main.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_main.c -o obj/$@

gw_mssql.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_mssql.c -o obj/$@

gw_sqlite.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_sqlite.c -o obj/$@

gw_snmp.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_snmp.c -o obj/$@

gw_http.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_http.c -o obj/$@

gw_os.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_os.c -o obj/$@

gw_app.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_app.c -o obj/$@

gw_win.o:
	$(CC) $(CFLAGS) -c $(SRC)/gw_win.c -o obj/$@

resource:
	$(RES) res/guardwatch.rc -o obj/guardwatch.rc.o
	
guardwatch-agent.exe: mongoose.o gw_main.o gw_mssql.o gw_sqlite.o gw_os.o gw_http.o gw_snmp.o gw_app.o gw_win.o
	rm -f obj/guardwatch-registry.rc.o
	$(CC) $(LFLAGS) -mwindows obj/*.o $(LIBS) -o bin/$@

guardwatch-registry.exe:
	$(RES) res/guardwatch-registry.rc -o obj/guardwatch-registry.rc.o
	$(CC) $(LFLAGS) -mwindows $(SRC)/gw_reg.c obj/guardwatch-registry.rc.o $(LIBS) -o bin/$@

test-suite:
	# gcc test/gw_snmp_test.c -lnetsnmpagent -lnetsnmp -o bin/gw_snmp_test

.PHONY: all