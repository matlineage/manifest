/* Taken from https://github.com/sabotage-linux/sabotage */
/*   KEEP/super_chroot.c */
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <stdio.h>
#include <limits.h>

#define MAXB 64

#define chk(X) if ((X) == -1) { perror(#X); exit(1); }
#define chkn(X) if ((X) == NULL) { perror(#X); exit(1); }

void usage(char *a0) {
	fprintf(stderr,
		"usage: %s [-b <src> <dst>] [-p <dir>] <dir> <cmd> [<arg>] [<...>]\n"
		"  -b <src> <dst> : bind-mount src to dst directory in the rootfs\n"
		"  -p <dir>       : bind-mount directory on it's original path\n"
		"  -B <src> <dst> : like -b but recursive (allows sub-mounts)\n"
		"  -P <dir>       : like -p but recursive\n", a0);
	exit(1);
}

void mc_bind(char *cpath, char *src, char *dst, int flags) {
	char fdst[PATH_MAX + 1];
	if (snprintf(fdst, sizeof(fdst), "%s/%s", cpath, dst) >= sizeof(fdst) - 1) {
		fprintf(stderr, "error: pathname too long\n");
		exit(1);
	}
	chk(mount(src, fdst, 0, MS_BIND | flags, 0));
}

int main(int argc, char *argv[]) {
	char buf[32], cpath[PATH_MAX + 1], *bsrcs[MAXB], *bdsts[MAXB];
	int fd, bcnt = 0, bflags[MAXB];

	for (optind = 1; optind < argc; ++optind) {
		if (argv[optind][0] != '-')
			break;
		if (argv[optind][1] == 0 || argv[optind][2] != 0)
			break;
		if (argv[optind][1] == '-') {
			++optind;
			break;
		}
		if (argv[optind][1] == 'b' || argv[optind][1] == 'p')
			bflags[bcnt] = 0;
		else if (argv[optind][1] == 'B' || argv[optind][1] == 'P')
			bflags[bcnt] = MS_REC;
		if (argv[optind][1] == 'b' || argv[optind][1] == 'B') {
			bsrcs[bcnt] = argv[optind + 1];
			bdsts[bcnt] = argv[optind + 2];
			++bcnt;
			optind += 2;
			continue;
		}
		if (argv[optind][1] == 'p' || argv[optind][1] == 'P') {
			bdsts[bcnt] = argv[optind + 1];
			bsrcs[bcnt] = argv[optind + 1];
			++bcnt;
			++optind;
			continue;
		}
		usage(argv[0]);
	}

	if (argc - optind < 2)
		usage(argv[0]);

	if (geteuid() == 0) {
		chk(unshare(CLONE_NEWNS));
	} else {
		uid_t uid = getuid();
		uid_t gid = getgid();

		chk(unshare(CLONE_NEWUSER | CLONE_NEWNS));

		chk(fd = open("/proc/self/uid_map", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "0 %u 1\n", uid)));
		close(fd);

		chk(fd = open("/proc/self/setgroups", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "deny\n")));
		close(fd);

		chk(fd = open("/proc/self/gid_map", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "0 %u 1\n", gid)));
		close(fd);
	}
	chk(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL));

	/* Make sure path is absolute without excess dots and slashes. */
	chkn(realpath(argv[optind], cpath));

	mc_bind(cpath, "/dev", "/dev", MS_REC);
	mc_bind(cpath, "/proc", "/proc", MS_REC);

	for (int i = 0; i < bcnt; ++i)
		mc_bind(cpath, bsrcs[i], bdsts[i], bflags[i]);

	chk(chdir(cpath));
	chk(chroot(cpath));
	chk(execv(argv[optind + 1], argv + optind + 1));
	return 1; /* never reached (hopefully) */
}
