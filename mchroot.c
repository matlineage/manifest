/* 
 * Simple namespaces/chroot utility, with the following uses:
 * - You can enter chroots (even as an ordinary user) and map UID/GID.
 * - Convenient bind-mounts that only exist for the spawned process.
 * 
 * I got the idea and original implementation from here:
 *   https://github.com/sabotage-linux/sabotage/ (KEEP/super_chroot.c)
 * 
 */
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
#include <sys/syscall.h>
#include <grp.h>

#define MAXB 64

#define chk(X) { if ((X) == -1) { perror(#X); exit(1); } }
#define chkn(X) { if ((X) == NULL) { perror(#X); exit(1); } }

void usage(char *a0) {
	fprintf(stderr,
		"usage: %s [<options>] <cmd> [<arg>] [<...>]\n"
		"  -b <src> <dst> : bind-mount src to dst directory in the rootfs\n"
		"  -B <src> <dst> : like -b but recursive (allows sub-mounts)\n"
		"  -p <dir>       : bind-mount directory on it's original path\n"
		"  -P <dir>       : like -p but recursive\n"
		"  -o <src> <dst> : bind-mount src to dst (read-only)\n"
		"  -O <src> <dst> : bind-mount src to dst (read-only and recursive)\n"
		"  -t <dir>       : like -p but read-only\n"
		"  -T <dir>       : like -P but read-only\n"
		"  -u <uid:gid>   : set UID and GID\n"
		"  -n             : preserve UID and GID of current user\n"
		"  -r <dir>       : specify new root directory\n"
		"  -c <dir>       : change directory before executing command\n", a0);
	exit(1);
}

void mc_bind(char *cpath, char *src, char *dst, int flags, int flags2) {
	char fdst[PATH_MAX + 1];
	if (snprintf(fdst, sizeof(fdst), "%s/%s", cpath, dst) >= sizeof(fdst) - 1) {
		fprintf(stderr, "error: pathname too long\n");
		exit(1);
	}
	chk(mount(src, fdst, 0, MS_BIND | flags, 0));
	if (flags2 != 0) {
		struct mount_attr attr = { .attr_set = flags2 };
		chk(syscall(SYS_mount_setattr, AT_FDCWD, fdst, (flags & MS_REC) ? AT_RECURSIVE : 0, &attr, sizeof(attr)));
	}
}

int main(int argc, char *argv[]) {
	char buf[32], cpath[PATH_MAX + 1] = "/", *bsrcs[MAXB], *bdsts[MAXB];
	char *newroot = NULL, *cd = NULL;
	int fd, bcnt = 0, bflags[MAXB], bflags2[MAXB];;
	int isroot = geteuid() == 0;
	uid_t target_uid = 0;
	gid_t target_gid = 0;

	for (optind = 1; optind < argc; ++optind) {
		if (argv[optind][0] != '-')
			break;
		if (argv[optind][1] == 0 || argv[optind][2] != 0)
			break;
		if (argv[optind][1] == '-') {
			++optind;
			break;
		}
		if (argv[optind][1] == 'b' || argv[optind][1] == 'p') {
			bflags[bcnt] = 0;
			bflags2[bcnt] = 0;
		} else if (argv[optind][1] == 'B' || argv[optind][1] == 'P') {
			bflags[bcnt] = MS_REC;
			bflags2[bcnt] = 0;
		} else if (argv[optind][1] == 'o' || argv[optind][1] == 't') {
			bflags[bcnt] = 0;
			bflags2[bcnt] = MOUNT_ATTR_RDONLY;
		} else if (argv[optind][1] == 'O' || argv[optind][1] == 'T') {
			bflags[bcnt] = MS_REC;
			bflags2[bcnt] = MOUNT_ATTR_RDONLY | AT_RECURSIVE;
		}
		if (argv[optind][1] == 'b' || argv[optind][1] == 'B' || argv[optind][1] == 'o' || argv[optind][1] == 'O') {
			bsrcs[bcnt] = argv[optind + 1];
			bdsts[bcnt] = argv[optind + 2];
			++bcnt;
			optind += 2;
			continue;
		}
		if (argv[optind][1] == 'p' || argv[optind][1] == 'P' || argv[optind][1] == 't' || argv[optind][1] == 'T') {
			bdsts[bcnt] = argv[optind + 1];
			bsrcs[bcnt] = argv[optind + 1];
			++bcnt;
			++optind;
			continue;
		}
		if (argv[optind][1] == 'u') {
			int iuid = 0, igid = 0; /* Read as int first, who knows what uid_t is. */
			sscanf(argv[optind + 1], "%d:%d", &iuid, &igid);
			target_uid = iuid;
			target_gid = igid;
			++optind;
			continue;
		}
		if (argv[optind][1] == 'n') {
			target_uid = geteuid();
			target_gid = getegid();
			continue;
		}
		if (argv[optind][1] == 'c') {
			cd = argv[optind + 1];
			++optind;
			continue;
		}
		if (argv[optind][1] == 'r') {
			newroot = argv[optind + 1];
			++optind;
			continue;
		}
		usage(argv[0]);
	}

	if (argc - optind < 1)
		usage(argv[0]);

	if (isroot) {
		chk(unshare(CLONE_NEWNS));
	} else {
		uid_t uid = geteuid();
		uid_t gid = getegid();

		chk(unshare(CLONE_NEWUSER | CLONE_NEWNS));

		chk(fd = open("/proc/self/uid_map", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "%u %u 1\n", target_uid, uid)));
		close(fd);

		chk(fd = open("/proc/self/setgroups", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "deny\n")));
		close(fd);

		chk(fd = open("/proc/self/gid_map", O_RDWR));
		chk(write(fd, buf, snprintf(buf, sizeof(buf), "%u %u 1\n", target_gid, gid)));
		close(fd);
	}
	chk(mount(NULL, "/", NULL, MS_REC | MS_PRIVATE, NULL));

	/* Make sure path is absolute without excess dots and slashes. */
	if (newroot != NULL) {
		chkn(realpath(newroot, cpath));
		mc_bind(cpath, "/dev", "/dev", MS_REC, 0);
		mc_bind(cpath, "/proc", "/proc", MS_REC, 0);
	}

	for (int i = 0; i < bcnt; ++i)
		mc_bind(cpath, bsrcs[i], bdsts[i], bflags[i], bflags2[i]);

	if (newroot != NULL) {
		chk(chdir(cpath));
		chk(chroot(cpath));
	}
	if (isroot && (target_uid != 0 || target_gid != 0)) {
		chk(setgroups(0, NULL)); /* Avoid inheriting supplementary groups. */
		chk(setresgid(target_gid, target_gid, target_gid));
		chk(setresuid(target_uid, target_uid, target_uid));
	}
	if (cd != NULL)
		chk(chdir(cd));
	chk(execv(argv[optind], argv + optind));
	return 1; /* Never reached (hopefully). */
}
