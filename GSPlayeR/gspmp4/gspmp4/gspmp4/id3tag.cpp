#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef _WIN32_WCE
#include <mmsystem.h>
#endif

#include "mapplugin.h"

// for V1

#pragma pack(1)
typedef struct tID3Tagv1 {
	BYTE	tag[3];
	BYTE	trackName[30];
	BYTE	artistName[30];
	BYTE	albumName[30];
	BYTE	year[4];
	BYTE	comment[30];
	BYTE	genre;
}ID3_TAG_V1;
#pragma pack()

const LPTSTR genre_strings[] = {
	_T("Blues"), _T("Classic Rock"), _T("Country"), _T("Dance"), _T("Disco"),
	_T("Funk"), _T("Grunge"), _T("Hip-Hop"), _T("Jazz"), _T("Metal"),
	_T("New Age"), _T("Oldies"), _T("Other"), _T("Pop"), _T("R&B"),
	_T("Rap"), _T("Reggae"), _T("Rock"), _T("Techno"), _T("Industrial"),
	_T("Alternative"), _T("Ska"), _T("Death Metal"), _T("Pranks"), _T("Soundtrack"),
	_T("Euro-Techno"), _T("Ambient"), _T("Trip-Hop"), _T("Vocal"), _T("Jazz+Funk"),
	_T("Fusion"), _T("Trance"), _T("Classical"), _T("Instrumental"), _T("Acid"),
	_T("House"), _T("Game"), _T("Sound Clip"), _T("Gospel"), _T("Noise"),
	_T("Alternative Rock"), _T("Bass"), _T("Soul"), _T("Punk"), _T("Space"),
	_T("Meditative"), _T("Instrumental Pop"), _T("Instrumental Rock"), _T("Ethnic"), _T("Gothic"),
	_T("Darkwave"), _T("Techno-Industrial"), _T("Electronic"), _T("Pop-Folk"), _T("Eurodance"),
	_T("Dream"), _T("Southern Rock"), _T("Comedy"), _T("Cult"), _T("Gangsta"),
	_T("Top 40"), _T("Christian Rap"), _T("Pop/Funk"), _T("Jungle"), _T("Native American"),
	_T("Cabaret"), _T("New Wave"), _T("Psychadelic"), _T("Rave"), _T("Showtunes"),
	_T("Trailer"), _T("Lo-Fi"), _T("Tribal"), _T("Acid Punk"), _T("Acid Jazz"),
	_T("Polka"), _T("Retro"), _T("Musical"), _T("Rock & Roll"), _T("Hard Rock"),
	_T("Folk"), _T("Folk-Rock"), _T("National Folk"), _T("Swing"), _T("Fast Fusion"),
	_T("Bebob"), _T("Latin"), _T("Revival"), _T("Celtic"), _T("Bluegrass"),
	_T("Avantgarde"), _T("Gothic Rock"), _T("Progressive Rock"), _T("Psychedelic Rock"), _T("Symphonic Rock"),
	_T("Slow Rock"), _T("Big Band"), _T("Chorus"), _T("Easy Listening"), _T("Acoustic"),
	_T("Humour"), _T("Speech"), _T("Chanson"), _T("Opera"), _T("Chamber Music"),
	_T("Sonata"), _T("Symphony"), _T("Booty Bass"), _T("Primus"), _T("Porn Groove"),
	_T("Satire"), _T("Slow Jam"), _T("Club"), _T("Tango"), _T("Samba"),
	_T("Folklore"), _T("Ballad"), _T("Power Ballad"), _T("Rhythmic Soul"), _T("Freestyle"),
	_T("Duet"), _T("Punk Rock"), _T("Drum Solo"), _T("Acapella"), _T("Euro-House"),
	_T("Dance Hall"), _T("Goa"), _T("Drum & Bass"), _T("Club-House"), _T("Hardcore"),
	_T("Terror"), _T("Indie"), _T("BritPop"), _T("Negerpunk"), _T("Polsk Punk"),
	_T("Beat"), _T("Christian Gangsta Rap"), _T("Heavy Metal"), _T("Black Metal"), _T("Crossover"),
	_T("Contemporary Christian"), _T("Christian Rock"), _T("Merengue"), _T("Salsa"), _T("Thrash Metal"),
	_T("Anime"), _T("Jpop"), _T("Synthpop"), NULL
};

void ConvertFromTagStr(BYTE buff[30], LPTSTR pszBuff, int nLen)
{
	char szBuff[31];
	memset(szBuff, 0, sizeof(szBuff));
	memcpy(szBuff, buff, sizeof(BYTE) * nLen);
	for (int i = nLen - 1; szBuff[i] == ' '; i--)
		szBuff[i] = '\0';

#ifdef _UNICODE
	MultiByteToWideChar(CP_ACP, 0, (char*)szBuff, -1, pszBuff, 31);
#else
	strcpy(pszBuff, szBuff);
#endif
}

BOOL GetId3TagV1(FILE* fp, MAP_PLUGIN_FILETAG* pTag)
{
	char buff[5];
	ID3_TAG_V1 id3tag;
	BOOL bRet = FALSE;

	long curoffset = ftell(fp);
	fseek(fp, -sizeof(ID3_TAG_V1), SEEK_END);
	
	if (!fread(&id3tag, sizeof(ID3_TAG_V1), 1, fp))
		goto done;

	if (id3tag.tag[0] != 'T' || id3tag.tag[1] != 'A' || id3tag.tag[2] != 'G')
		goto done;

	bRet = TRUE;
	memset(buff, 0, sizeof(buff));
	memcpy(buff, id3tag.year, sizeof(id3tag.year));
	pTag->nYear = atoi(buff);

	ConvertFromTagStr(id3tag.albumName, pTag->szAlbum, 30);
	ConvertFromTagStr(id3tag.artistName, pTag->szArtist, 30);
	ConvertFromTagStr(id3tag.trackName, pTag->szTrack, 30);

	//ID3TAG v1.1
	if (id3tag.comment[28] == NULL) {
		pTag->nTrackNum = id3tag.comment[29];
		ConvertFromTagStr(id3tag.comment, pTag->szComment, 28);
	}
	else
		ConvertFromTagStr(id3tag.comment, pTag->szComment, 30);

	if (id3tag.genre < 148)
		_tcscpy(pTag->szGenre, genre_strings[id3tag.genre]);
	else
		memset(pTag->szGenre, 0, sizeof(pTag->szGenre));
	
done:
	fseek(fp, curoffset, SEEK_SET);
	return bRet;
}

// for V2
#define ID3TAG_HEADER_LEN	10
#define ID3TAG23_FRAME_LEN	10
#define ID3TAG20_FRAME_LEN	6

int UTF8toUCS2(char* pszSrc, WCHAR* pszDst, DWORD dwDstLen)
{
	DWORD dwLen = 0;
	while (*pszSrc) {
		if (++dwLen == dwDstLen)
			break;
		if ((*pszSrc & 0x80) == 0x0) {
			// 1 byte
			*pszDst++ = *pszSrc++;
		}
		else if ((*pszSrc & 0xE0) == 0xC0) {
			// 2 bytes
			*pszDst++ = (((WORD)*pszSrc & 0x1F) << 6) | ((WORD)*(pszSrc + 1) & 0x3F);
			pszSrc += 2;
		}
		else if ((*pszSrc & 0xE0) == 0xE0) {
			// 3 bytes
			*pszDst++ = (((WORD)*pszSrc & 0x0F) << 12) | (((WORD)*(pszSrc + 1) & 0x3F) << 6) | ((WORD)*(pszSrc + 2) & 0x3F);
			pszSrc += 3;
		}
	}

	*pszDst = NULL;
	return dwLen;
}

BOOL CopyTagStringW(BYTE *buf, int buflen, LPWSTR str, int strlen)
{
	int len, i;
	BOOL ret = FALSE;
	BYTE tmp;
	LPBYTE ptr;
	char* psz = NULL;

	if (buf[0] == 0) {
		// ANSI
		len = buflen - 1;
		psz = new char[len + 1];
		if (!psz)
			return FALSE;
		
		memset(psz, 0, len + 1);
		memcpy(psz, (char*)&buf[1], len);
		
		MultiByteToWideChar(CP_ACP, 0, psz, -1, str, strlen);
		str [strlen - 1] = NULL;
		ret = TRUE;
	}
	else if (buf[1] == 0xFF && buf[2] == 0xFE) {
		// UCS2LE with BOM
		len = buflen - 3;
		len = len < (strlen - 1) * sizeof(WCHAR) ? len : (strlen - 1) * sizeof(WCHAR);
		memcpy(str, &buf[3], len);
		str[len / 2] = NULL;
		ret = TRUE;
	}
	else if (buf[1] == 0xFE && buf[2] == 0xFF) {
		// UCS2BE with BOM
		len = buflen - 3;
		len = len < (strlen - 1) * sizeof(WCHAR) ? len : (strlen - 1) * sizeof(WCHAR);
		memcpy(str, &buf[3], len);
		str[len / 2] = NULL;

		ptr = (LPBYTE)str;
		for (i = 0; i < len / 2; i++) {
			tmp = ptr[i * 2 + 1];
			ptr[i * 2] = ptr[i * 2 + 1];
			ptr[i * 2 + 1] = tmp;
		}
		ret = TRUE;
	}
	else if (buf[0] == 2) {
		// UCS2BE
		len = buflen - 1;
		len = len < (strlen - 1) * sizeof(WCHAR) ? len : (strlen - 1) * sizeof(WCHAR);
		memcpy(str, &buf[1], len);
		str[len / 2] = NULL;

		ptr = (LPBYTE)str;
		for (i = 0; i < len / 2; i++) {
			tmp = ptr[i * 2 + 1];
			ptr[i * 2] = ptr[i * 2 + 1];
			ptr[i * 2 + 1] = tmp;
		}
		ret = TRUE;
	}
	else if (buf[0] == 3) {
		// UTF-8
		len = buflen - 1;
		psz = new char[len + 1];
		if (!psz)
			return FALSE;

		memset(psz, 0, len + 1);
		memcpy(psz, (char*)&buf[1], len);
		UTF8toUCS2(psz, str, strlen);
		ret = TRUE;
	}

	if (psz) delete [] psz;
	return ret;
}

BOOL CopyTagString(BYTE *buf, int buflen, LPTSTR str, int strlen)
{
#ifdef _UNICODE
	return CopyTagStringW(buf, buflen, str, strlen);
#else
	WCHAR szTemp[MAX_PLUGIN_TAG_STR];
	if (!CopyTagStringW(buf, buflen, szTemp, MAX_PLUGIN_TAG_STR))
		return FALSE;

	WideCharToMultiByte(CP_ACP, 0, szTemp, -1, str, strlen, NULL, NULL);
	str[strlen - 1] = NULL;
	return TRUE;
#endif
}

BOOL ParseGenre(LPTSTR src, LPTSTR str, int strlen)
{
	LPTSTR psz;
	int nGenre;

	if (src[0] == _T('(')) {
		psz = _tcschr(&src[1], _T(')'));
		if (psz) {
			*psz = NULL;
			nGenre = _tcstol(&src[1], 0, 10);

			if (nGenre < sizeof(genre_strings) / sizeof(LPTSTR)) {
				_tcsncpy(str, genre_strings[nGenre], strlen);
				str[strlen - 1] = NULL;
				return TRUE;
			}
		}
	}
	
	_tcsncpy(str, src, strlen);
	str[strlen - 1] = NULL;
	
	return TRUE;
}

BOOL ParseFrameV20(BYTE *buf, int len, MAP_PLUGIN_FILETAG* pTag)
{
	LPTSTR psz;
	TCHAR szTemp[MAX_PLUGIN_TAG_STR];
	LPBYTE data = buf + ID3TAG20_FRAME_LEN;
	int datalen = len - ID3TAG20_FRAME_LEN;
	
	if (memcmp(buf, "TT2", 3) == 0) { 
		return CopyTagString(data, datalen, pTag->szTrack, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TAL", 3) == 0) {
		return CopyTagString(data, datalen, pTag->szAlbum, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TP1", 3) == 0) {
		return CopyTagString(data, datalen, pTag->szArtist, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TCO", 3) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			return ParseGenre(szTemp, pTag->szGenre, MAX_PLUGIN_TAG_STR);
		}
	}
	else if (memcmp(buf, "TRK", 3) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			psz = _tcschr(szTemp, _T('/'));
			if (psz)
				*psz = NULL;
			pTag->nTrackNum = _tcstol(szTemp, 0, 10);
			return TRUE;
		}
	}
	else if (memcmp(buf, "TYE", 3) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			pTag->nYear = _tcstol(szTemp, 0, 10);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL ParseFrameV23(BYTE *buf, int len, MAP_PLUGIN_FILETAG* pTag)
{
	LPTSTR psz;
	TCHAR szTemp[MAX_PLUGIN_TAG_STR];
	LPBYTE data = buf + ID3TAG23_FRAME_LEN;
	int datalen = len - ID3TAG23_FRAME_LEN;
	
	if (memcmp(buf, "TIT2", 4) == 0) { 
		return CopyTagString(data, datalen, pTag->szTrack, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TALB", 4) == 0) {
		return CopyTagString(data, datalen, pTag->szAlbum, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TPE1", 4) == 0) {
		return CopyTagString(data, datalen, pTag->szArtist, MAX_PLUGIN_TAG_STR);
	}
	else if (memcmp(buf, "TCON", 4) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			return ParseGenre(szTemp, pTag->szGenre, MAX_PLUGIN_TAG_STR);
		}
	}
	else if (memcmp(buf, "TRCK", 4) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			psz = _tcschr(szTemp, _T('/'));
			if (psz)
				*psz = NULL;
			pTag->nTrackNum = _tcstol(szTemp, 0, 10);
			return TRUE;
		}
	}
	else if (memcmp(buf, "TDRC", 4) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			if (_tcslen(szTemp) > 4)
				szTemp[4] = NULL;
			pTag->nYear = _tcstol(szTemp, 0, 10);
			return TRUE;
		}
	}
	else if (memcmp(buf, "TYER", 4) == 0) {
		if (CopyTagString(data, datalen, szTemp, MAX_PLUGIN_TAG_STR)) {
			pTag->nYear = _tcstol(szTemp, 0, 10);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL ParseId3TagV20(LPBYTE buf, int buflen, MAP_PLUGIN_FILETAG* pTag)
{
	int tagsize, framesize;

	if (memcmp(buf, "ID3", 3) != 0)
		return FALSE;

	if (buf[3] > 2)
		return FALSE;

	tagsize = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | (buf[9] << 0);
	if (buflen < tagsize + ID3TAG_HEADER_LEN)
		return FALSE;

	buf += ID3TAG_HEADER_LEN;
	while (tagsize) {
		framesize = (buf[3] << 16 | buf[4] << 8 | buf[5] << 0);
		if (framesize == 0)
			break;

		if (framesize > tagsize)
			break;

		ParseFrameV20(buf, framesize + ID3TAG20_FRAME_LEN, pTag);
		tagsize -= framesize + ID3TAG20_FRAME_LEN;
		buf += framesize + ID3TAG20_FRAME_LEN;
	}
	return TRUE;
}

BOOL ParseId3TagV23(LPBYTE buf, int buflen, MAP_PLUGIN_FILETAG* pTag)
{
	int tagsize, framesize;

	if (memcmp(buf, "ID3", 3) != 0)
		return FALSE;

	if (buf[3] < 3 || buf[3] > 4)
		return FALSE;

	tagsize = (buf[6] << 21) | (buf[7] << 14) | (buf[8] << 7) | (buf[9] << 0);
	if (buflen < tagsize + ID3TAG_HEADER_LEN)
		return FALSE;

	buf += ID3TAG_HEADER_LEN;
	while (tagsize) {
		framesize = (buf[4] << 24 | buf[5] << 16 | buf[6] << 8 | buf[7] << 0) & 0x7FFFFFFF;
		if (framesize == 0)
			break;

		if (framesize > tagsize)
			break;

		ParseFrameV23(buf, framesize + ID3TAG23_FRAME_LEN, pTag);
		tagsize -= framesize + ID3TAG23_FRAME_LEN;
		buf += framesize + ID3TAG23_FRAME_LEN;
	}
	return TRUE;
}

BOOL GetId3TagV2(FILE* fp, MAP_PLUGIN_FILETAG* pTag)
{
	BYTE header[10];
	BYTE *buf = NULL; 
	BYTE *ptr = NULL; 
	BOOL ret = FALSE;
	long curoffset = ftell(fp);
	int tagsize;
	fseek(fp, 0, SEEK_SET);
	int version;

	if (fread(header, 1, ID3TAG_HEADER_LEN, fp) != ID3TAG_HEADER_LEN) {
		goto done;
	}

	if (memcmp(header, "ID3", 3) != 0) {
		goto done;
	}

	version = header[3];
	if (version > 4)
		goto done;

	tagsize = (header[6] << 21) | (header[7] << 14) | (header[8] << 7) | (header[9] << 0);
	buf = new BYTE[tagsize + ID3TAG_HEADER_LEN];
	if (!buf) {
		goto done;
	}
	memcpy(buf, header, ID3TAG_HEADER_LEN);
	if (fread(buf + ID3TAG_HEADER_LEN, 1, tagsize, fp) != tagsize) {
		goto done;
	}

	if (version < 3)
		ret = ParseId3TagV20(buf, tagsize + ID3TAG_HEADER_LEN, pTag);
	else
		ret = ParseId3TagV23(buf, tagsize + ID3TAG_HEADER_LEN, pTag);
	
done:
	if (buf) delete [] buf;
	fseek(fp, curoffset, SEEK_SET);
	return ret;
}

BOOL GetId3TagV2File(LPCTSTR pszFile, MAP_PLUGIN_FILETAG* pTag)
{
	BOOL bRet;
	FILE* fp = _tfopen(pszFile, _T("rb"));
	if (!fp)
		return FALSE;

	bRet = GetId3TagV2(fp, pTag);

	fclose(fp);
	return bRet;
}

