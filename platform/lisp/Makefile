# Builds Lisp for C4 by bundling several source files together
C4=../../bin/c4
MKC4=$(C4) bin/makec4.c
SRC=c_src
IN_C=$(wildcard $(SRC)/*.c)
OUT_C=$(IN_C:.c=.c4)
PRE=c4_src/_preamble.c4
ENTRY=c4_src/_entry.c4
BUNDLE=c4_src/lisp.c
SNIP="// -- C4 SNIP"

.PHONY: clean run
all: $(OUT_C) $(BUNDLE)

%.c4: %.c
	$(MKC4) $< $(SNIP) > $@

$(BUNDLE): $(OUT_C) $(PRE) $(ENTRY)
	cat $(PRE) $(OUT_C) $(ENTRY) > $(BUNDLE)

bundle: $(BUNDLE)

clean:
	rm -f $(OUT_C)
	rm -f $(BUNDLE)

run: $(OUT_C) $(BUNDLE)
	$(C4) $(BUNDLE)
