#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
//#include <stdint.h>
#include <string.h>
#include <locale.h>

#ifdef _MSC_VER
#if _MSC_VER < 1200
// VC++ < 6.0 doesn't support unsigned __int64 to double (VC5 untested)
typedef __int64 uint64_t;
#else
typedef unsigned __int64 uint64_t;
#endif
#endif
#ifndef INVALID_FILE_ATTRIBUTES
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#endif

char *fdigitsA(uint64_t integer);
char *ComputerUnits(uint64_t integer, int maxdiv);
void DrawGauge(int length, int percent, int isempty);
int IsTargetDrive(int DriveType, int flgNoRemoveable, int flgIsRealFloppy);
void ShowVolume(char* VolumeName, int flgShowDevName, int flgVerbose, int flgUnit, int flgGauge, int flgNoRemoveable, uint64_t* AllTotalSize, uint64_t* AllFreeSize);
static INT ConvertULargeInteger(uint64_t num, CHAR cThousandSeparator, LPSTR des, INT len);

static INT ConvertULargeInteger(uint64_t num, CHAR cThousandSeparator, LPSTR des, INT len)
{
	CHAR temp[32];
	INT c = 0;
	INT n = 0;

	if (num == 0)
	{
		des[0] = '0';
		des[1] = '\0';
		n = 1;
	}
	else
	{
		temp[31] = 0;
		while (num > 0)
		{
			if (cThousandSeparator && ((c + 1) % (3 + 1)) == 0)
				temp[30 - c++] = cThousandSeparator;
			temp[30 - c++] = (CHAR)(num % 10) + '0';
			num /= 10;
		}

		for (n = 0; n <= c; n++)
			des[n] = temp[31 - c + n];
	}

	return n;
}

#define MAX_PLACES 64
char *fdigitsA(uint64_t integer) {
    static char fdigits[MAX_PLACES + (MAX_PLACES / 3) + 1];
    char digits[MAX_PLACES + 1];
    char *dgt = digits, *fdgt = fdigits;
    int places;

    places = ConvertULargeInteger(integer, ',', digits, MAX_PLACES + 1); /* convert integer to string */
    /* places = digits in string */
    while (*fdgt++ = *dgt++); /* while there are more digits*/
        //if (--places) /* if places-1 > 0 */
            //if ( !(places % 3) ) /* if places is multiple of 3 */
                //*fdgt++ = ','; /* insert a comma here */

    return fdigits;
}

char *ComputerUnits(uint64_t integer, int maxdiv) {
    static char fdigits[MAX_PLACES + (MAX_PLACES / 3) + 1];
    char *units[] = {"B","KB","MB","GB","TB","PB","EB", NULL};
    char tmp[16] = {0};
    int count = 0;
    double smallfloat;
    uint64_t smallint;

    for(smallint = smallfloat = integer; smallint > 1024 && count < maxdiv;) {
        count++;
        smallint /= 1024;
        smallfloat /= 1024;
    }

    if(count) {
        sprintf(tmp, "%.3f", smallfloat-smallint);
        tmp[4] = 0; // truncate
    }

    sprintf(fdigits, "%s%s %s", fdigitsA(smallint), tmp+1, units[count]); /* convert integer to string */

    return fdigits;
}

void DrawGauge(int length, int percent, int isempty) {
    int i,barlong,remain;
    char tmp[100];

    barlong = (length-2)*percent/100;
    remain = (length-2)-barlong;
    putchar('|');
    if(!isempty) {
        for(i=0; i<barlong; ++i)
            putchar('.');
        for(i=0; i<remain; ++i)
            putchar('#');
    } else {
        sprintf(tmp,"%%%ds",length-2);
        printf(tmp,"");
    }
    putchar('|');
}

int IsTargetDrive(int DriveType, int flgRemoveable, int flgIsRealFloppy) {
    int ret = 0;
    if(!flgRemoveable)
        ret = DriveType > 1 && DriveType != 2 && DriveType != 4 && DriveType != 5;
    if(flgRemoveable & 1)
        ret |= DriveType > 1 && DriveType != 4 && !flgIsRealFloppy;
    if(flgRemoveable & 2)
        ret |= DriveType > 1 && DriveType != 4;
    if(flgRemoveable & 4)
        ret |= DriveType > 1 && DriveType != 2 && DriveType != 5;
    return ret;
}

void ShowVolume(char* VolumeName, int flgShowDevName, int flgVerbose, int flgUnit, int flgGauge, int flgRemoveable, uint64_t* AllTotalSize, uint64_t* AllFreeSize) {
    DWORD  Error                = ERROR_SUCCESS;
    DWORD  CharCount            = 0;
    char   tmp[MAX_PATH]        = "";
    char   DeviceName[MAX_PATH] = "";
    char*  VolumeTypeName       = 0;
    UINT   VolumeType           = 0;
    size_t Index                = 0;
    DWORD  SectorsPerCluster    = 0;
    DWORD  BytesPerSector       = 0;
    DWORD  NumberOfFreeClusters = 0;
    DWORD  TotalNumberOfClusters= 0;
    uint64_t TotalSize          = 0;
    uint64_t FreeSize           = 0;

    UINT   flgIsRealFloppy      = 0;

    double den                  = 0.0f;
    double percent              = 0.0f;

    strcpy(tmp, VolumeName);

    VolumeType = GetDriveTypeA(tmp);
    switch(VolumeType) {
        case 0:
            VolumeTypeName = "(Unknown)";
            break;
        case 1:
            VolumeTypeName = "(invalid root path)";
            break;
        case 2:
            VolumeTypeName = "(Removable)";
            break;
        case 3:
            VolumeTypeName = "(Fixed)";
            break;
        case 4:
            VolumeTypeName = "(Remote)";
            break;
        case 5:
            VolumeTypeName = "(CD-ROM)";
            break;
        case 6:
            VolumeTypeName = "(RAM Disk)";
            break;
    }

    if(!flgVerbose && !IsTargetDrive(VolumeType,7,0)) return;
    // get free space information
    if(IsTargetDrive(VolumeType,flgRemoveable,flgIsRealFloppy)) {
        GetDiskFreeSpaceA(tmp, &SectorsPerCluster, &BytesPerSector, &NumberOfFreeClusters, &TotalNumberOfClusters);
        FreeSize = TotalSize = SectorsPerCluster;
        FreeSize *= BytesPerSector;
        TotalSize *= BytesPerSector;
        FreeSize *= NumberOfFreeClusters;
        TotalSize *= TotalNumberOfClusters;

        if(VolumeType == 3) {
            *AllTotalSize += TotalSize;
            *AllFreeSize += FreeSize;
        }

        den = TotalSize;
        percent = FreeSize;
        percent /= den;
        percent *= 100;
    }

    if(flgVerbose) {
        printf("\nFound a device: %s", DeviceName);
        printf("\nVolume name: %s %s", tmp, VolumeTypeName);
        printf("\nPaths:");
    }
    else if(flgGauge) {
        DrawGauge(36,(int)percent,!IsTargetDrive(VolumeType,flgRemoveable,flgIsRealFloppy)|!TotalSize);
    }

    if (!flgShowDevName)
        printf("  %s", tmp);


    if(IsTargetDrive(VolumeType,flgRemoveable,flgIsRealFloppy)) {
        // VolumeName is reused here
        strcpy(tmp, ComputerUnits(FreeSize, flgUnit));

        // DeviceName is reused here
        if(TotalSize)
            sprintf(DeviceName, "%.2f", percent);
        else
            strcpy(DeviceName, "--");

        printf(" (%s / %s) %s%%", tmp, ComputerUnits(TotalSize, flgUnit), DeviceName);
    }
    if(!flgVerbose) {
        if(VolumeType != 3) printf(" %s", VolumeTypeName);
    }
    printf("\n");
}


int main(int argc, char **argv)
{
    DWORD  Error                = ERROR_SUCCESS;
    HANDLE FindHandle           = INVALID_HANDLE_VALUE;
    BOOL   Found                = FALSE;
    size_t Index                = 0;
    BOOL   Success              = FALSE;
    char   VolumeName[MAX_PATH] = "";
    DWORD  dwAttrib             = 0;

    uint64_t allTotal           = 0;
    uint64_t allFree            = 0;
    double   percent            = 0.0f;

    int i               = 1;
    int flgVerbose      = 0;
    int flgUnit         = 3;
    int flgGauge        = 0;
    int flgRemoveable   = 0;
    int flgNonEnum      = 0;
    int flgForceEnum    = 0;
    int flgShowAllTotal = 0;
    int OldMode; //a place to store the old error mode

    //save the old error mode and set the new mode to let us do the work:
    OldMode = SetErrorMode(SEM_FAILCRITICALERRORS); 

    for (; i < argc; i++) {
        if(stricmp(argv[i],"-g") == 0) {
            flgGauge = 1;
        } else if(strnicmp(argv[i],"-u",2) == 0) {
            if(argv[i][2] >= '0' && argv[i][2] <= '4')
                flgUnit = argv[i][2] - '0';
        } else if(strnicmp(argv[i],"-r",2) == 0) {
            flgRemoveable |= 1;
            if(argv[i][2] == 'f')
                flgRemoveable |= 2;
        } else if(stricmp(argv[i],"-n") == 0) {
            flgRemoveable |= 4;
        } else if(stricmp(argv[i],"-v") == 0) {
            flgVerbose = 1;
        } else if(stricmp(argv[i],"-a") == 0) {
            flgForceEnum = 1;
        } else if(stricmp(argv[i],"-t") == 0) {
            flgShowAllTotal = 1;
        } else if(strcmp(argv[i],"-?") == 0) {
            printf("%s [-v] [-g] [-a] [-r[f]] [-n] [-u#] [-t] [-?] [drive|path...]\n\n"
                " -v\t\tbe verbose (-g switch will be turned off)\n"
                " -g\t\tdraw gauge of free space\n"
                " -a\t\tappend drive|path to detected list\n"
                " -r[f]\t\tdetect removable device free space (-rf for real floppy drive)\n"
                " -n\t\tdetect mounted network drive free space\n"
                " -u[num]\tuse unit # (0=byte, 1=KB, 2=MB, 3=GB, 4=TB)\n"
                " -t\t\tshow total and free of all local fixed drives\n"
                " -?\t\tshow this help and exit\n"
                " drive|path...\tshow individual drives\n"
                " \t\texample:  C: D: F:\\mountpoint\\\n", argv[0]);
            return 0;
        } else {
            // assume it is a path
            dwAttrib = GetFileAttributesA(argv[i]);
            if(dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
                // yes it is valid path
                if(!flgForceEnum)
                    flgNonEnum = 1;
                // dwAttrib is reused
                strcpy(VolumeName, argv[i]);
                dwAttrib = strlen(VolumeName);
                // workaround on getting free space from \\host\share
                if (VolumeName[dwAttrib-2] != '\\') VolumeName[dwAttrib-1] = '\\';
                ShowVolume(VolumeName, 0, flgVerbose, flgUnit, flgGauge, flgRemoveable, &allTotal, &allFree);
            } else {
                printf("%s is not valid path.\n", argv[i]);
                return 0;
            }
        }
    }

    if(!flgNonEnum) {

        strcpy(VolumeName,"A:\\");
        if(flgRemoveable & 2) i = 0;
        else i = 2;
        // i is reused here
        for (; i< 26 ; i++) {
        	VolumeName[0] = 'A' + i;
            ShowVolume(VolumeName, 0, flgVerbose, flgUnit, flgGauge, flgRemoveable, &allTotal, &allFree);
        }

        FindHandle = INVALID_HANDLE_VALUE;

    }

    if(flgShowAllTotal) {
        percent = allFree;
        percent /= allTotal;
        percent *= 100;

        if(flgVerbose)
            printf("\n");

        if(!flgVerbose && flgGauge) {
            DrawGauge(36,(int)percent,0);
        }
        // VolumeName is reused here
        strcpy(VolumeName, ComputerUnits(allFree, flgUnit));

        printf("Total: (%s / %s) %.2f%%\n", VolumeName, ComputerUnits(allTotal, flgUnit), percent);
    }

    SetErrorMode(OldMode); //put things back the way they were
    return 0;
}

