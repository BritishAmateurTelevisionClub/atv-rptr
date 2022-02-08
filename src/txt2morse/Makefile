# Makefile for txt2morse by Andrew Cashner 

app 	  = txt2morse
main	  = txt2morse.c
lib       = wavfile.c wavfile.h
build_dir = build
target	  = $(build_dir)/$(app)

.PHONY : all clean

all : $(target) 

$(target) : $(main) $(lib) | $(build_dir)
	$(CC) -o $@ $< -lm

$(build_dir) : 
	mkdir -p $(build_dir)

clean :
	rm -rf $(build_dir)

