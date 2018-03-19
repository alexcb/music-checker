#pragma once

#include <alsa/asoundlib.h>
//#include <ao/ao.h>
#include <mpg123.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <pulse/simple.h>

#include "circular_buffer.h"
#include "play_queue.h"
#include "playlist_manager.h"
#include "httpget.h"

#define PLAYER_ARTIST_LEN 1024
#define PLAYER_TITLE_LEN 1024

#define TRACK_CHANGE_IMMEDIATE 1
#define TRACK_CHANGE_NEXT 2

#define AUDIO_DATA 1
#define ID_DATA_START 2
#define ID_DATA_END 3


typedef void (*MetadataObserver)(bool playing, const PlaylistItem *playlist_item, void *data);

typedef int (*AudioConsumer)(const char *p, size_t n);

typedef struct Player
{
	int driver;
	snd_pcm_t *alsa_handle;
	pa_simple *pa_handle;

	//ao_sample_format format;
	int channels, encoding;
	long rate;

	mpg123_handle *mh;

	volatile bool next_track;

	pthread_t audio_thread;
	pthread_t reader_thread;

	CircularBuffer circular_buffer;

	PlayQueue play_queue;
	pthread_mutex_t the_lock;

	PlaylistManager *playlist_manager;

	int metadata_observers_num;
	int metadata_observers_cap;
	MetadataObserver *metadata_observers;
	void **metadata_observers_data;

	pthread_cond_t load_cond;
	pthread_cond_t done_track_cond;

	PlaylistItem *current_track;

	PlaylistItem *playlist_item_to_buffer;
	PlaylistItem *playlist_item_to_buffer_override;

	const char *library_path;

	// when true play, when false, pause / stop
	volatile bool playing;
	volatile bool exit;

	// control over changing tracks
	//pthread_mutex_t change_track_lock;
	bool load_in_progress;
	bool load_abort_requested;
	
	size_t max_payload_size;

	size_t decode_buffer_size;
	char *decode_buffer;

	AudioConsumer audio_consumer;
} Player;


int init_player( Player *player, const char *library_path );
int start_player( Player *player );
int stop_player( Player *player );

void player_lock( Player *player );
void player_unlock( Player *player );
//void player_rewind_buffer_unsafe( Player *player );

int player_add_metadata_observer( Player *player, MetadataObserver observer, void *data );

int player_change_track( Player *player, PlaylistItem *playlist_item, int when );
int player_change_next_album( Player *player, int when );

int player_notify_item_change( Player *player, PlaylistItem *playlist_item );

void player_set_playing( Player *player, bool playing );
