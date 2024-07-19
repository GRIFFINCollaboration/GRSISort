#ifndef TREDIRECT_H
#define TREDIRECT_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

/////////////////////////////////////////////////////////////////
///
/// \class TRedirect
///
/// A simple class to redirect stdout and stderr to file(s). To
/// use it, create it, providing the path of the files stdout and
/// stderr should be redirected to, and an (optional) flag whether
/// to append to those files (default) or not.
/// If only one file is provided, both stdout and stderr are
/// redirected to it.
/// The redirection ends when the TRedirect object is destroyed.
///
/////////////////////////////////////////////////////////////////

class TRedirect {
public:
   TRedirect(const char* newOut, const char* newErr, bool append = true)
   {
      Redirect(newOut, newErr, append);
   }
   explicit TRedirect(const char* newOut, bool append = true)
   {
      Redirect(newOut, newOut, append);
   }

   ~TRedirect()
   {
      fflush(stdout);
      dup2(fStdOutFileDescriptor, fileno(stdout));
      fflush(stderr);
      dup2(fStdErrFileDescriptor, fileno(stderr));
   }

   TRedirect(const TRedirect&) = delete;
   TRedirect(TRedirect&&)      = delete;

   TRedirect& operator=(const TRedirect&) = delete;
   TRedirect& operator=(TRedirect&&)      = delete;

private:
   void Redirect(const char* newOut, const char* newErr, bool append)
   {
      fStdOutFileDescriptor = dup(fileno(stdout));
      fflush(stdout);
      int newStdOut = open(newOut, (append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      dup2(newStdOut, fileno(stdout));
      close(newStdOut);
      fStdErrFileDescriptor = dup(fileno(stderr));
      fflush(stderr);
      int newStdErr = open(newErr, (append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
      dup2(newStdErr, fileno(stderr));
      close(newStdErr);
   }

   int fStdOutFileDescriptor{0};
   int fStdErrFileDescriptor{0};
};

#endif
