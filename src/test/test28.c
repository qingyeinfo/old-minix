 /* test28: mkdir() rmdir()	Author: Jan-Mark Wams (jms@cs.vu.nl) */

/*
** Not tested readonly file systems (EROFS.)
** Not tested fs full (ENOSPC.)
** Not really tested EBUSY.
** Not tested unlinking busy directories.
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <stdio.h>

#define MAX_ERROR	4
#define ITERATIONS      2

#define DIRENT0		((struct dirent *) NULL)

#define System(cmd)	if (system(cmd) != 0) printf("``%s'' failed\n", cmd)
#define Chdir(dir)	if (chdir(dir) != 0) printf("Can't goto %s\n", dir)

int errct = 0;
int subtest = 1;
int superuser;
char MaxName[NAME_MAX + 1];	/* Name of maximum length */
char MaxPath[PATH_MAX];
char ToLongName[NAME_MAX + 2];	/* Name of maximum +1 length */
char ToLongPath[PATH_MAX + 1];

_PROTOTYPE(void main, (int argc, char *argv[]));
_PROTOTYPE(void test28a, (void));
_PROTOTYPE(void test28c, (void));
_PROTOTYPE(void test28b, (void));
_PROTOTYPE(void makelongnames, (void));
_PROTOTYPE(void e, (int n));
_PROTOTYPE(void quit, (void));

void main(argc, argv)
int argc;
char *argv[];
{
  int i, m = 0xFFFF;

  sync();
  if (argc == 2) m = atoi(argv[1]);
  printf("Test 28 ");
  fflush(stdout);
  superuser = (getuid() == 0);
  makelongnames();
  system("chmod 777 DIR_28/* DIR_28/*/* > /dev/null 2>&1");
  System("rm -rf DIR_28; mkdir DIR_28");
  Chdir("DIR_28");
  umask(0000);			/* no umask */

  for (i = 0; i < ITERATIONS; i++) {
	if (m & 0001) test28a();
	if (m & 0002) test28b();
	if (m & 0004) test28c();
  }
  quit();
}


void test28a()
{
  int mode;			/* used in for loop */
  struct stat st;
  time_t time1, time2;
  DIR *dirp;
  struct dirent *dep;
  int dot = 0, dotdot = 0;

  subtest = 1;

  System("rm -rf foo /tmp/foo");/* clean up junk */

  /* Check relative path names */
  if (mkdir("./foo", 0777) != 0) e(1);	/* make a dir foo */
  if (mkdir("./foo/bar", 0777) != 0) e(2);	/* make foo/bar */
  if (rmdir("foo/bar") != 0) e(3);	/* delete bar */
  if (mkdir("foo/../foo/bar", 0777) != 0) e(4);	/* make bar again */
  if (rmdir("./foo/bar") != 0) e(5);	/* and remove again */

  /* Foo should be empty (ie. contain only "." and ".." */
  if ((dirp = opendir("foo")) == (DIR *) NULL) e(6);	/* open foo */
  if ((dep = readdir(dirp)) == DIRENT0) e(7);	/* get first entry */
  if (strcmp(dep->d_name, ".") == 0) dot += 1;	/* record what it is */
  if (strcmp(dep->d_name, "..") == 0) dotdot += 1;
  if ((dep = readdir(dirp)) == DIRENT0) e(8);	/* get second entry */
  if (strcmp(dep->d_name, ".") == 0) dot += 1;	/* record again */
  if (strcmp(dep->d_name, "..") == 0) dotdot += 1;
  if ((dep = readdir(dirp)) != DIRENT0) e(9);	/* no 3d entry */
  if (dot == 1 && dotdot != 1) e(10);	/* only . and .. */
  if (closedir(dirp) != 0) e(11);	/* close foo */
  if (rmdir("./foo") != 0) e(12);	/* remove dir foo */

  /* Check absolute path names */
  if (mkdir("/tmp/foo", 0777) != 0) e(13);
  if (mkdir("/tmp/foo/bar", 0777) != 0) e(14);
  if (rmdir("/tmp/foo/bar") != 0) e(15);	/* make some dirs */
  if (rmdir("/tmp/foo") != 0) e(16);

  /* Check the mode arument for mkdir() */
  for (mode = 0; mode <= 0777; mode++) {
	if (mkdir("foo", mode) != 0) e(17);	/* make foo */
	if (stat("foo", &st) != 0) e(18);
	if ((st.st_mode & 0777) != mode) e(19);	/* check it's mode */
	if (rmdir("foo") != 0) e(20);	/* and remove it */
  }

  /* Check the stat */
  time(&time1);
  while (time1 >= time((time_t *)0))
	;
  if (mkdir("foo", 0765) != 0) e(21);	/* make foo */
  if (stat("foo", &st) != 0) e(22);
  time(&time2);
  while (time2 >= time((time_t *)0))
	;
  time(&time2);
  if (st.st_nlink != 2) e(23);
  if (st.st_uid != geteuid()) e(24);
  if (st.st_gid != getegid()) e(25);
  if (st.st_size < 0) e(26);
  if ((st.st_mode & 0777) != 0765) e(27);
  if (st.st_atime <= time1) e(28);
  if (st.st_atime >= time2) e(29);
  if (st.st_ctime <= time1) e(30);
  if (st.st_ctime >= time2) e(31);
  if (st.st_mtime <= time1) e(32);
  if (st.st_mtime >= time2) e(33);

  /* Check if parent is updated */
  if (stat(".", &st) != 0) e(34);
  time(&time2);
  while (time2 >= time((time_t *)0))
	;
  time(&time2);
  if (st.st_ctime <= time1) e(35);
  if (st.st_ctime >= time2) e(36);
  if (st.st_mtime <= time1) e(37);
  if (st.st_mtime >= time2) e(38);
  time(&time1);
  while (time1 >= time((time_t *)0))
	;
  if (rmdir("foo") != 0) e(39);
  if (stat(".", &st) != 0) e(40);
  time(&time2);
  while (time2 >= time((time_t *)0))
	;
  time(&time2);
  if (st.st_ctime <= time1) e(41);
  if (st.st_ctime >= time2) e(42);
  if (st.st_mtime <= time1) e(43);
  if (st.st_mtime >= time2) e(44);
}


void test28b()
{				/* Test critical values. */
  struct stat st;
  DIR *dirp;
  struct dirent *dep;
  int fd;			/* file descriptor */
  int other = 0, dot = 0, dotdot = 0;	/* dirent counters */
  int rmdir_result;		/* tmp var */
  nlink_t nlink;
  char *bar = "foo/bar.xxx";
  int stat_loc;

  subtest = 2;

  System("rm -rf ../DIR_28/*");

  /* Check funny but valid path names */
  if (mkdir("/../../..////.//../tmp/foo/", 0777) != 0) e(1);
  if (mkdir("/tmp/foo//////..//foo//../foo/bar/", 0777) != 0) e(2);
  if (rmdir("///tmp/..//tmp/foo/bar//../..//foo/bar") != 0) e(3);
  if (mkdir("///tmp/foo/foobar//", 0777) != 0) e(4);
  if (rmdir("/tmp/foo/foobar//") != 0) e(5);
  if (rmdir("/.././/././/tmp/foo///////////////") != 0) e(6);
  if (rmdir("/tmp/foo") != -1) e(7);	/* try again */

  if (LINK_MAX >= 999) e(8);	/* "xxx" long inough */

  /* Test max path ed. */
  if (mkdir(MaxName, 0777) != 0) e(9);	/* make dir MaxName */
  if (rmdir(MaxName) != 0) e(10);	/* and remove it */
  MaxPath[strlen(MaxPath) - 2] = '/';	/* convert MaxPath */
  MaxPath[strlen(MaxPath) - 1] = 'a';	/* to ././.../a */
  if (mkdir(MaxPath, 0777) != 0) e(11);	/* it should be */
  if (rmdir(MaxPath) != 0) e(12);	/* ok */

  /* Test too long path ed. */
  if (mkdir(ToLongName, 0777) != 0) e(17);	/* Try ToLongName */
  if (rmdir(ToLongName) != 0) e(18);	/* and remove it */
  ToLongPath[strlen(ToLongPath) - 2] = '/';	/* make ToLongPath */
  ToLongPath[strlen(ToLongPath) - 1] = 'a';	/* contain ././.../a */
  if (mkdir(ToLongPath, 0777) != -1) e(19);	/* it should */
  if (errno != ENAMETOOLONG) e(20);	/* not be ok */
  if (rmdir(ToLongPath) != -1) e(21);
  if (errno != ENAMETOOLONG) e(22);

  /* Test what happens if the parent link count > LINK_MAX. */
  if (mkdir("foo", 0777) != 0) e(23);	/* make foo */
  for (nlink = 2; nlink < LINK_MAX; nlink++) {	/* make all */
	bar[8] = (char) ((nlink / 100) % 10) + '0';
	bar[9] = (char) ((nlink / 10) % 10) + '0';
	bar[10] = (char) (nlink % 10) + '0';
	if (mkdir(bar, 0777) != 0) e(24);
  }
  if (stat("foo", &st) != 0) e(25);	/* foo now */
  if (st.st_nlink != LINK_MAX) e(26);	/* is full */
  if (mkdir("foo/nono", 0777) != -1) e(27);	/* no more */
  if (errno != EMLINK) e(28);	/* entrys. */
  System("rm -rf foo/nono");	/* Just in case. */

  /* Test if rmdir removes only empty dirs */
  if (rmdir("foo") != -1) e(29);/* not empty */
  if (errno != EEXIST && errno != ENOTEMPTY) e(30);

  /* Test if rmdir removes a dir with an empty file (it shouldn't.) */
  System("rm -rf foo");		/* cleanup */
  if (mkdir("foo", 0777) != 0) e(31);
  System("> foo/empty");	/* > empty */
  if (rmdir("foo") != -1) e(32);/* not empty */
  if (errno != EEXIST && errno != ENOTEMPTY) e(33);
  if (unlink("foo/empty") != 0) e(34);	/* rm empty */

  /* See what happens if foo is linked. */
  if (superuser) {
	if (link("foo", "footoo") != 0) e(35);	/* foo still */
	if (rmdir("footoo") != 0) e(36);	/* exist */
	if (chdir("footoo") != -1) e(37);	/* footoo */
	if (errno != ENOENT) e(38);	/* is gone */
  }
#ifdef _MINIX
  /* Some implementations might allow users to link directories. */
  if (!superuser) {
	if (link("foo", "footoo") != -1) e(39);
	if (errno != EPERM) e(40);
	if (unlink("foo") != -1) e(41);
	if (errno != EPERM) e(42);
  }
#endif

  /* See if ".." and "." are removed from the dir, and if it is
   * unwriteable * Note, we can not remove any files in the PARENT
   * process, because this * will make readdir unpredicatble. (see
   * 1003.1 page 84 line 30.) However * removal of the directory is
   * not specified in the standard.
   */
  System("rm -rf /tmp/sema[12].07");
  switch (fork()) {
      case -1:	printf("Can't fork\n");	break;

      case 0:
	alarm(20);
	if ((fd = open("foo", O_RDONLY)) <= 2) e(43);	/* open */
	if ((dirp = opendir("foo")) == (DIR *) NULL) e(44);	/* opendir */
	/* UpA downB */
	system(">/tmp/sema1.07; while test -f /tmp/sema1.07; do sleep 1;done");
	while ((dep = readdir(dirp)) != DIRENT0) {
		if (strcmp(dep->d_name, "..") == 0)
			dotdot += 1;
		else if (strcmp(dep->d_name, ".") == 0)
			dot += 1;
		else
			other += 1;
	}
	if (dotdot != 0) e(45);	/* no entrys */
	if (dot != 0) e(46);	/* shoul be */
	if (other != 0) e(47);	/* left or */

	/* No new files (entrys) are allowed on foo */
	if (creat("foo/nono", 0777) != -1) e(48);	/* makeable */
	if (closedir(dirp) != 0) e(49);	/* close foo */
	system("while test ! -f /tmp/sema2.07; do sleep 1; done");  /* downA */
	System("rm -f /tmp/sema2.07");	/* clean up */

	/* Foo still exist, so we should be able to get a fstat */
	if (fstat(fd, &st) != 0) e(50);
	if (st.st_nlink != (nlink_t) 0) e(51);	/* 0 left */
	if (close(fd) != 0) e(52);	/* last one */
	exit(0);

      default:
	system("while test ! -f /tmp/sema1.07; do sleep 1; done");  /* downA */
	if (rmdir("foo") != 0) e(53);	/* cleanerup */
	System("rm -f /tmp/sema1.07");	/* upB */
	if (chdir("foo") != -1) e(54);	/* it should */
	if (errno != ENOENT) e(55);	/* be gone */
	System("> /tmp/sema2.07");	/* upA */
	if (wait(&stat_loc) == -1) e(56);
	if (stat_loc != 0) e(57);
  }

  /* See if foo isn't accessible any more */
  if (chdir("foo") != -1) e(58);
  if (errno != ENOENT) e(59);

  /* Let's see if we can get a EBUSSY..... */
  if (mkdir("foo", 0777) != 0) e(60);	/* mkdir foo */
  System("rm -f /tmp/sema.07");	/* unness */
  switch (fork()) {
      case -1:	printf("Can't fork\n");	break;
      case 0:
	alarm(20);
	if (chdir("foo") != 0) e(61);	/* child goes */
	System("> /tmp/sema.07");	/* upA */
	system("while test -f /tmp/sema.07; do sleep 1; done");	/* downB */
	sleep(1);
	exit(0);
      default:
	system("while test ! -f /tmp/sema.07; do sleep 1; done");   /* downA */
	rmdir_result = rmdir("foo");	/* try remove */
	if (rmdir_result == -1) {	/* if it failed */
		if (errno != EBUSY) e(62);	/* foo is busy */
	} else {
		if (rmdir_result != 0) e(63);
		if (rmdir("foo") != -1) e(64);	/* not removable */
		if (errno != ENOENT) e(65);	/* again. */
		if (chdir("foo") != -1) e(66);	/* we can't go */
		if (errno != ENOENT) e(67);	/* there any more */
		if (mkdir("foo", 0777) != 0) e(68);	/* we can remake foo */
	}
	System("rm -f /tmp/sema.07");	/* upB */
	if (wait(&stat_loc) == -1) e(69);
	if (stat_loc != 0) e(70);
  }
  if (rmdir("foo") != 0) e(71);	/* clean up */
}

void test28c()
{				/* Test error handeling. */
  subtest = 3;

  System("rm -rf ../DIR_28/*");
  System("rm -rf foo /tmp/foo");/* clean up junk */

  /* Test common errors */
  if (mkdir("foo", 0777) != 0) e(1);	/* mkdir shouldn't fail */
  if (mkdir("foo", 0777) != -1) e(2);	/* should fail the 2d time */
  if (errno != EEXIST) e(3);	/* because it exists already */
  if (rmdir("foo") != 0) e(4);	/* rmdir shouldn't fail */
  if (rmdir("foo") != -1) e(5);	/* but it should now because */
  if (errno != ENOENT) e(6);	/* it's gone the 1st time */
  /* Test on access etc. */
  if (mkdir("foo", 0777) != 0) e(7);
  if (mkdir("foo/bar", 0777) != 0) e(8);
  if (!superuser) {
	System("chmod 677 foo");/* make foo inaccesable */
	if (mkdir("foo/foo", 0777) != -1) e(9);
	if (errno != EACCES) e(10);
	if (rmdir("foo/bar") != -1) e(11);
	if (errno != EACCES) e(12);
	System("chmod 577 foo");/* make foo unwritable */
	if (mkdir("foo/foo", 0777) != -1) e(13);
	if (errno != EACCES) e(14);
	if (rmdir("foo/bar") != -1) e(15);
	if (errno != EACCES) e(16);
	System("chmod 777 foo");/* make foo full accessable */
  }
  if (rmdir("foo/bar") != 0) e(17);	/* bar should be removable */
  if (mkdir("foo/no/foo", 0777) != -1) e(18);	/* Note: "no" doesn't exist */
  if (errno != ENOENT) e(19);
  if (mkdir("", 0777) != -1) e(20);	/* empty string isn't ok */
  if (errno != ENOENT) e(21);
  if (rmdir("") != -1) e(22);	/* empty string isn't ok */
  if (errno != ENOENT) e(23);
  System("> foo/no");		/* make a file "no" */
  if (mkdir("foo/no/foo", 0777) != -1) e(24);
  if (errno != ENOTDIR) e(25);	/* note: "no" is not a a dir */
  if (rmdir("foo/no/foo") != -1) e(26);
  if (errno != ENOTDIR) e(27);
  System("rm -rf foo");		/* clean up */
}


void makelongnames()
{
  register int i;

  memset(MaxName, 'a', NAME_MAX);
  MaxName[NAME_MAX] = '\0';
  for (i = 0; i < PATH_MAX - 1; i++) {	/* idem path */
	MaxPath[i++] = '.';
	MaxPath[i] = '/';
  }
  MaxPath[PATH_MAX - 1] = '\0';

  strcpy(ToLongName, MaxName);	/* copy them Max to ToLong */
  strcpy(ToLongPath, MaxPath);

  ToLongName[NAME_MAX] = 'a';
  ToLongName[NAME_MAX + 1] = '\0';	/* extend ToLongName by one too many */
  ToLongPath[PATH_MAX - 1] = '/';
  ToLongPath[PATH_MAX] = '\0';	/* inc ToLongPath by one */
}


void e(n)
int n;
{
  int err_num = errno;		/* Save in case printf clobbers it. */

  printf("Subtest %d,  error %d  errno=%d: ", subtest, n, errno);
  errno = err_num;
  perror("");
  if (errct++ > MAX_ERROR) {
	printf("Too many errors; test aborted\n");
	chdir("..");
	system("rm -rf DIR*");
	exit(1);
  }
  errno = 0;
}


void quit()
{
  Chdir("..");
  System("rm -rf DIR_28");

  if (errct == 0) {
	printf("ok\n");
	exit(0);
  } else {
	printf("%d errors\n", errct);
	exit(1);
  }
}