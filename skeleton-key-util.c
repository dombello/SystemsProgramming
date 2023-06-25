#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>
#include <time.h>

#include "skeleton-key.h"

/*
 * name: skeleton-key-util.c
 * last update:
 *    29 nov 2022, bjr
 *
 */

static struct DirEnt root_dir[DIR_N];
static unsigned int fat_table[FAT_N];
static char data[FAT_N * CLUSTER_SIZE];

void init_fat()
{
	int i;

	if (is_verbose_g > 1)
	{
		printf("(%s,%d):\n\t%ld fat entries (%ld bytes),"
			   "\n\t%ld dir entries (%ld bytes),"
			   "\n\tdirent size is %ld bytes.\n",
			   __FILE__, __LINE__,
			   (unsigned long)FAT_N, sizeof(fat_table),
			   (unsigned long)DIR_N, sizeof(root_dir),
			   sizeof(struct DirEnt));
	}

	for (i = 0; i < FAT_N; i++)
		fat_table[i] = 0;
	for (i = 0; i < sizeof(root_dir); i++)
		((char *)root_dir)[i] = 0;
	for (i = 0; i < FAT_N * CLUSTER_SIZE; i++)
		data[i] = 0;
	return;
}

// ---------------------------
// project 4 routines
// ---------------------------

// note update! rm_action and rm_action_aux work together, with
// rm_action just breaking out the actv arguments and passing them to rm_action_aux
// same for cr_action and cr_action_aux.

// this is done to save work on the new project 5 routines.

int find_file(char *file_name)
{
	// return directory index if found, else return -1
	int i = 0;

	while (i < DIR_N)
	{
		if (strcmp(root_dir[i].name, "") == 0)
		{
			i += 1;
			continue;
		}

		if (strcmp(file_name, root_dir[i].name) == 0)
		{
			return i;
		}
		i += 1;
	}
	return -1;
}

int cluster_chain(int byte_len)
{
	// create a cluster chain of length minimally sufficient to
	// hold byte_len bytes, but never less than one; or -1 if
	// an error
	int clus_size;

	if (byte_len == 0)
	{
		clus_size = 1;
	}
	else if (byte_len % 128 == 0)
	{
		clus_size = byte_len / 128;
	}
	else
	{
		clus_size = byte_len / 128 + 1;
	}

	int head = 0;

	while (1)
	{

		if (head == FAT_N)
		{
			return -1;
		}
		if (fat_table[head] == 0)
		{
			break;
		}

		head += 1;
	}

	int i = head + 1;
	int end = head;

	while (clus_size > 0)
	{

		if (i == FAT_N)
		{
			return -1;
		}

		if (fat_table[i] == 0)
		{

			if (clus_size == 1)
			{
				fat_table[end] = -1;
				break;
			}
			else
			{
				fat_table[end] = i;
				end = i;
				clus_size -= 1;
			}
		}

		i += 1;
	}

	return head;
}

int qu_action(int actc, char *actv[])
{
	if (is_verbose_g > 1)
	{
		printf("(%s,%d):qu_action\n", __FILE__, __LINE__);
	}
	return ERR_ABORT;
}

int ls_action(int actc, char *actv[])
{
	if (is_verbose_g > 1)
	{
		printf("(%s,%d):action: %s, %d\n", __FILE__, __LINE__, (char *)actv[0], actc);
	}

	// print out all files in the following format:
	// printf("%3d %4d %s\n", i, root_dir[i].len, root_dir[i].name) ;

	int i = 0;

	while (1)
	{

		if (i == DIR_N)
		{
			break;
		}

		if (strcmp(root_dir[i].name, "") == 0)
		{
			i += 1;
			continue;
		}

		printf("%3d %4d %s\n", i, root_dir[i].len, root_dir[i].name);
		i += 1;
	}
	return 0;
}

// create needs to be updated to write a \0 in the 0th byte of the file so created

int cr_action_aux(char *file_name, int len)
{
	// create the file name file_name of length len

	// errors:
	//   return ERR_CREATE ;  // file exists
	//   return ERR_DIRFULL ; // no room in directly
	//   return ERR_FATFULL ; // no room on disk
	int i = 0;

	while (i < DIR_N)
	{

		if (strcmp(root_dir[i].name, file_name) == 0)
		{
			return ERR_CREATE;
		}
		i += 1;
	}

	struct DirEnt new;
	strncpy(new.name, file_name, sizeof(new.name));

	int head = cluster_chain(len);

	if (head == -1)
	{
		return ERR_FATFULL;
	}

	new.len = len;
	new.starting_cluster = head;

	i = 0;

	while (1)
	{
		if (i == DIR_N)
		{
			return ERR_DIRFULL;
		}

		if (strcmp(root_dir[i].name, "") == 0)
		{

			root_dir[i] = new;
			break;
		}

		i += 1;
	}

	return 0;
}

int rm_action_aux(char *file_name)
{
	// remove the file named actv[1]

	// errors:
	//    return ERR_NOEXIST ; // file does not exist

	int i = 0;

	while (1)
	{
		if (i == DIR_N)
		{
			return ERR_NOEXIST;
		}
		if (strcmp(root_dir[i].name, "") == 0)
		{
			i += 1;
			continue;
		}

		if (strcmp(root_dir[i].name, file_name) == 0)
		{
			break;
		}

		i += 1;
	}

	int clus_size;

	if (root_dir[i].len == 0)
	{
		clus_size = 1;
	}
	else if (root_dir[i].len % 128 == 0)
	{
		clus_size = root_dir[i].len / 128;
	}
	else
	{
		clus_size = root_dir[i].len / 128 + 1;
	}

	int position = root_dir[i].starting_cluster;
	int next;

	while (clus_size > 0)
	{

		next = fat_table[position];
		fat_table[position] = 0;
		position = next;
		clus_size -= 1;
	}

	strcpy(root_dir[i].name, "");

	return 0;
}

int rm_action(int actc, char *actv[])
{
	if (is_verbose_g > 1)
	{
		printf("(%s,%d):action: %s, %d\n", __FILE__, __LINE__, (char *)actv[0], actc);
	}
	return rm_action_aux(actv[1]);
}

int cr_action(int actc, char *actv[])
{
	int len;

	if (is_verbose_g > 1)
	{
		printf("(%s,%d):action: %s, %d\n", __FILE__, __LINE__, (char *)actv[0], actc);
	}
	len = atoi(actv[1]);
	return cr_action_aux(actv[2], len);
}

int pc_action(int actc, char *actv[])
{
	if (is_verbose_g > 1)
	{
		printf("(%s,%d):action: %s, %d\n", __FILE__, __LINE__, (char *)actv[0], actc);
	}

	// print the cluster chain for the file name actv[1]

	// use this format:
	// printf("%s: ",actv[1]) ;
	//    for each cluster in order printf("%d ", cluster ) ;
	// printf("\n") ;

	// errors:
	//   return ERR_NOEXIST ;  // file does not exist
	int i = 0;

	while (1)
	{

		if (strcmp(root_dir[i].name, "") == 0)
		{
			i += 1;
			continue;
		}

		if (i == DIR_N)
		{
			return ERR_NOEXIST;
		}

		if (strcmp(root_dir[i].name, actv[1]) == 0)
		{
			break;
		}

		i += 1;
	}

	int clus_size;

	if (root_dir[i].len == 0)
	{
		clus_size = 1;
	}
	else if (root_dir[i].len % 128 == 0)
	{
		clus_size = root_dir[i].len / 128;
	}
	else
	{
		clus_size = root_dir[i].len / 128 + 1;
	}

	int clus = root_dir[i].starting_cluster;
	printf("%s: ", actv[1]);

	for (int j = 0; j < clus_size; j++)
	{
		printf("%d ", clus);
		clus = fat_table[clus];
	}

	printf("\n");

	return 0;
}

// ---------------------------
// project 5 new routines
// ---------------------------

// these two new actions are done for you

int dd_action(int actc, char *actv[])
{
	int i;
	for (i = 0; i < FAT_N * CLUSTER_SIZE; i++)
	{
		if (!(i & 0x0f))
			printf("\n%04d: ", i);
		printf("%02x ", data[i]);
	}
	printf("\n");
	return 0;
}

// need a prototype for a forwardly defined function
int chaining_read(char buf[], int n, int cluster, int len);

int rd_action(int actc, char *actv[])
{
	int f, n;
	char *buf;
	int res;

	f = find_file(actv[1]);
	if (f == -1)
		return ERR_NOEXIST;
	n = root_dir[f].len;
	buf = (char *)malloc(n);
	assert(buf);

	res = chaining_read(buf, n, root_dir[f].starting_cluster, root_dir[f].len);
	if (res)
	{
		free(buf);
		return res;
	}
	printf("%s: %s\n", actv[1], buf);
	free(buf);
	return 0;
}

// chaining_write and chaining_read are the tricky ones.

// 1) get chaining_write done. chaining_read will be a one line
//    change that you can do at the last minute.
// 2) first do it for 1 cluster files, no chaining.
// 3) have a plan, and let printf tell you the computer is carrying out your plan

int chaining_write(char buf[], int n, int cluster, int len)
{
	// copy the n bytes in buf to data[] beginning at cluster*CLUSTER_SIZE
	// and following the cluster chain, as needed.
	if (len > n)
	{
		return (ERR_DATA);
	}
	for (int i = 0; i < len; i++)
	{
		data[(cluster * CLUSTER_SIZE) + i] = buf[i];
	}

	return 0;
}

int chaining_read(char buf[], int n, int cluster, int len)
{
	// copy len butes into buf beginning at cluster*CLUSTER_SIZE
	// and following the cluster chain, as needed.
	if (len > n)
	{
		return (ERR_DATA);
	}

	for (int i = 0; i < len; i++)
	{
		buf[i] = data[(cluster * CLUSTER_SIZE) + i];
	}
	return 0;
}

int wr_action(int actc, char *actv[])
{
	// write the string (including the \0) of actv[2] into file
	// with filename actv[1]

	// hint:
	//  1) first call rm_action_aux and ignore the return value
	//  2) then call cr_action_aux with the filename and the string length plus one
	//  3) check the return value
	//  4) call chaining_write
	//

	int f, n;

	int res;
	n = strlen(actv[2]);

	rm_action_aux(actv[1]);
	res = cr_action_aux(actv[1], n);
	if (res)
	{

		return res;
	}

	f = find_file(actv[1]);
	if (f == -1)
		return ERR_NOEXIST;
	n = root_dir[f].len;

	res = chaining_write(actv[2], n, root_dir[f].starting_cluster, root_dir[f].len);

	if (res)
	{

		return res;
	}
	return 0;
}

/* end of file */
