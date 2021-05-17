/*
 * A simple program to measure the performance of some of
 * the most commonly used system calls.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/time.h>

#define	DIR1		"./DIR1"
#define	DIR2		"./DIR1/DIR2"
#define	FILE1		"./DIR1/DIR2/FILE1"
#define	FILE2		"./DIR1/DIR2/FILE2"
#define	DO_NOTHING	"./do_nothing"
#define	GET_KILLED	"./get_killed"
#define LARGE_FILE	"./LARGE_FILE"
#define LARGE_OFFSET	4000000
#define	MULTIPLIER	10000

#define fatal(fmt, ...) \
	do { \
		fprintf(stderr, fmt ": %s\n", ##__VA_ARGS__, strerror(errno)); \
		exit(1); \
	} while (0)

#define TIMER_START() \
	do { \
		if (gettimeofday(&tv1, NULL) == -1) \
			fatal("gettimeofday: tv1"); \
	} while (0)

#define TIMER_STOP() \
	do { \
		if (gettimeofday(&tv2, NULL) == -1) \
			fatal("gettimeofday: tv2"); \
		timersub(&tv2, &tv1, &tv); \
	} while (0)
	
#define TIMER_DIFF_MS()	(tv.tv_sec * 1000 + tv.tv_usec / 1000)

char *pgname;
char *pgversion = "1.1";
int fflag;
int vflag;
int lflag;

struct timeval tv1, tv2, tv;

struct test {
	void (*tst_func)(int);
	int tst_count;
	char *tst_info;
};

void usage(void);
void test01(int);
void test02(int);
void test03(int);
void test04(int);
void test05(int);
void test06(int);
void test07(int);
void test08(int);
void test09(int);
void test10(int);
void test11(int);
void test12(int);
void test13(int);
void test14(int);
void test15(int);
void test16(int);
void test17(int);
void test18(int);
void test19(int);
void cleanup(void);


void usage(void)
{
	printf("%s v%s\n", pgname, pgversion);
	printf("Usage: %s [-fhlv][-m <count>]\n", pgname);
	printf("-f:	do not test the fork system call\n");
	printf("-h:	print help menu\n");
	printf("-l:	list tests and their number of iterations\n");
	printf("-m:	use next arg as iteration multiplier (default %d)\n",
		MULTIPLIER);
	printf("-v:	enable verbose mode\n");
	exit(1);
}


int main(int argc, char *argv[])
{
	char *cp;
	int multiplier;
	struct test *tp;
	struct test testfunc[] = {
		{ test01,     5, "mkdir, rmdir" },
		{ test02,    17, "creat, close, open" },
		{ test03,   150, "chdir" },
		{ test04,    88, "chown" },
		{ test05,   480, "lseek" },
		{ test06,   410, "read" },
		{ test07,    25, "link unlink" },
		{ test08,   250, "stat" },
		{ test09,    88, "write" },
		{ test10,     1, "fork, wait, exit" },
		{ test11,     1, "fork, wait, exec" },
		{ test12,   500, "getpid" },
		{ test13,  1120, "getuid" },
		{ test14,   300, "setuid" },
		{ test15,   210, "signal" },
		{ test16,   490, "kill" },
		{ test17,    50, "pipe" },
		{ test18,   310, "sbrk" },
		{ test19,   920, "syscall(getpid)" },
		{ 0,          0, "NULL" },
	};

	pgname = *argv;
	multiplier = MULTIPLIER;
	while (--argc > 0) {
		if (**++argv != '-') {
			printf("%c: invalid argument\n", **argv);
			usage();
		}
		for (cp = *argv+1; *cp; cp++) {
			switch (*cp) {
			case 'f':
				fflag++;
				break;
			case 'h':
				usage();
			case 'l':
				lflag++;
				break;
			case 'm':
				if (--argc <= 0)
					usage();
				multiplier = atoi(*++argv);
				break;
			case 'v':
				vflag++;
				break;
			default:
				printf("%c: invalid flag\n", *cp);
				usage();
			}
		}
	}

	if (lflag) {
		for (tp = testfunc; tp->tst_func; tp++) {
			if (vflag)
				printf("%-20s %5d\n", tp->tst_info, tp->tst_count);
			else
				printf("%-20s\n", tp->tst_info);
		}
		exit(0);
	}

	/* a large file for read and lseek */
	if (access(LARGE_FILE, R_OK) == -1) {
		perror(LARGE_FILE);
		exit(1);
	}

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	cleanup();
	for (tp = testfunc; tp->tst_func; tp++) {
		if (fflag && !strncmp(tp->tst_info, "fork", 4))
			continue;
		(*tp->tst_func)(tp->tst_count * multiplier);
	}
	cleanup();

	exit(0);
}


/*
 * mkdir and rmdir.
 */
void test01(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (mkdir(DIR1, 0755) == -1)
			fatal("mkdir: %s", DIR1);
		if (rmdir(DIR1) == -1)
			fatal("rmdir: %s", DIR1);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d mkdirs, %10d rmdirs\n", TIMER_DIFF_MS(), test_cnt, test_cnt);
}


/*
 * creat, close and open.
 */
void test02(int test_cnt)
{
	int i;
	int fd;
	struct timeval tv, tv1, tv2;

	if (mkdir(DIR1, 0755) == -1)
		fatal("mkdir: %s", DIR1);
	if (mkdir(DIR2, 0755) == -1)
		fatal("mkdir: %s", DIR2);

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if ((fd = creat(FILE1, 0644)) == -1)
			fatal("creat: %s", FILE1);
		if (close(fd) == -1)
			fatal("close: %s", FILE1);
		if ((fd = open(FILE1, 0)) == -1)
			fatal("open: %s", FILE1);
		if (close(fd) == -1)
			fatal("close: %s", FILE1);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d creats, %10d opens, %10d closes\n",
		TIMER_DIFF_MS(), test_cnt, test_cnt, test_cnt * 2);
}


/*
 * chdir.
 */
void test03(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (chdir(DIR2) == -1)
			fatal("chdir: %s", DIR2);
		if (chdir("../..") == -1)
			fatal("chdir: ../..");
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d chdirs\n", TIMER_DIFF_MS(), test_cnt * 2);
}


/*
 * chown.
 */
void test04(int test_cnt)
{
	int i;
	int fd;
	int uid, gid;

	if ((fd = creat(FILE1, 0644)) == -1)
		fatal("creat: %s", FILE1);
	if (close(fd))
		fatal("close: %s", FILE1);

	uid = getuid();
	gid = getgid();

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (chown(FILE1, uid, gid) == -1)
			fatal("chown: %s", FILE1);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d chowns\n", TIMER_DIFF_MS(), test_cnt);
}


/*
 * lseek.
 */
void test05(int test_cnt)
{
	int i;
	int fd;

	if ((fd = open(LARGE_FILE, 0)) == -1)
		fatal("open: %s", LARGE_FILE);

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (lseek(fd, (off_t)LARGE_OFFSET, SEEK_DATA) == (off_t)-1)
			fatal("lseek: %d", LARGE_OFFSET);
		if (lseek(fd, (off_t)0, SEEK_DATA) == (off_t)-1)
			fatal("lseek: 0");
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d lseeks\n", TIMER_DIFF_MS(), test_cnt * 2);

	(void)close(fd);
}


/*
 * read.
 */
void test06(int test_cnt)
{
	char c;
	int i;
	int fd;

	if ((fd = open(LARGE_FILE, 0)) == -1)
		fatal("open: LARGE_FILE");

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (read(fd , &c, 1) == -1)
			fatal("read: LARGE_FILE");
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d reads(1 byte)\n", TIMER_DIFF_MS(), test_cnt);

	(void)close(fd);
}


/*
 * link and unlink.
 */
void test07(int test_cnt)
{
	int i;

	(void)unlink(FILE2);

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (link(FILE1, FILE2) == -1)
			fatal("link: %s", FILE2);
		if (unlink(FILE2) == -1)
			fatal("unlink: %s", FILE2);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d links, %10d unlinks\n", TIMER_DIFF_MS(), test_cnt, test_cnt);
}


/*
 * stat.
 */
void test08(int test_cnt)
{
	int i;
	struct stat	buf;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (stat(FILE1, &buf) == -1)
			fatal("stat: %s", FILE1);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d stats\n", TIMER_DIFF_MS(), test_cnt);
}


/*
 * write.
 */
void test09(int test_cnt)
{
	int i;
	int fd;
	char c;

	if ((fd = open(FILE1, 1)) == -1)
		fatal("open: %s", FILE1);

	TIMER_START();
	c = 0;
	for (i = 0; i < test_cnt; i++) {
		if (write(fd, &c, 1) == -1)
			fatal("write: %s", FILE1);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d writes(1 byte)\n", TIMER_DIFF_MS(), test_cnt);

	(void)close(fd);
	(void)unlink(FILE1);
}


/*
 * fork, wait and exit.
 */
void test10(int test_cnt)
{
	int i;
	int pid;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if ((pid = fork()) == -1)
			fatal("fork: ");
		if (pid != 0)
			(void)wait((int *)0);
		else
			exit(0);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d forks, %10d exits, %10d waits\n",
		TIMER_DIFF_MS(), test_cnt, test_cnt, test_cnt);
}


/*
 * fork, wait and exec.
 */
void test11(int test_cnt)
{
	int i;
	int pid;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if ((pid = fork()) == -1)
			fatal("fork: ");
		if (pid != 0)
			(void)wait((int *)0);
		else {
			execl(DO_NOTHING, DO_NOTHING, NULL);
			fatal("execl: %s", DO_NOTHING);
		}
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d forks, %10d execs, %10d waits\n",
		TIMER_DIFF_MS(), test_cnt, test_cnt, test_cnt);
}


/*
 * getpid.
 */
void test12(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++)
		getpid();
	TIMER_STOP();
	printf("%6ld msecs: %10d getpids\n", TIMER_DIFF_MS(), test_cnt);
}


/*
 * getuid.
 */
void test13(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++)
		getuid();
	TIMER_STOP();
	printf("%6ld msecs: %10d getuids\n", TIMER_DIFF_MS(), test_cnt);
}


/*
 * setuid.
 */
void test14(int test_cnt)
{
	int i;
	int uid;

	uid = getuid();

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (setuid(uid) == -1)
			fatal("setuid: %d", uid);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d setuids\n", TIMER_DIFF_MS(), test_cnt);
}


/*
 * signal.
 */
void test15(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (signal(SIGHUP, (void (*)())0) == (void (*)())-1)
			fatal("signal: SIGHUP");
		if (signal(SIGQUIT, (void (*)())1) == (void (*)())-1)
			fatal("signal: SIGQUIT");
		if (signal(SIGILL, (void (*)())0) == (void (*)())-1)
			fatal("signal: SIGILL");
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d signals\n", TIMER_DIFF_MS(), test_cnt * 3);
}


/*
 * kill.
 */
void test16(int test_cnt)
{
	int i;
	int pid;

	while ((pid = fork()) == -1)
		;
	if (pid) {
		usleep(10);

		TIMER_START();
		for (i = 0; i < test_cnt; i++) {
			if (kill(pid, SIGHUP) == -1)
				fatal("kill: SIGHUP");
		}
		TIMER_STOP();
		printf("%6ld msecs: %10d kills\n", TIMER_DIFF_MS(), test_cnt);

		if (kill(pid, SIGINT) == -1)
			fatal("kill: SIGINT");
	} else {
		execl(GET_KILLED, GET_KILLED, NULL);
		printf("cannot execl %s\n", GET_KILLED);
	}
}


/*
 * pipe.
 */
void test17(int test_cnt)
{
	int i;
	int pfd[2];

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (pipe(pfd) == -1)
			fatal("pipe: ");
		if (close(pfd[0]) == -1)
			fatal("close: 0");
		if (close(pfd[1]) == -1)
			fatal("close: 1");
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d pipes, %10d closes\n", TIMER_DIFF_MS(), test_cnt, test_cnt * 2);
}


/*
 * sbrk.
 */
void test18(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++) {
		if (sbrk(i) == (void *)-1)
			fatal("sbrk: %d", i);
		if (sbrk(-i) == (void *)-1)
			fatal("sbrk: %d", -i);
	}
	TIMER_STOP();
	printf("%6ld msecs: %10d brks\n", TIMER_DIFF_MS(), test_cnt * 2);
}


/*
 * syscall(getpid).
 */
void test19(int test_cnt)
{
	int i;

	TIMER_START();
	for (i = 0; i < test_cnt; i++)
		syscall(SYS_getpid);
	TIMER_STOP();
	printf("%6ld msecs: %10d syscall(getpid)s\n", TIMER_DIFF_MS(), test_cnt);
}


void cleanup(void)
{
	(void)unlink(FILE2);
	(void)unlink(FILE1);
	(void)rmdir(DIR2);
	(void)rmdir(DIR1);
}
