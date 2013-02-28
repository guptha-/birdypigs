DFLAG = -g
WFLAG = -Wall
C11FLAG = -std=c++11

SRCC = coordmain.cpp
INCC = coordinc.h
OBJC = $(SRCC:.cpp=.o)
SRCDIR = src
OBJDIR = obj
INCDIR = inc
MVOBJ = mv -f *.o obj/

CSRC = $(patsubst %,$(SRCDIR)/%,$(SRCC))
COBJ = $(patsubst %,$(OBJDIR)/%,$(OBJC))
CINC = $(patsubst %,$(INCDIR)/%,$(INCC))

CREATEDIR = mkdir -p obj bin

coord: $(COBJ)
	$(CREATEDIR)
	g++ -o bin/coord $(WFLAG) $(COBJ) -lm

obj/coordmain.o: src/coordmain.cpp $(CINC)
	$(CREATEDIR)
	g++ -c src/coordmain.cpp -I inc $(C11FLAG) $(WFLAG) $(DFLAG) -o obj/coordmain.o

clean:
	rm -rf bin/remodel
	rm -rf obj/*
