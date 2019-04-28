all: worker instagrapd submitter

instagrapd : instagrapd.c
	gcc -o instagrapd -pthread instagrapd.c

submitter : submitter.c
	gcc -o submitter submitter.c

worker : worker.c
	gcc -o worker worker.c

clean:
	rm -rf worker submitter instagrapd output.c output.out output

