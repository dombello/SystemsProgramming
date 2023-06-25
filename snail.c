#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <assert.h>

#include "snail.h"

/*
 * author: burt rosenberg
 * last update:
 *    20 september 2022
 *
 */

int is_verbose_g = 0;
struct Board board_g;

void *draw_thread(void *the_args)
{

	struct T_arg *t_arg = (struct T_arg *)the_args;
	struct timeval tv;
	struct timespec ts;

	char *board_string;
	board_string = (char *)malloc(BOARD_SIZE + 1);
	assert(board_string);

	char *buff = (char *)malloc(50);

	while (1)
	{
		printf(board_draw(buff, &board_g));
		printf("\n");
		if (snail_collide(&board_g)&board_g.guess == board_g.gate_key)
		{
			board_gameover(&board_g);
			printf("The Snail got Smushed\n");
			break;
		}

		pthread_mutex_lock(t_arg->mutex);
		{
			gettimeofday(&tv, NULL);
			ts.tv_sec = tv.tv_sec + SLEEP_TIME;
			ts.tv_nsec = 0;
			pthread_cond_timedwait(t_arg->cond, t_arg->mutex, &ts);
			board_g.snail_loc++;
		}
		pthread_mutex_unlock(t_arg->mutex);
	}

	pthread_exit(NULL);
	assert(0 == 1); /* never gets here */
}

int main(int argc, char *argv[])
{
	int ch;
	pthread_t thread_id;
	char buf[LINE_MAX];
	struct T_arg t_arg;
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	t_arg.req_exit = 0;
	t_arg.mutex = &mutex;
	t_arg.cond = &cond;

	while ((ch = getopt(argc, argv, "v")) != -1)
	{
		switch (ch)
		{
		case 'v':
			is_verbose_g += 1;
			break;
		default:
			printf(USAGE_MSG, PROG_NAME);
			return 0;
		}
	}
	argc -= optind;
	argv += optind;

	init_board(&board_g);
	printf(INSTRUCTIONS, QUIT_STRING);

	if (is_verbose_g)
	{
		printf("(%s,%d): key %c\n", __FILE__, __LINE__, board_g.gate_key);
	}

	pthread_create(&thread_id, NULL, draw_thread, &t_arg);

	while (fgets(buf, LINE_MAX, stdin) != NULL)
	{
		buf[strlen(buf) - 1] = '\0';
		if (!strcmp(buf, QUIT_STRING))
		{
			break;
		}

		pthread_mutex_lock(&mutex);
		{

			pthread_cond_signal(&cond);
			strcpy(&board_g.guess,buf);
			if (board_g.guess == board_g.gate_key)
			{
				board_g.gate_state = GATE_IS_OPEN;
			}
		//	printf("%d\n", board_g.gate_state);
		//	printf("%d\n", board_g.gate_key);
		//	printf("%c\n", board_g.guess);
		}
		pthread_mutex_unlock(&mutex);
	}

	if (is_verbose_g)
	{
		printf("(%s,%d): requesting thread to exit ...\n", __FILE__, __LINE__);
	}

	pthread_mutex_lock(&mutex);
	t_arg.req_exit = 1; /* tell thread to exit */
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);

	pthread_exit(NULL); /* wait for the draw thread to exit */
	assert(0 == 1);		/* never gets here */
}
