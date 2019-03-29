#include <stdio.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char * argv[]) {
	int uid = 0;
	printf("testlog : %s %s\n", argv[1], argv[2]);
	FILE * fp;
	fp = fopen("/proc/bingo", "w");

	if (strcmp(argv[1], "1") == 0) {
		printf("file tracking\n");
		struct passwd *pwd = calloc(1, sizeof(struct passwd));
		if (pwd == NULL){
        	fprintf(stderr, "Failed to allocate struct passwd for getpwnam_r.\n");
        	exit(1);
    		}

    		size_t buffer_len = sysconf(_SC_GETPW_R_SIZE_MAX) * sizeof(char);
 
   		char *buffer = malloc(buffer_len);

    		if (buffer == NULL) {
        		fprintf(stderr, "Failed to allocate buffer for getpwnam_r.\n");
        		exit(2);
		}

    		getpwnam_r(argv[2], pwd, buffer, buffer_len, &pwd);

    		if(pwd == NULL){
        		fprintf(stderr, "getpwnam_r failed to find requested entry.\n");
        		exit(3);
    		}	

    		printf("uid: %d\n", pwd->pw_uid);

    		free(pwd);
    		free(buffer);

		fprintf(fp, "%d", pwd->pw_uid);

	}

	else if (strcmp(argv[1], "2") == 0) {
		printf("process kill\n");
		printf("pid : %s\n", argv[2]);
		fprintf(fp, "%s", argv[2]);	// print to file 
		
	}

	else if (strcmp(argv[1], "3") == 0) {
		printf("hide from list, command : %s\n", argv[2]);
		fprintf(fp, "%s", argv[2]);	// print the command to file
	}

	return 0;
}
