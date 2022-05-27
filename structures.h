#pragma once


struct User {
    int id;
    char nickname[15];
    char status[10];
    long f_u_adr;
    int count_artists;
};

struct SavedArtist {
    int artist_id;
    int user_id;
    char name[20];
    int joined_year;
    int exists;
    long self_adr;
    long next_adr;
};

struct Indexer
{
    int id;
    int adr;
    int exists;
};