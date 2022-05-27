#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#include "checks.h"

#define USER_DATA "user.fl"
#define USER_IND "user.ind"
#define USER_GARBAGE "user_garbage.txt"
#define USER_SIZE sizeof(struct User)
#define INDEXER_SIZE sizeof(struct Indexer)


//get

int get_user(struct User* user, int id, char* error) {
    FILE* ind_table = fopen(USER_IND, "rb");				// open for reading binary file
    FILE* database = fopen(USER_DATA, "rb");				//

    if (!check_file_exist(ind_table, database, error)) {
        return 0;
    }

    struct Indexer indexer;

    if (!check_ind_exist(ind_table, error, id)) {
        return 0;
    }

    fseek(ind_table, (id - 1) * INDEXER_SIZE, SEEK_SET);	// отримуємо індексатор шуканого запису
    fread(&indexer, INDEXER_SIZE, 1, ind_table);			// за вказаним номером

    if (!check_record_exist(indexer, error)) {
        return 0;
    }

    fseek(database, indexer.adr, SEEK_SET);				// отримуємо шуканий запис з БД
    fread(user, sizeof(struct User), 1, database);		// за знайденою адресою
    fclose(ind_table);										        // закриваємо файли
    fclose(database);

    return 1;
}

// del

void w_to_trash_user(int id);

int del_user(int id, char* error) {
    FILE* ind_table = fopen(USER_IND, "r+b");			// open for reading and writing binary file
    if (ind_table == NULL)
    {
        strcpy(error, "Database files are not created yet :(");
        return 0;
    }

    if (!check_ind_exist(ind_table, error, id))
    {
        return 0;
    }

    struct User user;
    get_user(&user, id, error);

    struct Indexer indexer;

    fseek(ind_table, (id - 1) * INDEXER_SIZE, SEEK_SET);	 // Отримуємо індексатор шуканого запису за вказаним номером
    fread(&indexer, INDEXER_SIZE, 1, ind_table);

    indexer.exists = 0;										                // Запис логічно не існує

    fseek(ind_table, (id - 1) * INDEXER_SIZE, SEEK_SET);

    fwrite(&indexer, INDEXER_SIZE, 1, ind_table);			// але фізично буде існувати
    fclose(ind_table);										// закриваємо файл щоб оновилось

    w_to_trash_user(id);									// заносимо індекс видаленого запису до "сміттєвої зони"


    if (user.count_artists)								// були пісні, видаляємо всі
    {
        FILE* artist_db = fopen(ARTIST_DATA, "r+b");

        struct SavedArtist artist;

        fseek(artist_db, user.f_u_adr, SEEK_SET);

        for (int i = 0; i < user.count_artists; i++)
        {
            fread(&artist, ARTIST_SIZE, 1, artist_db);
            fclose(artist_db);
            del_artist(user, artist, artist.artist_id, error);

            artist_db = fopen(ARTIST_DATA, "r+b");
            fseek(artist_db, artist.next_adr, SEEK_SET);
        }

        fclose(artist_db);
    }
    return 1;
}


void w_to_trash_user(int id) {
    FILE* garbage = fopen(USER_GARBAGE, "rb");		// read binary

    int garbage_count;
    fscanf(garbage, "%d", &garbage_count);

    int* del_ids = malloc(garbage_count * sizeof(int));		// виділяємо місце під список "сміттєвих" індексів

    for (int i = 0; i < garbage_count; i++)
    {
        fscanf(garbage, "%d", del_ids + i);				// заповнюємо його
    }

    fclose(garbage);									        // тут
    garbage = fopen(USER_GARBAGE, "wb");				// і тут повністю очищуємо файл зі "сміттям"
    fprintf(garbage, "%d", garbage_count + 1);			// записуємо нову кількість "сміттєвих" індексів

    for (int i = 0; i < garbage_count; i++)
    {
        fprintf(garbage, " %d", del_ids[i]);				// заносимо "сміттєві" індекси назад...
    }

    fprintf(garbage, " %d", id);						// дописуємо до них індекс щойно видаленого запису
    free(del_ids);											// звільняємо виділену під масив пам'ять
    fclose(garbage);
}


//update


int update_user(struct User user, char* error) {
    FILE* ind_table = fopen(USER_IND, "r+b");			// open binary file for r and w
    FILE* database = fopen(USER_DATA, "r+b");

    if (!check_file_exist(ind_table, database, error))
    {
        return 0;
    }

    struct Indexer indexer;
    int id = user.id;

    if (!check_ind_exist(ind_table, error, id))
    {
        return 0;
    }

    fseek(ind_table, (id - 1) * INDEXER_SIZE, SEEK_SET);	    // отримуємо індексатор шуканого запису за вказаним номером
    fread(&indexer, INDEXER_SIZE, 1, ind_table);

    if (!check_record_exist(indexer, error))
    {
        return 0;
    }

    fseek(database, indexer.adr, SEEK_SET);				// позиція за адресою запису в БД
    fwrite(&user, USER_SIZE, 1, database);				// оновлюємо запис
    fclose(ind_table);										// закриваємо файли
    fclose(database);

    return 1;
}


// insert
void overwrite_garbage_id(int garbage_count, FILE* garbage, struct User* record);


int insert_user(struct User record) {
    FILE* ind_table = fopen(USER_IND, "a+b");			// open binary file for r and w in the end of file
    FILE* database = fopen(USER_DATA, "a+b");
    FILE* garbage = fopen(USER_GARBAGE, "rb");		    // read binary file
    struct Indexer indexer;
    int garbage_count;

    fscanf(garbage, "%d", &garbage_count);

    if (garbage_count) {									            // якщо є "сміттєві" записи, то  ->
        overwrite_garbage_id(garbage_count, garbage, &record);  // -> перепишемо перший з них

        fclose(ind_table);									    // закриваємо файли для зміни режиму доступу в подальшому
        fclose(database);

        ind_table = fopen(USER_IND, "r+b");				// відкриваємо і змінюємо режим на
        database = fopen(USER_DATA, "r+b");				// "читання з та запис у довільне місце файлу"

        fseek(ind_table, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
        fread(&indexer, INDEXER_SIZE, 1, ind_table);
        fseek(database, indexer.adr, SEEK_SET);			// курсор -> на "сміття" для подальшого перезапису
    }
    else {                                                                // якщо сміттєвих записів немає
        long indexerSize = INDEXER_SIZE;

        fseek(ind_table, 0, SEEK_END);						// курсор -> у кінець файлу таблички

        if (ftell(ind_table)) {								// розмір індексної таблички ненульовий (позиція від початку)
            fseek(ind_table, -indexerSize, SEEK_END);		// курсор -> на останній індексатор
            fread(&indexer, INDEXER_SIZE, 1, ind_table);	// читаємо останній індексатор

            record.id = indexer.id + 1;						// нумеруємо запис наступним індексом
        }
        else                                                // індексна табличка порожня
        {
            record.id = 1;									// індексуємо наш запис як перший
        }
    }

    record.f_u_adr = -1;
    record.count_artists = 0;

    fwrite(&record, USER_SIZE, 1, database);				// записуємо в потрібне місце БД-таблички передану структуру

    indexer.id = record.id;									            // вносимо номер запису в індексатор
    indexer.adr = (record.id - 1) * USER_SIZE;		                    // вносимо адресу запису в індексатор
    indexer.exists = 1;										            // прапорець існування запису

    printf("Your user\'s id is %d.\n", record.id);

    fseek(ind_table, (record.id - 1) * INDEXER_SIZE, SEEK_SET);
    fwrite(&indexer, INDEXER_SIZE, 1, ind_table);			// записуємо індексатор у відповідну табличку, куди треба
    fclose(ind_table);										    // закриваємо файли
    fclose(database);

    return 1;
}


void overwrite_garbage_id(int garbage_count, FILE* garbage, struct User* record) {
    int* del_ids = malloc(garbage_count * sizeof(int));		        // виділяємо місце під список "сміттєвих" індексів

    for (int i = 0; i < garbage_count; i++)
    {
        fscanf(garbage, "%d", del_ids + i);				// заповнюємо його
    }

    record->id = del_ids[0];									        // для запису, замість логічно видаленого "сміттєвого"

    fclose(garbage);									            // повністю очищуємо файл зі "сміттям"
    fopen(USER_GARBAGE, "wb");
    fprintf(garbage, "%d", garbage_count - 1);			    // записуємо нову кількість "сміттєвих" індексів

    for (int i = 1; i < garbage_count; i++)
    {
        fprintf(garbage, " %d", del_ids[i]);				// записуємо решту "сміттєвих" індексів
    }

    free(del_ids);		        									// звільняємо виділену під масив пам'ять
    fclose(garbage);
}

