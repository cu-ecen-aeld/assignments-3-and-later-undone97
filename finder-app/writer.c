#include <errno.h>
#include <syslog.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

openlog(NULL,0,LOG_USER);
if (argc!=3) { //invalid #arguments
	syslog(LOG_ERR,"Invalid number of arguments: %d",argc);
	return 1;
}

const char *filename = argv[1]; //filename
FILE *file = fopen(filename,"a");
int err=errno;
if (file==NULL) {
	//fprintf(stderr,"The value of error is %d after attempting to open file %s\n",errno,filename);
	if (err==2) {
		syslog(LOG_ERR,"ERRNO: 2 - No such file or directory");
		return 1;
	}
	else {
		syslog(LOG_ERR,"Error opening file %d, please see man page errno for more details",err);
		return 1;
	}
}
else {
	fprintf(file,"%s",argv[2]);
	syslog(LOG_DEBUG,"Writing %s to %s",argv[2],argv[1]);
	fclose(file);
	closelog();
	return 0;
} 

return 0;
}
