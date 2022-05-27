#include <stdio.h>
#include "structures.h"
#include "user.h"
#include "artist.h"

void read_user(struct User* user);

void read_song(struct SavedArtist* artist);

void print_user(struct User user);

void print_song(struct SavedArtist artist, struct User user);


int main() {
    struct User user;
    struct SavedArtist artist;

    while (1) {
        int i;
        int id;
        char error[51];

        printf("Choose option:\n"
               "0 - Quit\n"
               "1 - Insert User\n"
               "2 - Get User\n"
               "3 - Update User\n"
               "4 - Delete User\n"
               "5 - Insert Artist\n"
               "6 - Get Artist\n"
               "7 - Update Artist\n"
               "8 - Delete Artist\n");

        scanf("%d", &i);

        switch (i) {
            case 0:
                return 0;
            case 1:
                read_user(&user);
                insert_user(user);
                break;
            case 2:
                printf("Enter id: ");
                scanf("%d", &id);

                get_user(&user, id, error) ? print_user(user) : printf("Error: %s\n", error);
                break;
            case 3:
                printf("Enter id: ");
                scanf("%d", &id);

                user.id = id;

                read_user(&user);
                update_user(user, error) ? printf("Updated!\n") : printf("Error: %s\n", error);
                break;

            case 4:
                printf("Enter id: ");
                scanf("%d", &id);
                del_user(id, error) ? printf("Deleted!\n") : printf("Error: %s\n", error);
                break;

            case 5:
                printf("Enter user\'s id: ");
                scanf("%d", &id);

                if (get_user(&user, id, error)) {
                    artist.user_id = id;

                    printf("Enter artist id: ");
                    scanf("%d", &id);
                    artist.artist_id = id;

                    read_song(&artist);
                    insert_artist(user, artist, error);
                    printf("Inserted! To access, use user\'s and artist\'s id-s\n");
                }
                else {
                    printf("Error: %s\n", error);
                }

                break;

            case 6:
                printf("Enter user\'s id: ");
                scanf("%d", &id);

                if (get_user(&user, id, error)) {
                    printf("Enter artist id: ");
                    scanf("%d", &id);
                    get_artist(user, &artist, id, error) ? print_song(artist, user) : printf("Error: %s\n", error);
                }
                else {
                    printf("Error: %s\n", error);
                }

                break;

            case 7:
                printf("Enter user\'s id: ");
                scanf("%d", &id);

                if (get_user(&user, id, error)) {
                    printf("Enter artist id: ");
                    scanf("%d", &id);

                    if (get_artist(user, &artist, id, error)) {
                        read_song(&artist);
                        update_artist(artist, id, error);
                        printf("Updated!\n");
                    }
                    else {
                        printf("Error: %s\n", error);
                    }
                }
                else {
                    printf("Error: %s\n", error);
                }

                break;

            case 8:
                printf("Enter user\'s id: ");
                scanf("%d", &id);

                if (get_user(&user, id, error)) {
                    printf("Enter artist id: ");
                    scanf("%d", &id);

                    if (get_artist(user, &artist, id, error)) {
                        del_artist(user, artist, id, error);
                        printf("Deleted!\n");
                    }
                    else {
                        printf("Error: %s\n", error);
                    }
                }
                else {
                    printf("Error: %s\n", error);
                }

                break;

            default:
                printf("Error input, please try again!\n");
        }

        printf("---------\n");
    }

    return 0;
}



void read_user(struct User* user) {
    char nickname[15];
    char status[10];

    nickname[0] = '\0';
    status[0] = '\0';

    printf("Enter user\'s nickname: ");
    scanf("%s", nickname);
    strcpy(user->nickname, nickname);

    printf("Enter user\'s status: ");
    scanf("%s", status);

    strcpy(user->status, status);
}


void read_song(struct SavedArtist* artist) {
    char name[20];
    int joined_year;

    name[0] = '\0';

    printf("Enter name: ");
    scanf("%s", name);
    strcpy(artist->name, name);


    printf("Enter release_year: ");
    scanf("%d", &joined_year);
    artist->joined_year = joined_year;
}


void print_user(struct User user)
{
    printf("User\'s nickname: %s\n", user.nickname);
    printf("User\'s status: %s\n", user.status);
}

void print_song(struct SavedArtist artist, struct User user)
{
    printf("User: %s, status: %s \n", user.nickname, user.status);
    printf("Name of saved artist with this id: %s\n", artist.name);
    printf("Year of joining: %d\n", artist.joined_year);
}