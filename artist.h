#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "structures.h"
#include "user.h"
#include "checks.h"

#define ARTIST_DATA "artist.fl"
#define ARTIST_GARBAGE "artist_garbage.txt"
#define ARTIST_SIZE sizeof(struct SavedArtist)


int update_user(struct User user, char* error);

// get

int get_artist(struct User user, struct SavedArtist* artist, int artist_id, char *error) {
    if (!user.count_artists) {
        strcpy(error, "This user doesn't exist.");
        return 0;
    }

    FILE *database = fopen(ARTIST_DATA, "rb");

    fseek(database, user.f_u_adr, SEEK_SET);
    fread(artist, ARTIST_SIZE, 1, database);

    for (int i = 0; i<user.count_artists; i++) {
        if (artist->artist_id == artist_id) {
            fclose(database);
            return 1;
        }

        fseek(database, artist->next_adr, SEEK_SET);
        fread(artist, ARTIST_SIZE, 1, database);

    }

    strcpy(error, "No such artist in database.");
    fclose(database);
    return 0;
}


//del

void relink_adr(FILE* database, struct SavedArtist artist_, struct SavedArtist artist, struct User* user);

void w_to_trash_artist(long adr);


int del_artist(struct User user, struct SavedArtist artist, int artist_id, char *error) {
    FILE* database = fopen(ARTIST_DATA, "r+b");
    struct SavedArtist artist_;

    fseek(database, user.f_u_adr, SEEK_SET);

    do {
        fread(&artist_, ARTIST_SIZE, 1, database);
        fseek(database, artist_.next_adr, SEEK_SET);
    }
    while (artist_.next_adr != artist.self_adr && artist.self_adr != user.f_u_adr);


    // збір сміття
    relink_adr(database, artist_, artist, &user);
    w_to_trash_artist(artist.self_adr);						                    // заносимо адресу видаленого запису у "смітник"

    artist.exists = 0;		                                                // логічно вилучаємо
    fseek(database, artist.self_adr, SEEK_SET);				// але фізично
    fwrite(&artist, ARTIST_SIZE, 1, database);					    // записуємо назад
    fclose(database);

    user.count_artists--;										                // на одну пісню менше
    update_user(user, error);

    return 0;
}


// перерозподіл
void relink_adr(FILE* database, struct SavedArtist artist_, struct SavedArtist artist, struct User* user) {
    if (artist.self_adr == user->f_u_adr)		            //* немає previous(artist_) (перший запис)
    {
        if (artist.self_adr == artist.next_adr)			    // і немає next (запис лише один)
        {
            user->f_u_adr = -1;					        // неможлива адреса
        }
        else                                            // якщо ж next є,
        {
            user->f_u_adr = artist.next_adr;              // він стає першим
        }
    }
    else                                                //* є previous
    {
        if (artist.self_adr == artist.next_adr)			    // немає next (останній запис)
        {
            artist_.next_adr = artist_.self_adr;            // next == last
        }
        else                                            // є next
        {
            artist_.next_adr = artist.next_adr;		        // next стає next of previous
        }

        fseek(database, artist_.self_adr, SEEK_SET);	    // записуємо оновлений previous (artist_)
        fwrite(&artist_, ARTIST_SIZE, 1, database);			// назад до таблички
    }
}


// занести адресу до смітника
void w_to_trash_artist(long adr) {
    FILE* garbage = fopen(ARTIST_GARBAGE, "rb");			// "rb" = відкриваємо бінарний файл для читання

    int garbage_count;
    fscanf(garbage, "%d", &garbage_count);

    long* delAddresses = malloc(garbage_count * sizeof(long));          // виділяємо місце під список "сміттєвих" адрес

    for (int i = 0; i < garbage_count; i++)
    {
        fscanf(garbage, "%ld", delAddresses + i);		    // Заповнюємо його
    }

    fclose(garbage);									// За допомогою цих двох команд
    garbage = fopen(ARTIST_GARBAGE, "wb");				// повністю очищуємо файл зі "сміттям"
    fprintf(garbage, "%d", garbage_count + 1);			// Записуємо нову кількість "сміттєвих" адрес

    for (int i = 0; i < garbage_count; i++)
    {
        fprintf(garbage, " %ld", delAddresses[i]);		// Заносимо "сміттєві" адреси назад...
    }

    fprintf(garbage, " %ld", adr);					// ...і дописуємо до них адресу щойно видаленого запису
    free(delAddresses);										// Звільняємо виділену під масив пам'ять
    fclose(garbage);
}


// update



// на вхід дається оновлена пісня, яку треба записати у файл

int update_artist(struct SavedArtist artist, int artist_id, char* error) {
    FILE* database = fopen(ARTIST_DATA, "r+b");           // open for reading and writing

    fseek(database, artist.self_adr, SEEK_SET);
    fwrite(&artist, ARTIST_SIZE, 1, database);
    fclose(database);

    return 1;
}


//insert
void overwrite_garbage_adr(int garbage_count, FILE* garbage, struct SavedArtist* record);
void reopen_db(FILE* database);
void link_adr(FILE* database, struct User user, struct SavedArtist artist);


int insert_artist(struct User user, struct SavedArtist artist, char* error) {
    artist.exists = 1;

    FILE* database = fopen(ARTIST_DATA, "a+b");                    // open for writing a binary file
    FILE* garbage = fopen(ARTIST_GARBAGE, "rb");                   // open for reading a binary file

    int garbage_count;

    fscanf(garbage, "%d", &garbage_count);                        // прочитати з файлу

    if (garbage_count)											               // якщо наявні видалені записи
    {
        overwrite_garbage_adr(garbage_count, garbage, &artist);
        reopen_db(database);								                // змінюємо режим доступу файлу
        fseek(database, artist.self_adr, SEEK_SET);			// ставимо курсор на "сміття" для подальшого перезапису
    }
    else                                                                     // якщо видалених немає, пишемо в кінець файлу
    {
        fseek(database, 0, SEEK_END);

        int dbSize = ftell(database);

        artist.self_adr = dbSize;
        artist.next_adr = dbSize;
    }

    fwrite(&artist, ARTIST_SIZE, 1, database);					// Записуємо пісню до свого файлу

    if (!user.count_artists)								            // якщо пісень ще немає, пишемо адресу першої
    {
        user.f_u_adr= artist.self_adr;
    }
    else                                                        // якщо пісні вже є, оновлюємо "адресу next" останньої
    {
        link_adr(database, user, artist);
    }

    fclose(database);											// закриваємо файл

    user.count_artists++;										// стало на одну пісню більше
    update_user(user, error);								// оновлюємо запис user

    return 1;
}

void overwrite_garbage_adr(int garbage_count, FILE* garbage, struct SavedArtist* record)
{
    long* del_ids = malloc(garbage_count * sizeof(long));		// виділяємо місце під список "сміттєвих" адрес

    for (int i = 0; i < garbage_count; i++)
    {
        fscanf(garbage, "%ld", del_ids + i);				// заповнюємо його
    }

    record->self_adr = del_ids[0];						// для запису замість логічно видаленої "сміттєвої"
    record->next_adr = del_ids[0];

    fclose(garbage);									// за допомогою цих двох команд
    fopen(ARTIST_GARBAGE, "wb");							    // повністю очищуємо файл зі "сміттям"
    fprintf(garbage, "%d", garbage_count - 1);			// записуємо нову кількість "сміттєвих" адрес

    for (int i = 1; i < garbage_count; i++)
    {
        fprintf(garbage, " %ld", del_ids[i]);				// записуємо решту "сміттєвих" адрес
    }

    free(del_ids);											// звільняємо виділену під масив пам'ять
    fclose(garbage);									// закриваємо файл
}


void reopen_db(FILE* database) {
    fclose(database);
    database = fopen(ARTIST_DATA, "r+b");
}

void link_adr(FILE* database, struct User user, struct SavedArtist artist) {
    reopen_db(database);								// змінюємо режим на "читання з та запис у будь-яке місце"

    struct SavedArtist previous;

    fseek(database, user.f_u_adr, SEEK_SET);

    for (int i = 0; i < user.count_artists; i++)		    // пробігаємо зв'язаний список до останньої поставки
    {
        fread(&previous, ARTIST_SIZE, 1, database);
        fseek(database, previous.next_adr, SEEK_SET);
    }

    previous.next_adr = artist.self_adr;				// зв'язуємо адреси
    fwrite(&previous, ARTIST_SIZE, 1, database);				// заносимо оновлений запис назад до файлу
}