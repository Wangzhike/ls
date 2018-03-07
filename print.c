#include "myls.h"

void printdata(struct fileinfo_t *fileinfo, int n, options_t *opt, columns_t *col, int total)
{
	int i = 0, num_col = 0;
	struct winsize size;
	char *print_str = (char *)calloc(1024, sizeof(char));
	int *array_i = (int *)calloc(n, sizeof(int));
	int isTerminal = 0;

	if (isatty(fileno(stdout)))
		isTerminal = 1;

	//printf("isTerminal\t");

	if (ioctl(fileno(stdin), TIOCGWINSZ, (char *)&size) == -1)
		err_sys("ioctl for TIOCGWINSZ error");

	if (opt->flag_S)
		qsort(fileinfo, n ,sizeof(fileinfo_t), comp_finfos_S);
	else if (opt->flag_t)
		qsort(fileinfo, n, sizeof(fileinfo_t), comp_finfos_t);

	if (opt->flag_l == 1) {		/* 长输出格式 */
		if (total != -1)
			printf("total %d\n", total);
		for (i = 0; i < n; i++) {
			printf("%10s ", fileinfo[i].mode);	/* mode:type and permissions */
			print_str = getprintstring(print_str, INT, &col->nlink_c);
			printf(print_str, fileinfo[i].n_link);
			print_str = getprintstring(print_str, STRING, &col->user_c);
			printf(print_str, fileinfo[i].user);
			print_str = getprintstring(print_str, STRING, &col->group_c);
			printf(print_str, fileinfo[i].group);
			print_str = getprintstring(print_str, LONG_INT, &col->size_c);
			printf(print_str, fileinfo[i].f_size);
			printf("%s ", fileinfo[i].mtime);
			print_str = getcolorstring(print_str, fileinfo[i].mode, isTerminal, opt->flag_l);
			printf(print_str, 0, fileinfo[i].f_name);
			printf("\n");
			if (isTerminal)
				printf("\e[0;39m");
		}
		//printf("\n");
	} else {		/* 短输出，此时要注意输出分栏 */
		num_col = size.ws_col;
		int n_r = 1, flag_col = 0, j = 0, sum = 0;
		int *length = (int *)malloc(n*sizeof(int));
		int *max_length = (int *)malloc(n*sizeof(int));

		for (i = 0; i < n; i++)
			length[i] = strlen(fileinfo[i].f_name) + 2;

		while (flag_col != 1) {
			for (i = 0; i < n/n_r; i++)
				max_length[i] = findmax(length, i, n_r);
			sum = 0;
			for (i = 0; i < n/n_r; i++)
				sum += max_length[i];
			sum += n/n_r;
			if (sum > num_col)
				n_r++;
			else
				flag_col = 1;
		}
		for (i = 0; i < n; i++)
			array_i[i] = -1;
		for (i = 0; i <= n_r; i++) {
			for (j = 0; j <= n/n_r; j++) {
				if ( (j * n_r + i) < n && array_i[j * n_r + i] == -1) {
					print_str = getcolorstring(print_str, fileinfo[j * n_r + i].mode, isTerminal, opt->flag_l);
					printf(print_str, -max_length[j], fileinfo[j * n_r + i].f_name);
					array_i[j * n_r + i] = 0;
				}
			}
			printf("\n");
		}
		free(length);
		free(max_length);
	}
	free(print_str);
	free(array_i);
}

int findmax(int *array, int col, int row)
{
	int i = 0;
	int max = array[col * row];
	for (i = 0; i < row; i++)
		if (array[col * row + i] > max)
			max = array[col * row + i];
	return max;
}

char *getprintstring(char *print_str, type_t type, void *value)
{
	switch(type) {
		case INT:
			sprintf(print_str, "%%%dd ", *((int*)value) );
			break;
		case LONG_INT:
			sprintf(print_str, "%%%dld ", *((int*)value) );
			break;
		case STRING:
			sprintf(print_str, "%%%ds ", *((int*)value) );
	}
	return print_str;

}

char *getcolorstring(char *print_str, char *mode, char isTerminal, int flag_l) {
	if (isTerminal == 0) {
		if (flag_l == 1)
			strcpy(print_str, "%*s");
		else
			strcpy(print_str, "%*s\n");
		return print_str;
	}
	if (mode[0] == 'd')
		sprintf(print_str, "\e[1;34m%%*s\e[0;39m");
	else if (mode[0] == 'l')
		sprintf(print_str, "\e[1;36m%%*s\e[0;39m");
	else if (mode[0] == 'p')
		sprintf(print_str, "\e[1;33;40m%%*s\e[0;39m");
	else if (mode[0] == 'x' || mode[6] == 'x' || mode[9] == 'x')
		sprintf(print_str, "\e[1;92m%%*s\e[0;39m");
	else
		sprintf(print_str, "\e[0;39m%%*s\e[0;39m");
	return print_str;
}
