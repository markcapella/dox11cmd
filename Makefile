
# *****************************************************
# Variables to control Compile / Link.


# ****************************************************
# Compile any single FOO.cpp file and link it as FOO.
all:
	@echo
	@echo "Build starts ..."
	@echo

	g++ -Wall -ansi -g -m64 -std=c++17 -c dox11cmd.cpp
	g++ dox11cmd.o -m64 -L/usr/lib/x86_64-linux-gnu -lX11 -o dox11cmd

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
