#ifndef BF_CONTENT_H
#define BF_CONTENT_H

#include <stddef.h>

typedef enum BFMediaType {
    BF_MEDIA_VIDEO,
    BF_MEDIA_SHORT,
    BF_MEDIA_SONG
} BFMediaType;

typedef enum BFListGroup {
    BF_LIST_VIDEO,
    BF_LIST_SHORT
} BFListGroup;

typedef struct BFAlbum {
    const wchar_t *id;
    const wchar_t *title;
    const wchar_t *subtitle;
    const wchar_t *release;
    const wchar_t *description;
} BFAlbum;

typedef struct BFGoods {
    const wchar_t *id;
    const wchar_t *name;
    const wchar_t *type;
    const wchar_t *line;
    const wchar_t *date;
    const wchar_t *price;
    const wchar_t *link;
    const wchar_t *description;
} BFGoods;

typedef struct BFMediaItem {
    const wchar_t *id;
    const wchar_t *albumId;
    const wchar_t *title;
    const wchar_t *category;
    BFMediaType type;
    const wchar_t *date;
    const wchar_t *length;
    const wchar_t *previewSource;
    const wchar_t *link;
    const wchar_t *description;
} BFMediaItem;

typedef struct BFMediaList {
    const wchar_t *name;
    BFListGroup group;
    const BFMediaItem *items;
    size_t count;
} BFMediaList;

extern const BFAlbum BF_ALBUMS[];
extern const size_t BF_ALBUM_COUNT;

extern const BFMediaItem BF_SONGS[];
extern const size_t BF_SONG_COUNT;

extern const BFGoods BF_GOODS[];
extern const size_t BF_GOODS_COUNT;

extern const BFMediaList BF_CHEON_LISTS[];
extern const size_t BF_CHEON_LIST_COUNT;

extern const BFMediaList BF_CIO_LISTS[];
extern const size_t BF_CIO_LIST_COUNT;

#endif
