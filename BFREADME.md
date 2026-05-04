# BlackFix

`BFBuild.ps1`을 실행하면 `build\BlackFix.exe`가 생성됩니다.

직접 추가하거나 수정할 콘텐츠는 [BF_Content.c](C:/Users/rasud/Desktop/BlackFix/BF_Content.c)에 모여 있습니다.

앨범은 `BF_ALBUMS` 배열에 추가합니다.

노래는 `BF_SONGS` 배열에 추가합니다. `albumId`에는 연결할 앨범의 `id`를 넣고, `title`, `description`, `link`를 직접 설정하면 됩니다. 노래 카드를 누르면 `link`로 이동합니다.

굿즈는 `BF_GOODS` 배열에 추가합니다. `name`, `line`, `date`, `price`, `link`, `description`을 직접 설정할 수 있고 굿즈 카드를 누르면 `link`로 이동합니다.

천/씨오 영상과 쇼츠는 `BF_CHEON_LISTS`, `BF_CIO_LISTS`가 가리키는 `BFMediaItem` 배열에 추가합니다. 영상/쇼츠도 `title`, `description`, `link`를 직접 설정할 수 있습니다.

창 코드는 파일별로 분리되어 있습니다.

- [BF_BlackFix.c](C:/Users/rasud/Desktop/BlackFix/BF_BlackFix.c): BlackFix 메인 창
- [BF_Album.c](C:/Users/rasud/Desktop/BlackFix/BF_Album.c): 앨범별 노래 창
- [BF_Member.c](C:/Users/rasud/Desktop/BlackFix/BF_Member.c): 천/씨오 창
- [BF_Settings.c](C:/Users/rasud/Desktop/BlackFix/BF_Settings.c): 설정 창
- [BF_App.c](C:/Users/rasud/Desktop/BlackFix/BF_App.c): 공통 UI, 저장, 별표, 미리보기, 실행 관리

`previewSource`는 1분 미리보기용 로컬 파일입니다. 실행 파일 기준 상대 경로 또는 절대 경로를 사용할 수 있습니다.

별표, 테마, 소리, 언어 설정은 `%APPDATA%\BlackFix\BFState.txt`에 저장됩니다.

GitHub에 `main` 브랜치로 올리면 `.github/workflows/release.yml`이 Windows exe를 빌드하고 `latest` 릴리스에 `BlackFix.exe`와 `version.txt`를 갱신합니다.

사용자는 GitHub Release의 `BlackFix.exe`만 받으면 됩니다. GitHub 공개 저장소 특성상 소스 압축 다운로드 자체를 없앨 수는 없지만, 릴리스 자산은 exe 중심으로 구성했습니다.

앱은 시작할 때 `latest` 릴리스의 `version.txt`를 확인하고 새 버전이면 `BlackFix.exe`를 내려받아 자동으로 교체한 뒤 다시 실행합니다.
