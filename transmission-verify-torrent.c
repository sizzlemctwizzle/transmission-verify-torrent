#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

#define MILLI_WAIT 500
#define LINE_LEN   4096
#define BUF_LEN    512
#define ARGS_NUM   16
#define ARG_LEN    128

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
    /* 
       If we got a bad response from "transmission-remote -l"
       then print what ever response we got.
    */
    fprintf(stderr, "Unable to parse torrent list:\n\n");
    do
    {
        fprintf(stderr, "%s\n", line);
    } while (fgets(line, LINE_LEN, fp));
    exit(EXIT_FAILURE);
  }

  #if SINGLE_TORRENT
  if (
  #else
  while (lid != tid &&
  #endif
    fgets(line, LINE_LEN, fp))
  {
    sscanf(line, "%d %d", &lid, &done);
  }

  if (lid != tid) 
  {
    fprintf(stderr, "Torrent id could not be found\n");
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

/* Build the arguments array required for execvp */
char ** buildArgsArr(char * transStr, int tid)
{
  /* Since everything is static this only runs once */
  static char args[ARGS_NUM][ARG_LEN] = {{0}};
  static char * rval[ARGS_NUM] = {0};
  static int argi = 0;

  if (argi <= 0)
  {
    char * arg = NULL;
    int cpi;
    arg = strtok(transStr," ");

    while (arg != NULL && argi < ARGS_NUM - 2)
    {
      strcpy(args[(argi++)], arg);
      arg = strtok (NULL, " ");
    }

    /* These are the torrent id and list args */
    #if SINGLE_TORRENT
    snprintf(args[(argi++)], ARG_LEN, "-t%d", tid);
    #endif
    strcpy(args[(argi++)], "-l");
    
    /* Copy the pointers to the strings */
    for (cpi = 0; cpi < argi; ++cpi) rval[cpi] = args[cpi];
  }
  
  return rval;
}

int checkStatus(int tid, char * status, char * transStr)
{
  int fd[2];
  int done = 0;
  pid_t pid;

  /* 
     Set up a pipe to redirect the stdout of the child process
     to the parent process.
  */
  pipe(fd);   
  pid = fork();

  if (pid == 0) {
    dup2(fd[1], STDOUT_FILENO);
    execvp("transmission-remote", buildArgsArr(transStr, tid));
  }
  else
  {
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
  char transBuf[BUF_LEN] = "transmission-remote ";
  char cmdBuf[BUF_LEN];

  /* Usage: transmission-verify-torrent [host] torrent-id [options] */
  if (argc < 2)
  {
    fprintf(stderr, "Not enough arguments\n");
    exit(EXIT_FAILURE);
  }
  else if (argc == 2)
  {
    tid = atoi(argv[1]);
  }
  else
  {
    strcat(transBuf, argv[1]); /* host param */
    tid = atoi(argv[2]);

    /* options */
    if (argc > 3)
    {
      int i;
      char * lastOpt = NULL;
      for (i = 3; i < argc; ++i) /* loop through remaining args */
      {
        char * opt = argv[i];
        if (*(opt++) == '-' && *opt != '\0')
        {
          if (!strcmp("ne", opt) || !strcmp("-authenv", opt))
          {
            strcat(transBuf, " -ne");
          }
          else if (!strcmp("n", opt) || !strcmp("-auth", opt))
          {
            strcat(transBuf, " -n");
            lastOpt = opt;
          }
        }
        else if (lastOpt)
        {
          strcat(transBuf, " ");
          strcat(transBuf, (--opt));
          lastOpt = NULL;
        }
      }
    }
  }

  if (tid <= 0) 
  {
    fprintf(stderr, "Torrent id is invalid\n");
    exit(EXIT_FAILURE);
  }

  /* Since we don't need the output of this call, we can be lazy and use a system call */
  snprintf(cmdBuf, BUF_LEN, "%s -t %d --verify > /dev/null", transBuf, tid);
  system(cmdBuf);

  /* Keep checking status until verification is done */
  while (!strcmp(status, "Verifying") || !strcmp(status, "Will Verify"))
  {
    done = checkStatus(tid, status, transBuf);
    /* Wait between status checks */
    nanosleep((struct timespec[]){{0, MILLI_WAIT * 1000000}}, NULL);
  }

  /* Make sure the torrent isn't stopped if unfinished */
  if (done < 100)
  {
    snprintf(cmdBuf, BUF_LEN, "%s -t %d -s > /dev/null", transBuf, tid);
    system(cmdBuf);
    checkStatus(tid, status, transBuf);
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
  printf("%s", status);

  return 0;
}
