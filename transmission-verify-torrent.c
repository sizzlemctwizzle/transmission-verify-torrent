#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define LINE_LEN 4096
#define BUF_LEN  512

/*
   Parse the output of "transmission-remote -l" to get
   the status of target torrent.
*/
int parseStatus(FILE * fp, int tid, char * statusOut)
{
  char line[LINE_LEN];
  int lid = -1;
  int done = 0;
  int statStart = -1;
  int statEnd = -1;
  char * status = NULL;
  char * stendptr = NULL;
 
  if (fgets(line, LINE_LEN, fp))
  {
    statStart = (int)(strstr(line, "Status") - line);
    statEnd = ((int)(strstr(line, "Name") - line));
  }
  else
  {
    exit(EXIT_FAILURE);
  }

  if (fgets(line, LINE_LEN, fp))
  {
    /* This line keeps throwing a crazy gcc warning for me */
    sscanf(line, "%d %d", &lid, &done);
  }

  if (lid != tid) 
  {
    exit(EXIT_FAILURE);
  }
  
  status = line;
  status += statStart;
  stendptr = status + (statEnd - statStart - 1);

  while(stendptr >= status && ' ' == *(--stendptr));
  stendptr[1] = '\0';

  strcpy(statusOut, status);

  return done;
}

int checkStatus(int tid, char * status)
{
  int fd[2];
  int done = 0;
  pid_t pid;
  char tidStr[BUF_LEN];

  snprintf(tidStr, BUF_LEN, "-t%d", tid);

  /* 
     Set up a pipe to redirect the stdout of the child process
     to the parent process.
  */
  pipe(fd);   
  pid = fork();

  if (pid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    execlp("transmission-remote", "transmission-remote", tidStr, "-l", NULL);
  }
  else {
    FILE * fp = NULL;
    close(fd[1]);
    dup2(fd[0], STDIN_FILENO);

    fp = fdopen(fd[0], "r");
    done = parseStatus(fp, tid, status);
    waitpid(pid, NULL, 0);
    close(fd[0]);
  }

  return done;
}

int main(int argc, char *argv[])
{
  int tid = 0;
  int done = 0;
  char status[BUF_LEN] = "Verifying";
  char cmdBuf[BUF_LEN];

  if (argc != 2)
  {
    exit(EXIT_FAILURE);
  }
  tid = atoi(argv[1]);

  if (tid <= 0) 
  {
    exit(EXIT_FAILURE);
  }

  /* Since we don't need the output of this call, we can be lazy and use a system call */
  snprintf(cmdBuf, BUF_LEN, "transmission-remote -t %d --verify > /dev/null", tid);
  system(cmdBuf);

  /* Keep checking status until verification is done */
  while (!strcmp(status, "Verifying"))
  {
    done = checkStatus(tid, status);
    /* Wait half a second between checks */
    nanosleep((struct timespec[]){{0, 500000000}}, NULL);
  }

  /* Make sure the torrent isn't stopped if unfinished */
  if (done < 100)
  {
    snprintf(cmdBuf, BUF_LEN, "transmission-remote -t %d -s > /dev/null", tid);
    system(cmdBuf);
    checkStatus(tid, status);
  }
  else
  {
    /* Always return this status if the torrent is verified and done */
    strcpy(status, "Complete");
  }

  /* 
     The only output of this program is the status of the torrent after it 
     has been verified. 
  */
  printf(status);

  return 0;
}
