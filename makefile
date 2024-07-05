
# *****************************************************
# Variables to control Compile / Link.

LIBX11DEV = /usr/include/X11/Xlib.h

# ****************************************************
# Compile any single FOO.cpp file and link it as FOO.
all:
	@if [ ! -f $(LIBX11DEV) ]; then \
		echo "Error! The libx11-dev package is not installed,"; \
		echo "   but is required to compile."; \
		echo ""; \
		echo "Try 'sudo apt install libx11-dev'"; \
		echo "   then re-run this make."; \
		echo ""; \
		exit 1; \
	fi

	@echo
	@echo "Build starts ..."
	@echo

	g++ -Wall -ansi -g -m64 -std=c++11 -c dox11cmd.cpp
	g++ dox11cmd.o \
		-m64 -L/usr/lib/x86_64-linux-gnu -lX11 \
		-lxcb -o dox11cmd

	@echo
	@echo "Build Done !"
	@echo

# ****************************************************
# sudo make install

install: all
ifneq ($(shell id -u), 0)
	@echo "You must be root to perform this action. Please re-run with:"
	@echo "   sudo make install"
	@echo
	@exit 1;
endif

	@echo
	@echo "Install: starts ..."
	@echo

	cp dox11cmd /usr/local/bin
	chmod +x /usr/local/bin/dox11cmd

	@echo
	@echo "Install Done !"
	@echo

# ****************************************************
# sudo make uninstall

uninstall:
ifneq ($(shell id -u), 0)
	@echo "You must be root to perform this action. Please re-run with:"
	@echo "   sudo make uninstall"
	@echo
	@exit 1;
endif

	@echo
	@echo "Uninstall: starts ..."
	@echo

	rm -f /usr/local/bin/dox11cmd

	@echo
	@echo "Uninstall Done !"
	@echo

# ****************************************************
# make clean

clean:
	@echo
	@echo "Clean: starts ..."
	@echo

	rm -f dox11cmd.o
	rm -f dox11cmd

	@echo
	@echo "Clean Done !"
	@echo
