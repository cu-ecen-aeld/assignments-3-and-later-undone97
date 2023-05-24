#include "systemcalls.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
//#include <linux/stat.h>

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    int ret;
    ret=system(cmd);
    if (ret==1) {
        return true;
    }
    else {
        return false;
    }
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/

bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/
    //int ret,stat_loc;
    int err;
    pid_t pid;
    bool ret;
    pid=fork();
    if (pid==-1){
        fprintf(stderr,"Failed to create child process");
        return false;
    }
    //slicing the arguments to exclude the path command##################
    char *dest[count];
    for (i=1;i<count;i++){
        dest[i-1]=command[i];
    }
    //###################################################################
    err=execv(command[0],dest);
    if(err==-1){
        fprintf(stderr,"Error executing");
        fprintf(stdout,"Error executing");
        return false;
    }
    pid=wait(NULL);
    if (pid != 0){
        ret = true;
    }
    else{
        ret = false;
        fprintf(stderr,"No response from child process");
        fprintf(stdout,"No response from child process");

    }
    va_end(args);

    return ret;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/

/*File Open sequence and validation*/

openlog(NULL,0,LOG_USER);
pid_t pid;
bool ret;
if (fork()==0){
    int fd = open(outputfile,O_RDWR | O_CREAT,S_IRUSR | S_IWUSR);
    int err=errno;
    if (fd==-1) {
        //fprintf(stderr,"The value of error is %d after attempting to open file %s\n",errno,filename);
        if (err==2) {
            syslog(LOG_ERR,"ERRNO: 2 - No such file or directory");
            return false;
        }
        else {
            syslog(LOG_ERR,"Error opening file %d, please see man page errno for more details",err);
            return false;
        }
    }

    dup2(fd,1);
    close(fd);
    //slicing the arguments to exclude the path command##################
    char *dest[count];
    for (i=1;i<count;i++){
        dest[i-1]=command[i];
    }
    //###################################################################
    err=execv(command[0],dest);
    if(err==-1){
        return false;
    }
    pid=wait(NULL);
    if (pid != 0){
        ret = true;
    }
    else{
        ret = false;
        fprintf(stderr,"No response from child process");
    }
}

    va_end(args);

    return ret;
}
