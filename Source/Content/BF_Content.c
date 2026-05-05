#include <wchar.h>
#include "BF_Content.h"

const BFAlbum BF_ALBUMS[] = {
    {L"album_blackfix_001", L"BLACKFIX_001", L"데뷔 싱글", L"2026-05-01", L"프로그래밍 감성의 신스 사운드와 팀 BlackFix의 첫 콘셉트를 담은 앨범입니다."},
    {L"album_green_terminal", L"GREEN TERMINAL", L"미니 앨범", L"2026-07-15", L"검은 화면 위 초록 코드처럼 흐르는 보컬과 랩 트랙을 중심으로 구성했습니다."},
    {L"album_patch_note", L"PATCH NOTE", L"스페셜 앨범", L"2026-09-20", L"팬들에게 전하는 업데이트 로그 형식의 스페셜 트랙 모음입니다."}
};

const size_t BF_ALBUM_COUNT = sizeof(BF_ALBUMS) / sizeof(BF_ALBUMS[0]);

const BFMediaItem BF_SONGS[] = {
    {L"track_blackfix_boot", L"album_blackfix_001", L"Boot Sequence", L"BLACKFIX_001", BF_MEDIA_SONG, L"2026-05-01", L"03:12", L"media\\album\\boot_sequence.mp3", L"https://www.youtube.com/results?search_query=BlackFix+Boot+Sequence", L"BlackFix의 첫 시작을 알리는 인트로 트랙입니다."},
    {L"track_blackfix_fix", L"album_blackfix_001", L"Fix The Night", L"BLACKFIX_001", BF_MEDIA_SONG, L"2026-05-01", L"03:46", L"media\\album\\fix_the_night.mp3", L"https://www.youtube.com/results?search_query=BlackFix+Fix+The+Night", L"검은 밤 위로 초록빛 리듬을 쌓아 올린 타이틀 곡입니다."},
    {L"track_green_terminal", L"album_green_terminal", L"Green Terminal", L"GREEN TERMINAL", BF_MEDIA_SONG, L"2026-07-15", L"03:28", L"media\\album\\green_terminal.mp3", L"https://www.youtube.com/results?search_query=BlackFix+Green+Terminal", L"터미널 화면처럼 빠르게 전환되는 신스와 보컬이 중심입니다."},
    {L"track_patch_note", L"album_patch_note", L"Patch Note", L"PATCH NOTE", BF_MEDIA_SONG, L"2026-09-20", L"04:01", L"media\\album\\patch_note.mp3", L"https://www.youtube.com/results?search_query=BlackFix+Patch+Note", L"팬들에게 전하는 업데이트 로그 콘셉트의 스페셜 트랙입니다."}
};

const size_t BF_SONG_COUNT = sizeof(BF_SONGS) / sizeof(BF_SONGS[0]);

const BFGoods BF_GOODS[] = {
    {L"goods_pixel_tshirt", L"BF 픽셀 티셔츠", L"의류", L"BLACKFIX_001", L"2026-05-01", L"39,000원", L"https://www.youtube.com/results?search_query=BlackFix+goods+pixel+tshirt", L"BlackFix 로고와 초록 픽셀 라인을 넣은 기본 굿즈입니다."},
    {L"goods_terminal_keyring", L"터미널 키링", L"액세서리", L"GREEN TERMINAL", L"2026-07-15", L"12,000원", L"https://www.youtube.com/results?search_query=BlackFix+goods+terminal+keyring", L"작은 콘솔 창 모양의 아크릴 키링입니다."},
    {L"goods_debug_cards", L"디버그 포토카드 세트", L"포토카드", L"PATCH NOTE", L"2026-09-20", L"18,000원", L"https://www.youtube.com/results?search_query=BlackFix+goods+photo+card", L"멤버별 콘셉트 컷을 코드 카드처럼 디자인한 세트입니다."}
};

const size_t BF_GOODS_COUNT = sizeof(BF_GOODS) / sizeof(BF_GOODS[0]);

static const BFMediaItem BF_CHEON_IRI[] = {
    {L"cheon_iri_01", L"", L"천 이리 #01", L"이리", BF_MEDIA_VIDEO, L"2026-05-02", L"03:18", L"media\\cheon\\iri_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EC%9D%B4%EB%A6%AC+01", L"천의 짧은 개인 영상 첫 번째입니다."},
    {L"cheon_iri_02", L"", L"천 이리 #02", L"이리", BF_MEDIA_VIDEO, L"2026-05-20", L"04:02", L"media\\cheon\\iri_02.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EC%9D%B4%EB%A6%AC+02", L"픽셀 테마 촬영 비하인드 영상입니다."}
};

static const BFMediaItem BF_CHEON_SONGS[] = {
    {L"cheon_song_clip", L"", L"천 노래 클립", L"노래", BF_MEDIA_VIDEO, L"2026-06-03", L"02:46", L"media\\cheon\\song_clip.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EB%85%B8%EB%9E%98", L"천 보컬 파트를 모은 클립입니다."},
    {L"cheon_live_code", L"", L"천 라이브 코드", L"노래", BF_MEDIA_VIDEO, L"2026-06-24", L"03:55", L"media\\cheon\\live_code.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EB%9D%BC%EC%9D%B4%EB%B8%8C", L"라이브 버전으로 편집한 노래 영상입니다."}
};

static const BFMediaItem BF_CHEON_FULL[] = {
    {L"cheon_full_01", L"", L"천 풀영상 #01", L"풀영상", BF_MEDIA_VIDEO, L"2026-07-07", L"18:40", L"media\\cheon\\full_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%ED%92%80%EC%98%81%EC%83%81", L"천 개인 콘텐츠 풀버전입니다."}
};

static const BFMediaItem BF_CHEON_SHORT_IRI[] = {
    {L"cheon_short_iri_01", L"", L"천 이리 쇼츠 #01", L"이리", BF_MEDIA_SHORT, L"2026-07-12", L"00:45", L"media\\cheon\\short_iri_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EC%9D%B4%EB%A6%AC+shorts", L"이리 콘텐츠를 짧게 편집한 쇼츠입니다."},
    {L"cheon_short_iri_02", L"", L"천 이리 쇼츠 #02", L"이리", BF_MEDIA_SHORT, L"2026-08-02", L"00:58", L"media\\cheon\\short_iri_02.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+iri+shorts", L"팬 반응을 중심으로 편집한 이리 쇼츠입니다."}
};

static const BFMediaItem BF_CHEON_SHORT_SONGS[] = {
    {L"cheon_short_song_01", L"", L"천 노래 쇼츠 #01", L"노래", BF_MEDIA_SHORT, L"2026-08-09", L"00:51", L"media\\cheon\\short_song_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%EB%85%B8%EB%9E%98+shorts", L"천 노래 파트를 짧게 모은 쇼츠입니다."}
};

static const BFMediaItem BF_CHEON_SHORT_FULL[] = {
    {L"cheon_short_full_01", L"", L"천 풀영상 쇼츠 #01", L"풀영상", BF_MEDIA_SHORT, L"2026-08-16", L"00:59", L"media\\cheon\\short_full_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%B2%9C+%ED%92%80%EC%98%81%EC%83%81+shorts", L"풀영상 주요 장면만 모은 쇼츠입니다."}
};

const BFMediaList BF_CHEON_LISTS[] = {
    {L"이리", BF_LIST_VIDEO, BF_CHEON_IRI, sizeof(BF_CHEON_IRI) / sizeof(BF_CHEON_IRI[0])},
    {L"노래", BF_LIST_VIDEO, BF_CHEON_SONGS, sizeof(BF_CHEON_SONGS) / sizeof(BF_CHEON_SONGS[0])},
    {L"풀영상", BF_LIST_VIDEO, BF_CHEON_FULL, sizeof(BF_CHEON_FULL) / sizeof(BF_CHEON_FULL[0])},
    {L"이리", BF_LIST_SHORT, BF_CHEON_SHORT_IRI, sizeof(BF_CHEON_SHORT_IRI) / sizeof(BF_CHEON_SHORT_IRI[0])},
    {L"노래", BF_LIST_SHORT, BF_CHEON_SHORT_SONGS, sizeof(BF_CHEON_SHORT_SONGS) / sizeof(BF_CHEON_SHORT_SONGS[0])},
    {L"풀영상", BF_LIST_SHORT, BF_CHEON_SHORT_FULL, sizeof(BF_CHEON_SHORT_FULL) / sizeof(BF_CHEON_SHORT_FULL[0])}
};

const size_t BF_CHEON_LIST_COUNT = sizeof(BF_CHEON_LISTS) / sizeof(BF_CHEON_LISTS[0]);

static const BFMediaItem BF_CIO_SONGS[] = {
    {L"cio_song_clip", L"", L"씨오 노래 클립", L"노래", BF_MEDIA_VIDEO, L"2026-05-11", L"03:21", L"media\\cio\\song_clip.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+%EB%85%B8%EB%9E%98", L"씨오 보컬과 랩 파트를 모은 클립입니다."},
    {L"cio_studio", L"", L"씨오 스튜디오", L"노래", BF_MEDIA_VIDEO, L"2026-06-19", L"04:14", L"media\\cio\\studio.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+studio", L"스튜디오 녹음 분위기를 담은 영상입니다."}
};

static const BFMediaItem BF_CIO_HIGHLIGHTS[] = {
    {L"cio_highlight_01", L"", L"씨오 방송 하이라이트 #01", L"방송 하이라이트", BF_MEDIA_VIDEO, L"2026-06-30", L"07:33", L"media\\cio\\highlight_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+%ED%95%98%EC%9D%B4%EB%9D%BC%EC%9D%B4%ED%8A%B8", L"방송 중 주요 장면만 빠르게 볼 수 있는 하이라이트입니다."},
    {L"cio_highlight_02", L"", L"씨오 방송 하이라이트 #02", L"방송 하이라이트", BF_MEDIA_VIDEO, L"2026-07-18", L"06:48", L"media\\cio\\highlight_02.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+highlight", L"팬 반응이 좋았던 장면을 중심으로 구성했습니다."}
};

static const BFMediaItem BF_CIO_FULL[] = {
    {L"cio_full_01", L"", L"씨오 풀영상 #01", L"풀영상", BF_MEDIA_VIDEO, L"2026-08-05", L"22:08", L"media\\cio\\full_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+%ED%92%80%EC%98%81%EC%83%81", L"씨오 개인 콘텐츠 풀버전입니다."}
};

static const BFMediaItem BF_CIO_SHORT_SONGS[] = {
    {L"cio_short_song_01", L"", L"씨오 노래 쇼츠 #01", L"노래", BF_MEDIA_SHORT, L"2026-08-12", L"00:49", L"media\\cio\\short_song_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+%EB%85%B8%EB%9E%98+shorts", L"씨오 노래 포인트를 짧게 편집한 쇼츠입니다."}
};

static const BFMediaItem BF_CIO_SHORT_HIGHLIGHTS[] = {
    {L"cio_short_highlight_01", L"", L"씨오 방송 하이라이트 쇼츠 #01", L"방송 하이라이트", BF_MEDIA_SHORT, L"2026-08-19", L"00:55", L"media\\cio\\short_highlight_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+highlight+shorts", L"방송 하이라이트를 쇼츠 길이로 편집했습니다."}
};

static const BFMediaItem BF_CIO_SHORT_FULL[] = {
    {L"cio_short_full_01", L"", L"씨오 풀영상 쇼츠 #01", L"풀영상", BF_MEDIA_SHORT, L"2026-08-29", L"00:57", L"media\\cio\\short_full_01.mp4", L"https://www.youtube.com/results?search_query=BlackFix+%EC%94%A8%EC%98%A4+%ED%92%80%EC%98%81%EC%83%81+shorts", L"풀영상 주요 장면을 짧게 편집했습니다."}
};

const BFMediaList BF_CIO_LISTS[] = {
    {L"노래", BF_LIST_VIDEO, BF_CIO_SONGS, sizeof(BF_CIO_SONGS) / sizeof(BF_CIO_SONGS[0])},
    {L"방송 하이라이트", BF_LIST_VIDEO, BF_CIO_HIGHLIGHTS, sizeof(BF_CIO_HIGHLIGHTS) / sizeof(BF_CIO_HIGHLIGHTS[0])},
    {L"풀영상", BF_LIST_VIDEO, BF_CIO_FULL, sizeof(BF_CIO_FULL) / sizeof(BF_CIO_FULL[0])},
    {L"노래", BF_LIST_SHORT, BF_CIO_SHORT_SONGS, sizeof(BF_CIO_SHORT_SONGS) / sizeof(BF_CIO_SHORT_SONGS[0])},
    {L"방송 하이라이트", BF_LIST_SHORT, BF_CIO_SHORT_HIGHLIGHTS, sizeof(BF_CIO_SHORT_HIGHLIGHTS) / sizeof(BF_CIO_SHORT_HIGHLIGHTS[0])},
    {L"풀영상", BF_LIST_SHORT, BF_CIO_SHORT_FULL, sizeof(BF_CIO_SHORT_FULL) / sizeof(BF_CIO_SHORT_FULL[0])}
};

const size_t BF_CIO_LIST_COUNT = sizeof(BF_CIO_LISTS) / sizeof(BF_CIO_LISTS[0]);
