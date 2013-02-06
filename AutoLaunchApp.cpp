//http://stackoverflow.com/questions/4127567/linux-c-run-and-communicate-with-new-process
#include <unistd.h>
// Include some other things I forgot. See manpages.
#include <sys/wait.h>
#include <iostream>
int main()
{
  // Open two pipes for communication
  // The descriptors will be available to both
  // parent and child.
  int in_fd[2];
  int out_fd[2];

  pipe(in_fd);  // For child's stdin
  pipe(out_fd); // For child's stdout

  // Fork
  pid_t pid = fork();
  std::cout<<"pid:"<<pid;
  if (pid == 0)
    {
      // We're in the child
      close(out_fd[0]);
      dup2(out_fd[1], STDOUT_FILENO);
      close(out_fd[1]);

      close(in_fd[1]);
      dup2(in_fd[0], STDIN_FILENO);
      close(in_fd[0]);
      std::cout<<"woohoo";
      // Now, launch your child whichever way you want
      // see eg. man 2 exec for this.
      execv("/usr/bin/gweled",NULL);

      _exit(0); // If you must exit manually, use _exit, not exit.
      // If you use exec, I think you don't have to. Check manpages.
    }

  else if (pid == -1)
    ; // Handle the error with fork

  else
    {
      std::cout<<"in parent";
      // You're in the parent
      close(out_fd[1]);
      close(in_fd[0]);

      // Now you can read child's stdout with out_fd[0]
      // and write to its stdin with in_fd[1].
      // See man 2 read and man 2 write.

      // ...

      // Wait for the child to terminate (or it becomes a zombie)
      int status ;
      //      waitpid(pid, &status, 0);

        // see man waitpid for what to do with status
    } 
}
