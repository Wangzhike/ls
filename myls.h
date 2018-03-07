#ifndef _MYLS_H
#define _MYLS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <termios.h>
#ifndef TIOCGWINSZ
#include <sys/ioctl.h>
#include <limits.h>
#endif
#include "err_handlers/err_handlers.h"
#include "path_alloc/path_alloc.h"

#define SIZE_P	16
#define SIZE_T	16

/*
 * Enum type_t is datatype enum which is used for getting the format specifying string.
 * ex: for string it will return "%xs" where x is the number like 10 so function will return %10s
 */
typedef enum {
	INT, LONG_INT, STRING
} type_t;

/*
 * options_t struct is used to store options parsed by getopt.
 * For each option parsed, the corresponding flag is set.
 */
typedef struct options_t {
    int flag_a;     //all, do not ignore entries starting with .
    int flag_d;     //list directories instead of contents
    int flag_l;     //use a long listing format: mode, nlinks, user, group, size, mtime, filename
    int flag_R;     //list subdirectories recursively
    int flag_S;     //sort by file size, largest first
    int flag_t;     //sort by modification time, newest first
} options_t;

/*
 * The fileinfo_t structure is used for storing the detailed info about the file.
 * It store clock ticks, mode(file type && permissions), number of hard links, user, group, file size, last modification time, filename.
 */
typedef struct fileinfo_t {
	long long int tick;
	char *mode;
	int n_link;
	char *user;
	char *group;
	long int f_size;
	char *mtime;
	char *f_name;
} fileinfo_t;

/*
 * The columns_t structure is used for column adjustment is `ls -l` option.
 * The values in this structure will be the maximum width of the corresponding columns.
 * This data structure can be used to print column in well indented way.
 */
typedef struct columns_t {
	int nlink_c;
	int user_c;
	int group_c;
	int size_c;
} columns_t;

int myls(options_t *opt, char **args, int n_args);
int ls_f(options_t *opt, char **files, int n_files, columns_t *col);
int ls_p(options_t *opt, char *path, columns_t *col);

int getfinfos(char **files, fileinfo_t **finfos, int n_files, options_t *opt, columns_t *col, struct stat *fileStat);
char* getmode(char *mode, struct stat* fileStat);
int getlinks(struct stat* fileStat);
char* getuser(struct stat* fileStat);
char* getgroup(struct stat* fileStat);
int getsize(struct stat* fileStat);
char* getmtime(char *mtime, struct stat* fileStat);
char* getfilename(char *filename, char *orgin, char is_link, int flag_l);

/* util.c */
void reset_col(columns_t *col);
void set_c_size(columns_t *col, int n_link, char *user, char *group, long long int size);
int filter(const struct dirent *direntry);
int compare(const struct dirent **dirent1, const struct dirent **dirent2);
int comp_finfos_S(const void *a, const void *b);
int comp_finfos_t(const void *a, const void *b);

/* print.c */
void printdata(struct fileinfo_t *finfos, int n_finfos, options_t *opt, columns_t *col, int total);
int findmax(int *array, int col, int row);
char *getprintstring(char *print_str, type_t type, void *value);
char *getcolorstring(char *print_str, char *mode, char isTerminal, int flag_l);

#endif /* _MYLS_H */
