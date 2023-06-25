#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

/*
 * name: my-word-count
 * description: project 1, csc421 (231)
 *		this project is essentially cat _filename_ | wc _options_
 *
 * last-update:
 *		4 sep 2022 -bjr: template for student
 */

#include "my-word-count.h"

#define USAGE_MSG "usage: %s [-vclmw] _filename_\n"
#define PROG_NAME "my-word-count"

#define CAT_PROGRAM "cat"
#define WC_PROGRAM "wc"

// a rare use of globals; name mangle to alert

int is_verbose_g;
int c;
int l;
int m;
int w;

int main(int argc, char *argv[])
{
	int fd[2];
	pid_t c_pid;
	int ch;

	char *filepath;
	while (optind < argc)
	{
		if ((c = getopt(argc, argv, "vclmw")) != -1)
		{
			switch (c)
			{
			case 'v':
				is_verbose_g = 1;
				break;
			case 'c':
				c = 1;
				break;
			case 'l':
				l = 1;
				break;
			case 'm':
				m = 1;
				break;
			case 'w':
				w = 1;
				break;
			default:
				break;
			}
		}
		else
		{
			filepath = argv[optind];
			optind++;
		}
	}

	// char *a[2];
	// a[0] = "-l";
	// a[1] - "-w";

	pipe(fd);
	c_pid = fork();

	if (c_pid == -1)
	{
		perror("fork");
		return errno;
	}

	if (!c_pid)
	{
		// child
		close(fd[0]);
		dup2(fd[1], 1);
		close(fd[1]);
		execlp("cat", "cat", filepath, NULL);
		exit(1);
	}
	else
	{
		// parent
		close(fd[1]);
		dup2(fd[0], 0);
		close(fd[0]);
		if (argc == 2)
		{
			execlp("wc", "wc", NULL);
		}
		else
		{
			int count = 0;
			if (c == 1)
			{
				count++;
			}
			if (l == 1)
			{
				count++;
			}
			if (m == 1)
			{
				count++;
			}
			if (w == 1)
			{
				count++;
			}
			char *array = (char *)malloc(count + 2);
			int counter = 1;

			array[0] = '-';
			if (c == 1)
			{
				array[counter] = 'c';
				counter++;
			}
			if (l == 1)
			{
				array[counter] = 'l';
				counter++;
			}
			if (m == 1)
			{
				array[counter] = 'm';
				counter++;
			}
			if (w == 1)
			{
				array[counter] = 'w';
				counter++;
			}
			array[counter] = '\0';
			// printf("%d\n", counter);
			// printf("%s", array);
			
			execlp("wc", "wc", array, NULL);
		}

		exit(1);
	}
}
