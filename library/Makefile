TARGET 			= ../build/libgnomekeyring.so
TESTTARGET 		= ../build/testGnomeKeyring
GNOME_CFLAGS 	= $(shell pkg-config --cflags gnome-keyring-1)
GNOME_LDFLAGS	= $(shell pkg-config --libs gnome-keyring-1)
CXXFLAGS 		+= -Wall -fPIC -g -Werror
LDFLAGS 		+=

all: $(TARGET) $(TESTTARGET)

$(TARGET): GnomeKeyring.c GnomeKeyring.h Makefile
	$(CC) $< -o $@ -shared $(GNOME_CFLAGS) $(GNOME_LDFLAGS) $(CXXFLAGS) $(LDFLAGS)

$(TESTTARGET): testGnomeKeyring.c GnomeKeyring.h Makefile
	$(CC) $< -o $@ $(CXXFLAGS) $(LDFLAGS) -ldl

clean:
	rm -f $(TARGET) $(TESTTARGET)
