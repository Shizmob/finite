CFLAGS    = -std=c99 -Wall -Wextra -pedantic -Wno-unused-parameter -fPIC
CPPFLAGS  = -D_XOPEN_SOURCE=700
LDFLAGS   = -pie -Wl,-z,relro -Wl,-z,now

DESTDIR   =
PREFIX    =
MANPREFIX = $(if $(subst /,,$(PREFIX)),$(PREFIX),/usr)
MAN5DIR   = $(MANPREFIX)/share/man/man5
MAN8DIR   = $(MANPREFIX)/share/man/man8


all: sysvinit simple
.PHONY: all clean distclean install uninstall

clean:
	@echo [CLN]
	@rm -rf bin obj
distclean: clean

install: install-sysvinit install-simple
uninstall: uninstall-sysvinit uninstall-simple

bin obj:
	@mkdir $@
$(DESTDIR)$(PREFIX)/sbin $(DESTDIR)$(MAN5DIR) $(DESTDIR)$(MAN8DIR):
	@install -d -m 0755 $@


.SECONDARY:

obj/init.o: src/common.h


.PHONY: sysvinit install-sysvinit uninstall-sysvinit symlink-sysvinit
sysvinit: \
    bin/sysvinit-init \
    bin/sysvinit-halt \
    bin/sysvinit-killall5 \
    bin/sysvinit-shutdown
bin/sysvinit-init: obj/init.o obj/common.o obj/sysvinit/inittab.o obj/sysvinit/runlevel.o
bin/sysvinit-halt: obj/sysvinit/runlevel.o obj/sysvinit/wall.o
bin/sysvinit-shutdown: obj/sysvinit/runlevel.o obj/sysvinit/wall.o
bin/sysvinit-%: obj/sysvinit/%.o | bin
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $^ -o $@

SYSVINIT= \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-init \
    $(DESTDIR)$(MAN8DIR)/sysvinit-init.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-halt \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-poweroff \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-reboot \
    $(DESTDIR)$(MAN8DIR)/sysvinit-halt.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-killall5 \
    $(DESTDIR)$(MAN8DIR)/sysvinit-killall5.8 \
    $(DESTDIR)$(PREFIX)/sbin/sysvinit-shutdown \
    $(DESTDIR)$(MAN8DIR)/sysvinit-shutdown.8 \
    $(DESTDIR)$(MAN5DIR)/sysvinit-inittab.5
install-sysvinit: $(SYSVINIT)
uninstall-sysvinit:
	@rm -f $(SYSVINIT)
symlink-sysvinit: $(SYSVINIT)
	@for f in $(SYSVINIT) ; do nf=$$(dirname "$$f")/$$(echo $$(basename "$$f") | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -sf $$(basename "$$f") $$nf  ; done

$(DESTDIR)$(PREFIX)/sbin/sysvinit-%: bin/sysvinit-% | $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@

$(DESTDIR)$(PREFIX)/sbin/sysvinit-poweroff $(DESTDIR)$(PREFIX)/sbin/sysvinit-reboot: $(DESTDIR)$(PREFIX)/sbin/sysvinit-halt
	@echo [ LN] $(notdir $@)
	@ln -s $(notdir $<) $@

$(DESTDIR)$(MAN5DIR)/sysvinit-%.5: docs/sysvinit/%.5 | $(DESTDIR)$(MAN5DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@

$(DESTDIR)$(MAN8DIR)/sysvinit-%.8: docs/sysvinit/%.8 | $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@


.PHONY: simple install-simple uninstall-simple symlink-simple
simple: bin/simple-init
bin/simple-init: obj/init.o obj/common.o
bin/simple-%: obj/simple/%.o | bin
	@echo [ LD] $(notdir $@)
	@$(CC) $(LDFLAGS) $^ -o $@

SIMPLE= \
    $(DESTDIR)$(PREFIX)/sbin/simple-init \
    $(DESTDIR)$(MAN8DIR)/simple-init.8
install-simple: $(SIMPLE)
uninstall-simple:
	@rm -f $(SIMPLE)
symlink-simple: $(SIMPLE)
	@for f in $(SIMPLE) ; do nf=$$(dirname "$$f")/$$(echo $$(basename "$$f") | cut -d- -f2-) ; echo [ LN] $$(basename "$$nf") ; ln -sf $$(basename "$$f") $$nf ; done

$(DESTDIR)$(PREFIX)/sbin/simple-%: bin/simple-% | $(DESTDIR)$(PREFIX)/sbin
	@echo [BIN] $(notdir $@)
	@install -m 0755 $< $@

$(DESTDIR)$(MAN8DIR)/simple-%.8: docs/simple/%.8 | $(DESTDIR)$(MAN8DIR)
	@echo [MAN] $(notdir $@)
	@install -m 0644 $< $@


obj/%.o: src/%.c | obj
	@mkdir -p $(dir $(@))
	@echo [ CC] $^
	@$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@
