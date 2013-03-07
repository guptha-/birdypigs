DFLAG = -g
WFLAG = -Wall
C11FLAG = -std=c++0x
THREADFLAG = -pthread

SRCC = coordmain.cpp\
			 coordmsg.cpp\
			 PracticalSocket.cpp
INCC = coordinc.h\
			 coordprot.h\
			 coordconst.h\
			 comconst.h\
			 PracticalSocket.h
OBJC = $(SRCC:.cpp=.o)
SRCDIR = src
OBJDIR = obj
INCDIR = inc
MVOBJ = mv -f *.o obj/

CSRC = $(patsubst %,$(SRCDIR)/%,$(SRCC))
COBJ = $(patsubst %,$(OBJDIR)/%,$(OBJC))
CINC = $(patsubst %,$(INCDIR)/%,$(INCC))


SRCP = pigmain.cpp\
			 pigmsg.cpp\
			 PracticalSocket.cpp
INCP = piginc.h\
			 pigconst.h\
			 comconst.h\
			 PracticalSocket.h
OBJP = $(SRCP:.cpp=.o)

PSRC = $(patsubst %,$(SRCDIR)/%,$(SRCP))
POBJ = $(patsubst %,$(OBJDIR)/%,$(OBJP))
PINC = $(patsubst %,$(INCDIR)/%,$(INCP))

CREATEDIR = mkdir -p obj bin

all: coord pig

coord: $(COBJ)
	$(CREATEDIR)
	g++ -o bin/coord $(WFLAG) $(COBJ) -lm $(THREADFLAG)

obj/PracticalSocket.o: src/PracticalSocket.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/PracticalSocket.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/PracticalSocket.o

obj/coordmain.o: src/coordmain.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/coordmain.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/coordmain.o

obj/coordmsg.o: src/coordmsg.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/coordmsg.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/coordmsg.o


pig: $(POBJ)
	$(CREATEDIR)
	g++ -o bin/pig $(WFLAG) $(POBJ) -lm $(THREADFLAG)

obj/pigmain.o: src/pigmain.cpp $(PINC)
	$(CREATEDIR)
	g++ -c src/pigmain.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/pigmain.o

obj/pigmsg.o: src/pigmsg.cpp $(PINC)
	$(CREATEDIR)
	g++ -c src/pigmsg.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/pigmsg.o

clean:
	rm -rf bin/*
	rm -rf obj/*
