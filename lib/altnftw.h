/**
 * @file altnftw.h
 * @author MXPSQL
 * @brief An alternative nftw
 * @version 0.1
 * @date 2023-11-19
 * 
 * @copyright Copyright (c) 2023
 * 
 * 
 * altnftw is an alternative nftw. Its implemented on top of dirent.
 * This implementation is copied from musl. Grab yours at http://git.musl-libc.org/cgit/musl/tree/src/misc/nftw.c
 * Its changed so it will work with C++ and has an additional context pointer, so no more global variables.
 * This altnftw still uses the ftw constants, so use it.
 */

#ifndef MXPSQL_ALTNFTW_H
#define MXPSQL_ALTNFTW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ftw.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>

struct althistory
{
	struct althistory *chain;
	dev_t dev;
	ino_t ino;
	int level;
	int base;
};

#undef dirfd
#define dirfd(d) (*(int *)d)

static int do_altnftw(char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *, void*), int fd_limit, int flags, struct althistory *h, void* ctx)
{
	size_t l = strlen(path), j = l && path[l-1]=='/' ? l-1 : l;
	struct stat st;
	struct althistory newh;
	int type = 0;
	int r = 0;
	int dfd = 0;
	int err = 0;
	struct FTW lev;

	st.st_dev = st.st_ino = 0;

	if ((flags & FTW_PHYS) ? lstat(path, &st) : stat(path, &st) < 0) {
		if (!(flags & FTW_PHYS) && errno==ENOENT && !lstat(path, &st))
			type = FTW_SLN;
		else if (errno != EACCES) return -1;
		else type = FTW_NS;
	} else if (S_ISDIR(st.st_mode)) {
		if (flags & FTW_DEPTH) type = FTW_DP;
		else type = FTW_D;
	} else if (S_ISLNK(st.st_mode)) {
		if (flags & FTW_PHYS) type = FTW_SL;
		else type = FTW_SLN;
	} else {
		type = FTW_F;
	}

	if ((flags & FTW_MOUNT) && h && type != FTW_NS && st.st_dev != h->dev)
		return 0;
	
	newh.chain = h;
	newh.dev = st.st_dev;
	newh.ino = st.st_ino;
	newh.level = h ? h->level+1 : 0;
	newh.base = j+1;
	
	lev.level = newh.level;
	if (h) {
		lev.base = h->base;
	} else {
		size_t k;
		for (k=j; k && path[k]=='/'; k--);
		for (; k && path[k-1]!='/'; k--);
		lev.base = k;
	}

	if (type == FTW_D || type == FTW_DP) {
		dfd = open(path, O_RDONLY);
		err = errno;
		if (dfd < 0 && err == EACCES) type = FTW_DNR;
		if (!fd_limit) close(dfd);
	}

	if (!(flags & FTW_DEPTH) && (r=fn(path, &st, type, &lev, ctx)))
		return r;

	for (; h; h = h->chain)
		if (h->dev == st.st_dev && h->ino == st.st_ino)
			return 0;

	if ((type == FTW_D || type == FTW_DP) && fd_limit) {
		if (dfd < 0) {
			errno = err;
			return -1;
		}
		DIR *d = fdopendir(dfd);
		if (d) {
			struct dirent *de;
			while ((de = readdir(d))) {
				if (de->d_name[0] == '.'
				 && (!de->d_name[1]
				  || (de->d_name[1]=='.'
				   && !de->d_name[2]))) continue;
				if (strlen(de->d_name) >= PATH_MAX-l) {
					errno = ENAMETOOLONG;
					closedir(d);
					return -1;
				}
				path[j]='/';
				strcpy(path+j+1, de->d_name);
				if ((r=do_altnftw(path, fn, fd_limit-1, flags, &newh, ctx))) {
					closedir(d);
					return r;
				}
			}
			closedir(d);
		} else {
			close(dfd);
			return -1;
		}
	}

	path[l] = 0;
	if ((flags & FTW_DEPTH) && (r=fn(path, &st, type, &lev, ctx)))
		return r;

	return 0;
}

int altnftw(const char *path, int (*fn)(const char *, const struct stat *, int, struct FTW *, void*), int fd_limit, int flags, void* ctx)
{
	int r, cs;
	size_t l;
	char pathbuf[PATH_MAX+1];

	if (fd_limit <= 0) return 0;

	l = strlen(path);
	if (l > PATH_MAX) {
		errno = ENAMETOOLONG;
		return -1;
	}
	memcpy(pathbuf, path, l+1);
	
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &cs);
	r = do_altnftw(pathbuf, fn, fd_limit, flags, NULL, ctx);
	pthread_setcancelstate(cs, 0);
	return r;
}

#ifdef __cplusplus
}
#endif

#endif