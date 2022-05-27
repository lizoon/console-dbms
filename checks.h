#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "artist.h"



int get_user(struct User* user, int id, char* error);


int check_file_exist(FILE* ind_table, FILE* database, char* error)
{
    // DB files do not exist yet
    if (ind_table == NULL || database == NULL) {
        strcpy(error, "Database files are not created yet :(.");
        return 0;
    }

    return 1;
}


int check_ind_exist(FILE* ind_table, char* error, int id)
{
    fseek(ind_table, 0, SEEK_END);

    long size_ind_table = ftell(ind_table);

    if (size_ind_table == 0 || id * sizeof(struct Indexer) > size_ind_table)
    {
        strcpy(error, "No such id in table.");
        return 0;
    }

    return 1;
}


int check_record_exist(struct Indexer indexer, char* error)
{
    // Record's been removed
    if (!indexer.exists)
    {
        strcpy(error, "This record has been removed.");
        return 0;
    }

    return 1;
}



void info()
{
    FILE* ind_table = fopen("user.ind", "rb");

    if (ind_table == NULL)
    {
        printf("Error. Database files are not created yet.\n");
        return;
    }

    int user_count = 0;
    int artist_count = 0;

    fseek(ind_table, 0, SEEK_END);
    int ind_count = ftell(ind_table) / sizeof(struct Indexer);

    struct User user;

    char dummy[51];

    for (int i = 1; i <= ind_count; i++)
    {
        if (get_user(&user, i, dummy))
        {
            user_count++;
            artist_count += user.count_artists;

            printf("User #%d has %d saved artist(s)\n", i, user.count_artists);
        }
    }

    fclose(ind_table);

    printf("Total users: %d.\n", user_count);
    printf("Total saved artists: %d.\n", artist_count);
}