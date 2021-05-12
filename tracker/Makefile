CC = g++
CPP_EXT = cpp

SRCDIR = src
BUILDDIR = build
TARGETDIR = bin
TESTDIR = unitTests
TARGET = tracker.out

CPP_SOURCES = $(shell find $(SRCDIR) -type f -name *.$(CPP_EXT))
CPP_TEST_SOURCES = $(shell find $(TESTDIR) -type f -name *.$(CPP_EXT))

OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(CPP_SOURCES:.$(CPP_EXT)=.o))
TEST_OBJECTS = $(patsubst $(TESTDIR)/%,$(BUILDDIR)/%,$(CPP_TEST_SOURCES:.$(CPP_EXT)=.o))
CFLAGS = -std=c++17 -g -Wall
TESTFLAG = -lboost_unit_test_framework
INC = -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(TARGETDIR)
	$(CC) $^ -o $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(CPP_EXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.o: $(TESTDIR)/%.$(CPP_EXT)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

tests: $(OBJECTS) $(TEST_OBJECTS)
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)
	#$(CC) $(BUILDDIR)/TestObject.o $(BUILDDIR)/TestFile.o -o $(TARGETDIR)/UnitTests $(TESTFLAG)

test: tests
	@echo " "
	@echo " "
	#./bin/UnitTests

clean:
	@echo " Cleaning...";
	@echo " $(RM) -r $(BUILDDIR) $(TARGETDIR)"; $(RM) -r $(BUILDDIR) $(TARGETDIR)
	@echo " rm $(TARGET)"; rm $(TARGET)

.PHONY: clean
