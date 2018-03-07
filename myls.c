#include "myls.h"

int myls(options_t *opt, char **args, int n_args)
{
	int i = 0;
	int n_args_f = 0, n_args_d = 0;
	char **args_f = NULL, **args_d = NULL;
	struct stat fileStat;
	char *cwd;
	char *fullpath;
	size_t pathlen;

	columns_t *col = (columns_t *)calloc(1, sizeof(columns_t));
	args_d = (char **)calloc(n_args, sizeof(char *));
	args_f = (char **)calloc(n_args, sizeof(char *));

	/*
	 * 首先对argv参数中提取出的路径名进行分类：文件和目录
	 */
	if (n_args == 0) {	/* 没有指定路径名，则默认路径为当前目录`.` */
		if (opt->flag_d == 0) {
			ls_p(opt, (char *)".", col);
			free(col);
			free(args_d);
			free(args_f);
			return 0;
		} else {
			free(args_d);
			args_d = (char **)calloc(1, sizeof(char *));
			args_d[0] = (char *)calloc(SIZE_P, sizeof(char));
			strcpy(args_d[0], ".");

			ls_f(opt, args_d, 1, col);
			free(col);
			free(args_d[0]);
			free(args_d);
			free(args_f);
			return 0;
		}
	}
	for (i = 0; i < n_args; i++) {
		reset_col(col);
		if (lstat(args[i], &fileStat) == -1) {
			err_ret("can't stat %s", args[i]);
			break;
		}

		if (S_ISDIR(fileStat.st_mode))
			args_d[n_args_d++] = args[i];
		else
			args_f[n_args_f++] = args[i];
	}
	/*
	 * 然后检测是否有`-d`选项：输出目录本身，而不是其内容
	 */
	if (opt->flag_d == 1)
		ls_f(opt, args, n_args, col);
	else {	/* 没有`-d`选项 */
		if (n_args_f != 0)		/* 路径参数中含有普通文件 */
			ls_f(opt, args_f, n_args_f, col);

		for (i = 0; i < n_args_d; i++) {
			/* 获取目录的绝对路径名 */
				/* 获取当前工作目录的绝对路径 */
			cwd = path_alloc(&pathlen);
			if (getcwd(cwd, pathlen) == NULL)
				err_sys("getcwd error");
				/* 切换当前工作目录到args_d[i] */
			if (chdir(args_d[i]) == -1)
				err_sys("can't chdir to %s", args_d[i]);
				/* 获取当前目录(即指定目录)的绝对路径 */
			fullpath = path_alloc(&pathlen);
			if (getcwd(fullpath, pathlen) == NULL)
				err_sys("getcwd for %s error", args_d[i]);
				/* 重新切回之前的工作目录 */
			if (chdir(cwd) == -1)
				err_sys("can't chdir to %s", cwd);
			printf("%s:\n", args_d[i]);

			ls_p(opt, args_d[i], col);
			free(cwd);
			free(fullpath);
		}
	}

	free(col);
	free(args_d);
	free(args_f);
	return 0;
}

int ls_f(options_t *opt, char **files, int n_files, columns_t *col)
{
	struct fileinfo_t *fileinfos = NULL;
	struct stat fileStat;
	int i = 0;

	reset_col(col);
	getfinfos(files, &fileinfos, n_files, opt, col, &fileStat);
	printdata(fileinfos, n_files, opt, col, -1);

	for (i = 0; i < n_files; i++) {
		free(fileinfos[i].mode);
		free(fileinfos[i].mtime);
		free(fileinfos[i].f_name);
	}
	free(fileinfos);
	return 0;
}

int ls_p(options_t *opt, char *path, columns_t *col)
{
	struct dirent **namelist = NULL;
	char **files = NULL;
	struct fileinfo_t *fileinfos = NULL;
	struct stat fileStat;
	char *cwd = NULL;
	size_t pathlen;
	char **list_d = NULL;
	int n = 0, i = 0, n_d = 0;
	int total = 0;

	/* 获取当前目录下的文件个数 */
	if (opt->flag_a == 0)
		n = scandir(path, &namelist, filter, compare);
	else
		n = scandir(path, &namelist, NULL, compare);
	if (n == -1)
		err_sys("can't scandir for %s", path);

	list_d = (char **)calloc(n, sizeof(char *));
	cwd = path_alloc(&pathlen);

	if (getcwd(cwd, pathlen) == NULL)
		err_sys("getcwd error");
	if (chdir(path) == -1)
		err_ret("can't chdir to %s", path);

	files = (char **)calloc(n, sizeof(char *));
	for (i = 0; i < n; i++)
		files[i] = namelist[i]->d_name;
	reset_col(col);
	total = getfinfos(files, &fileinfos, n, opt, col, &fileStat);
	printdata(fileinfos, n, opt, col, total/2);
	for (i = 0; i < n; i++) {
		/* 如果目录项对应的文件为目录文件 */
		if (fileinfos[i].mode[0] == 'd') {
			list_d[n_d] = path_alloc(&pathlen);
			strcpy(list_d[n_d], fileinfos[i].f_name);
			n_d++;
		}
	}
	/* 如果指定了`-R`选项：递归输出子目录 */
	if (opt->flag_R == 1) {
		for (i = 0; i < n_d; i++) {
			/* 忽略`.`和`..`目录 */
			if (strcmp(".", list_d[i]) && strcmp("..", list_d[i])) {
				/* 重新调用`myls`处理 */
				
				//printf("recursively!\t");
				myls(opt, &list_d[i], 1);

			}
		}
	}

	if (chdir(cwd) == -1)
		err_sys("can't chdir to %s", cwd);
	free(files);
	for (i = 0; i < n; i++) {
		free(namelist[i]);
		free(fileinfos[i].mode);
		free(fileinfos[i].mtime);
		free(fileinfos[i].f_name);
	}
	for (i = 0; i < n_d; i++) {
		free(list_d[i]);
	}
	free(namelist);
	free(fileinfos);
	free(list_d);
	free(cwd);
	return 0;
}

int getfinfos(char **files, fileinfo_t **finfos, int n_files, options_t *opt, columns_t *col, struct stat *fileStat)
{
	int i = 0;
	int total = 0;
	size_t pathlen;

	*finfos = (fileinfo_t *)calloc(n_files, sizeof(fileinfo_t));
	for (i = 0; i < n_files; i++) {
		if (lstat(files[i], fileStat) == -1)
			err_sys("can't stat for %s", files[i]);
		(*finfos)[i].mode = (char *)calloc(SIZE_P, sizeof(char));
		(*finfos)[i].mtime = (char *)calloc(SIZE_T, sizeof(char));
		(*finfos)[i].f_name = path_alloc(&pathlen);

		(*finfos)[i].mode = getmode((*finfos)[i].mode, fileStat);
		(*finfos)[i].n_link = getlinks(fileStat);
		(*finfos)[i].user = getuser(fileStat);
		(*finfos)[i].group = getgroup(fileStat);
		(*finfos)[i].f_size = getsize(fileStat);
		(*finfos)[i].mtime = getmtime((*finfos)[i].mtime, fileStat);
		(*finfos)[i].f_name = getfilename((*finfos)[i].f_name, files[i], (*finfos)[i].mode[0], opt->flag_l);
		(*finfos)[i].tick = (long long int)fileStat->st_mtime;
		total += fileStat->st_blocks;

		set_c_size(col, (*finfos)[i].n_link, (*finfos)[i].user, (*finfos)[i].group, (*finfos)[i].f_size);
	}
	return total;
}

char *getmode(char *mode, struct stat *fileStat)
{
	if (S_ISLNK(fileStat->st_mode))
		mode[0] = 'l';
	else if (S_ISDIR(fileStat->st_mode))
		mode[0] = 'd';
	else if (S_ISCHR(fileStat->st_mode))
		mode[0] = 'c';
	else if (S_ISSOCK(fileStat->st_mode))
		mode[0] = 's';
	else if (S_ISFIFO(fileStat->st_mode))
		mode[0] = 'p';
	else if (S_ISBLK(fileStat->st_mode))
		mode[0] = 'b';
	else 
		mode[0] = '-';	/* regular file */
	mode[1] = ( (fileStat->st_mode & S_IRUSR) ? 'r' : '-');
	mode[2] = ( (fileStat->st_mode & S_IWUSR) ? 'w' : '-');
	mode[3] = ( (fileStat->st_mode & S_IXUSR) ? 'x' : '-');
	mode[4] = ( (fileStat->st_mode & S_IRGRP) ? 'r' : '-');
	mode[5] = ( (fileStat->st_mode & S_IWGRP) ? 'w' : '-');
	mode[6] = ( (fileStat->st_mode & S_IXGRP) ? 'x' : '-');
	mode[7] = ( (fileStat->st_mode & S_IROTH) ? 'r' : '-');
	mode[8] = ( (fileStat->st_mode & S_IWOTH) ? 'w' : '-');
	mode[9] = ( (fileStat->st_mode & S_IXOTH) ? 'x' : '-');

	if (fileStat->st_mode & S_ISUID)
		mode[3] = 's';
	else if (fileStat->st_mode & S_IXUSR)
		mode[3] = 'x';
	else
		mode[3] = '-';
	if (fileStat->st_mode & S_ISGID)
		mode[6] = 's';
	else if (fileStat->st_mode & S_IXGRP)
		mode[6] = 'x';
	else
		mode[6] = '-';
	if (fileStat->st_mode & S_ISVTX)
		mode[9] = 't';
	else if (fileStat->st_mode & S_IXOTH)
		mode[9] = 'x';
	else
		mode[9] = '-';
	mode[10] = 0;
	return mode;
}

int getlinks(struct stat *fileStat)
{
	return fileStat->st_nlink;
}

char *getuser(struct stat *fileStat)
{
	struct passwd *pass = getpwuid(fileStat->st_uid);
	return pass->pw_name;
}

char *getgroup(struct stat *fileStat)
{
	struct group *pass = getgrgid(fileStat->st_gid);
	return pass->gr_name;
}

int getsize(struct stat *fileStat)
{
	return fileStat->st_size;	
}

char *getmtime(char *mtime, struct stat *fileStat)
{
	int i, j, count;
	struct tm *tminfo = localtime(&(fileStat->st_mtim.tv_sec));
	char *time = asctime(tminfo);
	i = j = count = 0;
	while (time[i++] != ' ');
	while(count != 2) {
		if (time[i] == ':')
			count++;
		if (count == 2)
			break;
		mtime[j++] = time[i++];
	}
	mtime[j] = '\0';

	return mtime;
}

char *getfilename(char *filename, char *orgin, char is_link, int flag_l)
{
	size_t pathlen;
	int n = 0;
	char *tmp = path_alloc(&pathlen);

	if (flag_l == 0 || is_link != 'l')
		strcpy(filename, orgin);
	else {
		n = readlink(orgin, tmp, pathlen);
		if (n == -1) {
			perror("readlink");
			free(tmp);
			strcpy(filename, orgin);
			return filename;
		}
		tmp[n] = '\0';
		sprintf(filename, "%s -> %s", orgin, tmp);
	}
	free(tmp);
	return filename;
}
