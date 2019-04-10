all: instagrapd submitter

instagrapd : instagrapd.c
	gcc -o instagrapd instagrapd.c

submitter : submitter.c
	gcc -o submitter submitter.c

clean:
	rm -rf submitter instagrapd

