
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "snail.h"

int main()

{

    struct Board board_g;

    char *buff = (char *)malloc(50);

    init_board(&board_g);

    while (1)
    {
        printf(board_draw(buff, &board_g));
        printf("\n");
        board_g.snail_loc++;
        if (snail_collide(&board_g))
        {
            board_gameover(&board_g);
            printf("The Snail got Smushed\n");
            break;
        }
    }
    return 0;
}