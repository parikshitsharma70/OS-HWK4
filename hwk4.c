/*  
*  Name:       Parikshit Sharma 
*  Login:      SP_19_CPS536_12
*  Purpose:    Create chain of processes and use them to traverse through the locker problem.
*  Bug report: No bugs reported, and all features were implemented
*/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

int main(int argc, char * argv[]) {
  pid_t pid; 
  
  //Declare global variables
  int error; 
  int fd[2];
  int status = 1; 
  int i, r, nprocs; 
  int y = 0, k = 0, counter = 0, x = 0;

  //Error checking
  if ((argc != 3) || ((nprocs = atoi(argv[1])) <= 0)) {
    fprintf(stderr, "Usage: %s <n> <m>(and n divides m)\n", argv[0]);
    return 1;
  }
  
  //Assigning values from user input
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);
  
  //Checking if m is divisible by n
  if (m % n != 0) {
    fprintf(stderr, "m must be divisible by n \n");
    return 1;
  }

  int * lock = malloc(m * sizeof(int));
  
  //Connect stdout and stdin to pipes
  if (pipe(fd) == -1) {
    perror("Failed to create starting pipe");
    return 1;
  }

  //Handle the pipe dup failure
  if ((dup2(fd[0], STDIN_FILENO) == -1) ||
    (dup2(fd[1], STDOUT_FILENO) == -1)) {
    perror("Failed to connect pipe");
    return 1;
  }

  //Handle the pipe closing errors
  if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
    perror("Failed to close extra descriptors");
    return 1;
  }

  for (i = 1; i < nprocs; i++) {
    //Create the remaining child processes
    if (pipe(fd) == -1) {
      //Pipe creation error handling
      fprintf(stderr, "[%ld]:failed to create pipe %d: %s\n",
        (long) getpid(), i, strerror(errno));
      return 1;
    }
    if ((pid = fork()) == -1) {
      //Fork error handling
      fprintf(stderr, "[%ld]:failed to create child %d: %s\n",
        (long) getpid(), i, strerror(errno));
      return 1;
    }
    if (pid > 0) {
      //Enter parent process set stdout
      error = dup2(fd[1], STDOUT_FILENO);
    } else {
      //Enter child process set stdin
      x = 1; 
      error = dup2(fd[0], STDIN_FILENO);
    }

    //Pipe dup2 error handling
    if (error == -1) {
      fprintf(stderr, "[%ld]:failed to dup pipes for iteration %d: %s\n",
        (long) getpid(), i, strerror(errno));
      return 1;
    }

    //Handle pipe closing failure
    if ((close(fd[0]) == -1) || (close(fd[1]) == -1)) {
      fprintf(stderr, "[%ld]:failed to close extra descriptors %d: %s\n",
        (long) getpid(), i, strerror(errno));
      return 1;
    }

    //Exit from loop
    if (pid)
      break;
  }
  if (i == 1) {
    if (status == 1) {
      //Write to pipe
      write(STDOUT_FILENO, & status, sizeof(int));
      status += 1;

      //Reset locker
      for (k = 0; k < m; k++) {
        lock[k] = 0;
        write(STDOUT_FILENO, & lock[k], sizeof(int));
      }
    }
    while (status <= m + 1) {
      //Read from pipe number of iterations
      read(STDIN_FILENO, & status, sizeof(int));
      for (k = 0; k < m; k++) {
        //Read status of locker
        read(STDIN_FILENO, & lock[k], sizeof(int));
      }
      if (status < m + 1 && status != 1) {

        status += 1;
        //Writing status of locker
        write(STDOUT_FILENO, & status, sizeof(int));

        //Flipping the locker
        for (k = status - 1; k <= m; k += status) {
          if (lock[k] == 0){
              lock[k] = 1;
          }
          else lock[k] = 0;
        }

        for (k = 0; k < m; k++) {
          //Write locker status back to the pipe
          write(STDOUT_FILENO, & lock[k], sizeof(int));

        }

      }
    }
    if (status >= m + 1) {
      //Print status of locker at the end of last traversal through ring
      fprintf(stderr, "Following lockers are open at the end: \n");
      for (k = 0; k < m; k++) {
        //If locker is open
        if(lock[k]==0)
        fprintf(stderr, "%d \t", k+1);
      }
      
    fprintf(stderr, "\n");
    }

  } else {
    //Looping through the rest of the iterations
    while (status < m + 2) {
      //Read status and locker info from pipe
      read(STDIN_FILENO, & status, sizeof(int));
      status += 1;
      for (k = 0; k < m; k++) {
        read(STDIN_FILENO, & lock[k], sizeof(int));
      }
      //Write status back to pipe
      write(STDOUT_FILENO, & status, sizeof(int));
      
      //Flip the lockers
      for (k = status - 1; k <= m; k += status) {
        if (lock[k] == 0){
            lock[k] = 1;
        }
        else lock[k] = 0;
      }
      
      //Write locker info back to the pipe
      for (k = 0; k < m; k++) {
        write(STDOUT_FILENO, & lock[k], sizeof(int));
      }
      //Increment counter to run m/n number of iterations
      counter = counter + m / status;
    }
    if (status >= m + 2) {
      //Count how many lockers flipped by each process
      fprintf(stderr, "Proc %d flipped %d lockers.\n", i, counter);
      write(STDOUT_FILENO, & status, sizeof(int));
    }
  }
  return 0;
}