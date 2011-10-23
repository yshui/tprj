#include <stdio.h>
#include <limits.h>
#include <libintl.h>
#include "fcitx/fcitx.h"
#include "fcitx-utils/log.h"
#include "fcitx-config/xdg.h"
#include "fcitx-utils/utf8.h"
#include "tabledict.h"
#include <unistd.h>

#define TEMP_FILE       "FCITX_TABLE_TEMP"

const int iInternalVersion = INTERNAL_VERSION;

boolean LoadTableDict(TableMetaData* tableMetaData)
{
    char            strCode[MAX_CODE_LENGTH + 1];
    char            strHZ[PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1];
    FILE           *fpDict;
    RECORD         *recTemp;
    char            strPath[PATH_MAX];
    unsigned int    i = 0;
    unsigned int    iTemp, iTempCount;
    char            cChar = 0, cTemp;
    char            iVersion = 1;
    int             iRecordIndex;
    char           *pstr;
    TableDict      *tableDict;
    tableMetaData->tableDict = fcitx_malloc0(sizeof(TableDict));
    tableDict = tableMetaData->tableDict;

    //读入码表
    FcitxLog(DEBUG, _("Loading Table Dict"));

    fpDict = GetXDGFileWithPrefix("table", tableMetaData->strPath, "r", &pstr);
    FcitxLog(INFO, _("Load Table Dict from %s"), pstr);
    free(pstr);

    if (!fpDict) {
        FcitxLog(DEBUG, _("Cannot load table file: %s"), strPath);
        return false;
    }

    //先读取码表的信息
    //判断版本信息
    fread(&iTemp, sizeof(unsigned int), 1, fpDict);
    if (!iTemp) {
        fread(&iVersion, sizeof(char), 1, fpDict);
        iVersion = (iVersion < INTERNAL_VERSION);
        fread(&iTemp, sizeof(unsigned int), 1, fpDict);
    }

    tableDict->strInputCode = (char *) realloc(tableDict->strInputCode, sizeof(char) * (iTemp + 1));
    fread(tableDict->strInputCode, sizeof(char), iTemp + 1, fpDict);
    /*
     * 建立索引，加26是为了为拼音编码预留空间
     */
    tableDict->recordIndex = (RECORD_INDEX *) fcitx_malloc0((strlen(tableDict->strInputCode) + 26) * sizeof(RECORD_INDEX));
    for (iTemp = 0; iTemp < strlen(tableDict->strInputCode) + 26; iTemp++) {
        tableDict->recordIndex[iTemp].cCode = 0;
        tableDict->recordIndex[iTemp].record = NULL;
    }
    /* ********************************************************************** */

    fread(&(tableDict->iCodeLength), sizeof(unsigned char), 1, fpDict);
    if (tableMetaData->iTableAutoSendToClient == -1)
        tableMetaData->iTableAutoSendToClient = tableDict->iCodeLength;
    if (tableMetaData->iTableAutoSendToClientWhenNone == -1)
        tableMetaData->iTableAutoSendToClientWhenNone = tableDict->iCodeLength;

    if (!iVersion)
        fread(&(tableDict->iPYCodeLength), sizeof(unsigned char), 1, fpDict);
    else
        tableDict->iPYCodeLength = tableDict->iCodeLength;

    fread(&iTemp, sizeof(unsigned int), 1, fpDict);
    tableDict->strIgnoreChars = (char *) fcitx_malloc0(sizeof(char) * (iTemp + 1));
    fread(tableDict->strIgnoreChars, sizeof(char), iTemp + 1, fpDict);

    fread(&(tableDict->bRule), sizeof(unsigned char), 1, fpDict);

    if (tableDict->bRule) { //表示有组词规则
        tableDict->rule = (RULE *) fcitx_malloc0(sizeof(RULE) * (tableDict->iCodeLength - 1));
        for (i = 0; i < tableDict->iCodeLength - 1; i++) {
            fread(&(tableDict->rule[i].iFlag), sizeof(unsigned char), 1, fpDict);
            fread(&(tableDict->rule[i].iWords), sizeof(unsigned char), 1, fpDict);
            tableDict->rule[i].rule = (RULE_RULE *) fcitx_malloc0(sizeof(RULE_RULE) * tableDict->iCodeLength);
            for (iTemp = 0; iTemp < tableDict->iCodeLength; iTemp++) {
                fread(&(tableDict->rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpDict);
                fread(&(tableDict->rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpDict);
                fread(&(tableDict->rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpDict);
            }
        }
    }

    tableDict->recordHead = (RECORD *) fcitx_malloc0(sizeof(RECORD));
    tableDict->currentRecord = tableDict->recordHead;

    fread(&(tableDict->iRecordCount), sizeof(unsigned int), 1, fpDict);

    for (i = 0; i < SINGLE_HZ_COUNT; i++)
        tableDict->tableSingleHZ[i] = (RECORD *) NULL;

    iRecordIndex = 0;
    for (i = 0; i < tableDict->iRecordCount; i++) {
        fread(strCode, sizeof(char), tableDict->iPYCodeLength + 1, fpDict);
        fread(&iTemp, sizeof(unsigned int), 1, fpDict);
        fread(strHZ, sizeof(char), iTemp, fpDict);
        recTemp = (RECORD *) fcitx_malloc0(sizeof(RECORD));
        recTemp->strCode = (char *) fcitx_malloc0(sizeof(char) * (tableDict->iPYCodeLength + 1));
        memset(recTemp->strCode, 0, sizeof(char) * (tableDict->iPYCodeLength + 1));
        strcpy(recTemp->strCode, strCode);
        recTemp->strHZ = (char *) fcitx_malloc0(sizeof(char) * iTemp);
        strcpy(recTemp->strHZ, strHZ);

        if (!iVersion) {
            fread(&cTemp, sizeof(char), 1, fpDict);
            recTemp->bPinyin = cTemp;
        }

        fread(&(recTemp->iHit), sizeof(unsigned int), 1, fpDict);
        fread(&(recTemp->iIndex), sizeof(unsigned int), 1, fpDict);
        if (recTemp->iIndex > tableDict->iTableIndex)
            tableDict->iTableIndex = recTemp->iIndex;

        /* 建立索引 */
        if (cChar != recTemp->strCode[0]) {
            cChar = recTemp->strCode[0];
            tableDict->recordIndex[iRecordIndex].cCode = cChar;
            tableDict->recordIndex[iRecordIndex].record = recTemp;
            iRecordIndex++;
        }
        /* **************************************************************** */
        /** 为单字生成一个表   */
        if (utf8_strlen(recTemp->strHZ) == 1 && !IsIgnoreChar(tableDict, strCode[0]) && !recTemp->bPinyin) {
            iTemp = CalHZIndex(recTemp->strHZ);
            if (iTemp < SINGLE_HZ_COUNT) {
                if (tableDict->tableSingleHZ[iTemp]) {
                    if (strlen(strCode) > strlen(tableDict->tableSingleHZ[iTemp]->strCode))
                        tableDict->tableSingleHZ[iTemp] = recTemp;
                } else
                    tableDict->tableSingleHZ[iTemp] = recTemp;
            }
        }

        if (recTemp->bPinyin)
            tableDict->bHasPinyin = true;

        tableDict->currentRecord->next = recTemp;
        recTemp->prev = tableDict->currentRecord;
        tableDict->currentRecord = recTemp;
    }

    tableDict->currentRecord->next = tableDict->recordHead;
    tableDict->recordHead->prev = tableDict->currentRecord;

    fclose(fpDict);
    FcitxLog(DEBUG, _("Load Table Dict OK"));

    //读取相应的特殊符号表
    fpDict = GetXDGFileWithPrefix("table", tableMetaData->strSymbolFile, "rt", NULL);

    if (fpDict) {
        tableDict->iFH = CalculateRecordNumber(fpDict);
        tableDict->fh = (FH *) fcitx_malloc0(sizeof(FH) * tableDict->iFH);

        for (i = 0; i < tableDict->iFH; i++) {
            if (EOF == fscanf(fpDict, "%s\n", tableDict->fh[i].strFH))
                break;
        }
        tableDict->iFH = i;

        fclose(fpDict);
    }

    tableDict->strNewPhraseCode = (char *) fcitx_malloc0(sizeof(char) * (tableDict->iCodeLength + 1));
    tableDict->strNewPhraseCode[tableDict->iCodeLength] = '\0';

    tableDict->iAutoPhrase = 0;
    if (tableMetaData->bAutoPhrase) {
        //为自动词组分配空间
        tableDict->autoPhrase = (AUTOPHRASE *) fcitx_malloc0(sizeof(AUTOPHRASE) * AUTO_PHRASE_COUNT);

        //读取上次保存的自动词组信息
        FcitxLog(DEBUG, _("Loading Autophrase."));

        strcpy(strPath, tableMetaData->uniqueName);
        strcat(strPath, "_LastAutoPhrase.tmp");
        fpDict = GetXDGFileWithPrefix("table", strPath, "rb", NULL);
        i = 0;
        if (fpDict) {
            fread(&tableDict->iAutoPhrase, sizeof(unsigned int), 1, fpDict);

            for (; i < tableDict->iAutoPhrase; i++) {
                tableDict->autoPhrase[i].strCode = (char *) fcitx_malloc0(sizeof(char) * (tableDict->iCodeLength + 1));
                tableDict->autoPhrase[i].strHZ = (char *) fcitx_malloc0(sizeof(char) * (PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1));
                fread(tableDict->autoPhrase[i].strCode, tableDict->iCodeLength + 1, 1, fpDict);
                fread(tableDict->autoPhrase[i].strHZ, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1, 1, fpDict);
                fread(&iTempCount, sizeof(unsigned int), 1, fpDict);
                tableDict->autoPhrase[i].iSelected = iTempCount;
                if (i == AUTO_PHRASE_COUNT - 1)
                    tableDict->autoPhrase[i].next = &tableDict->autoPhrase[0];
                else
                    tableDict->autoPhrase[i].next = &tableDict->autoPhrase[i + 1];
            }
            fclose(fpDict);
        }

        for (; i < AUTO_PHRASE_COUNT; i++) {
            tableDict->autoPhrase[i].strCode = (char *) fcitx_malloc0(sizeof(char) * (tableDict->iCodeLength + 1));
            tableDict->autoPhrase[i].strHZ = (char *) fcitx_malloc0(sizeof(char) * (PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1));
            tableDict->autoPhrase[i].iSelected = 0;
            if (i == AUTO_PHRASE_COUNT - 1)
                tableDict->autoPhrase[i].next = &tableDict->autoPhrase[0];
            else
                tableDict->autoPhrase[i].next = &tableDict->autoPhrase[i + 1];
        }

        if (i == AUTO_PHRASE_COUNT)
            tableDict->insertPoint = &tableDict->autoPhrase[0];
        else
            tableDict->insertPoint = &tableDict->autoPhrase[i - 1];

        FcitxLog(DEBUG, _("Load Autophrase OK"));
    } else
        tableDict->autoPhrase = (AUTOPHRASE *) NULL;

    return true;
}

void SaveTableDict(TableMetaData *tableMetaData)
{
    RECORD         *recTemp;
    char            strPathTemp[PATH_MAX];
    char            strPath[PATH_MAX];
    char       *pstr;
    FILE           *fpDict;
    unsigned int    iTemp;
    unsigned int    i;
    char            cTemp;
    TableDict      *tableDict = tableMetaData->tableDict;

    if (!tableDict->iTableChanged)
        return;

    fpDict = GetXDGFileUserWithPrefix("table", TEMP_FILE, "wb", &pstr);
    strcpy(strPathTemp, pstr);
    free(pstr);

    //写入版本号--如果第一个字为0,表示后面那个字节为版本号，为了与老版本兼容
    iTemp = 0;
    fwrite(&iTemp, sizeof(unsigned int), 1, fpDict);
    fwrite(&iInternalVersion, sizeof(char), 1, fpDict);

    iTemp = strlen(tableDict->strInputCode);
    fwrite(&iTemp, sizeof(unsigned int), 1, fpDict);
    fwrite(tableDict->strInputCode, sizeof(char), iTemp + 1, fpDict);
    fwrite(&(tableDict->iCodeLength), sizeof(char), 1, fpDict);
    fwrite(&(tableDict->iPYCodeLength), sizeof(char), 1, fpDict);
    iTemp = strlen(tableDict->strIgnoreChars);
    fwrite(&iTemp, sizeof(unsigned int), 1, fpDict);
    fwrite(tableDict->strIgnoreChars, sizeof(char), iTemp + 1, fpDict);

    fwrite(&(tableDict->bRule), sizeof(unsigned char), 1, fpDict);
    if (tableDict->bRule) { //表示有组词规则
        for (i = 0; i < tableDict->iCodeLength - 1; i++) {
            fwrite(&(tableDict->rule[i].iFlag), sizeof(unsigned char), 1, fpDict);
            fwrite(&(tableDict->rule[i].iWords), sizeof(unsigned char), 1, fpDict);
            for (iTemp = 0; iTemp < tableDict->iCodeLength; iTemp++) {
                fwrite(&(tableDict->rule[i].rule[iTemp].iFlag), sizeof(unsigned char), 1, fpDict);
                fwrite(&(tableDict->rule[i].rule[iTemp].iWhich), sizeof(unsigned char), 1, fpDict);
                fwrite(&(tableDict->rule[i].rule[iTemp].iIndex), sizeof(unsigned char), 1, fpDict);
            }
        }
    }

    fwrite(&(tableDict->iRecordCount), sizeof(unsigned int), 1, fpDict);
    recTemp = tableDict->recordHead->next;
    while (recTemp != tableDict->recordHead) {
        fwrite(recTemp->strCode, sizeof(char), tableDict->iPYCodeLength + 1, fpDict);

        iTemp = strlen(recTemp->strHZ) + 1;
        fwrite(&iTemp, sizeof(unsigned int), 1, fpDict);
        fwrite(recTemp->strHZ, sizeof(char), iTemp, fpDict);

        cTemp = recTemp->bPinyin;
        fwrite(&cTemp, sizeof(char), 1, fpDict);
        fwrite(&(recTemp->iHit), sizeof(unsigned int), 1, fpDict);
        fwrite(&(recTemp->iIndex), sizeof(unsigned int), 1, fpDict);
        recTemp = recTemp->next;
    }

    fclose(fpDict);
    fpDict = GetXDGFileUserWithPrefix("table", tableMetaData->strPath, NULL, &pstr);
    if (access(pstr, 0))
        unlink(pstr);
    rename(strPathTemp, pstr);
    free(pstr);

    FcitxLog(DEBUG, _("Rename OK"));

    tableDict->iTableChanged = 0;

    if (tableDict->autoPhrase) {
        //保存上次的自动词组信息
        fpDict = GetXDGFileUserWithPrefix("table", TEMP_FILE, "wb", &pstr);
        strncpy(strPathTemp, pstr, PATH_MAX);
        if (fpDict) {
            fwrite(&tableDict->iAutoPhrase, sizeof(int), 1, fpDict);
            for (i = 0; i < tableDict->iAutoPhrase; i++) {
                fwrite(tableDict->autoPhrase[i].strCode, tableDict->iCodeLength + 1, 1, fpDict);
                fwrite(tableDict->autoPhrase[i].strHZ, PHRASE_MAX_LENGTH * UTF8_MAX_LENGTH + 1, 1, fpDict);
                iTemp = tableDict->autoPhrase[i].iSelected;
                fwrite(&iTemp, sizeof(unsigned int), 1, fpDict);
            }
            fclose(fpDict);
        }
        free(pstr);

        strncpy(strPath, tableMetaData->uniqueName, PATH_MAX);
        strncat(strPath, "_LastAutoPhrase.tmp", PATH_MAX);
        fpDict = GetXDGFileWithPrefix("table", strPath, NULL, &pstr);
        if (access(pstr, F_OK))
            unlink(pstr);
        rename(strPathTemp, pstr);
        free(pstr);
    }
}

void FreeTableDict(TableMetaData* tableMetaData)
{
    RECORD         *recTemp, *recNext;
    short           i;
    TableDict      *tableDict = tableMetaData->tableDict;

    if (!tableDict->recordHead)
        return;

    if (tableDict->iTableChanged)
        SaveTableDict(tableMetaData);

    //释放码表
    recTemp = tableDict->recordHead->next;
    while (recTemp != tableDict->recordHead) {
        recNext = recTemp->next;

        free(recTemp->strCode);
        free(recTemp->strHZ);
        free(recTemp);

        recTemp = recNext;
    }

    free(tableDict->strIgnoreChars);
    tableDict->strIgnoreChars = NULL;

    free(tableDict->strInputCode);
    tableDict->strInputCode = NULL;

    free(tableDict->recordHead);
    tableDict->recordHead = NULL;

    if (tableDict->fh) {
        free(tableDict->fh);
        tableDict->fh = NULL;
        tableDict->iFH = 0;
    }

    tableDict->iRecordCount = 0;

    free(tableDict->strNewPhraseCode);

    //释放组词规则的空间
    if (tableDict->rule) {
        for (i = 0; i < tableDict->iCodeLength - 1; i++)
            free(tableDict->rule[i].rule);
        free(tableDict->rule);

        tableDict->rule = NULL;
    }

    //释放索引的空间
    if (tableDict->recordIndex) {
        free(tableDict->recordIndex);
        tableDict->recordIndex = NULL;
    }

    //释放自动词组的空间
    if (tableDict->autoPhrase) {
        for (i = 0; i < AUTO_PHRASE_COUNT; i++) {
            free(tableDict->autoPhrase[i].strCode);
            free(tableDict->autoPhrase[i].strHZ);
        }
        free(tableDict->autoPhrase);

        tableDict->autoPhrase = (AUTOPHRASE *) NULL;
    }

    free(tableDict);
    tableMetaData->tableDict = NULL;
}

/*
 *根据字串判断词库中是否有某个字/词，注意该函数会忽略拼音词组
 */
RECORD         *TableFindPhrase(const TableDict* tableDict, const char *strHZ)
{
    RECORD         *recTemp;
    char            strTemp[UTF8_MAX_LENGTH + 1];
    int             i;

    //首先，先查找第一个汉字的编码
    strncpy(strTemp, strHZ, utf8_char_len(strHZ));
    strTemp[utf8_char_len(strHZ)] = '\0';

    recTemp = tableDict->tableSingleHZ[CalHZIndex(strTemp)];
    if (!recTemp)
        return (RECORD *) NULL;

    //然后根据该编码找到检索的起始点
    i = 0;
    while (recTemp->strCode[0] != tableDict->recordIndex[i].cCode)
        i++;

    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead) {
        if (recTemp->strCode[0] != tableDict->recordIndex[i].cCode)
            break;
        if (!strcmp(recTemp->strHZ, strHZ)) {
            if (!recTemp->bPinyin)
                return recTemp;
        }

        recTemp = recTemp->next;
    }

    return (RECORD *) NULL;
}

void TableCreateAutoPhrase(TableMetaData* tableMetaData, char iCount)
{
    char            *strHZ;
    short           i, j, k;
    TableDict      *tableDict = tableMetaData->tableDict;

    if (!tableDict->autoPhrase)
        return;

    strHZ = (char *)fcitx_malloc0((tableMetaData->iAutoPhraseLength * UTF8_MAX_LENGTH + 1) * sizeof(char));
    /*
     * 为了提高效率，此处只重新生成新录入字构成的词组
     */
    j = tableDict->iHZLastInputCount - tableMetaData->iAutoPhraseLength - iCount;
    if (j < 0)
        j = 0;

    for (; j < tableDict->iHZLastInputCount - 1; j++) {
        for (i = tableMetaData->iAutoPhraseLength; i >= 2; i--) {
            if ((j + i - 1) > tableDict->iHZLastInputCount)
                continue;

            strcpy(strHZ, tableDict->hzLastInput[j].strHZ);
            for (k = 1; k < i; k++)
                strcat(strHZ, tableDict->hzLastInput[j + k].strHZ);

            //再去掉重复的词组
            for (k = 0; k < tableDict->iAutoPhrase; k++) {
                if (!strcmp(tableDict->autoPhrase[k].strHZ, strHZ))
                    goto _next;
            }
            //然后去掉系统中已经有的词组
            if (TableFindPhrase(tableDict, strHZ))
                goto _next;

            TableCreatePhraseCode(tableDict, strHZ);
            if (tableDict->iAutoPhrase != AUTO_PHRASE_COUNT) {
                strcpy(tableDict->autoPhrase[tableDict->iAutoPhrase].strCode, tableDict->strNewPhraseCode);
                strcpy(tableDict->autoPhrase[tableDict->iAutoPhrase].strHZ, strHZ);
                tableDict->autoPhrase[tableDict->iAutoPhrase].iSelected = 0;
                tableDict->iAutoPhrase++;
            } else {
                strcpy(tableDict->insertPoint->strCode, tableDict->strNewPhraseCode);
                strcpy(tableDict->insertPoint->strHZ, strHZ);
                tableDict->insertPoint->iSelected = 0;
                tableDict->insertPoint = tableDict->insertPoint->next;
            }

        _next:
            continue;
        }

    }

    free(strHZ);
}

boolean TableCreatePhraseCode(TableDict* tableDict, char *strHZ)
{
    unsigned char   i;
    unsigned char   i1, i2;
    size_t          iLen;
    char            strTemp[UTF8_MAX_LENGTH + 1] = {'\0', };
    RECORD         *recTemp;
    boolean bCanntFindCode = false;

    iLen = utf8_strlen(strHZ);
    if (iLen >= tableDict->iCodeLength) {
        i2 = tableDict->iCodeLength;
        i1 = 1;
    } else {
        i2 = iLen;
        i1 = 0;
    }

    for (i = 0; i < tableDict->iCodeLength - 1; i++) {
        if (tableDict->rule[i].iWords == i2 && tableDict->rule[i].iFlag == i1)
            break;
    }

    for (i1 = 0; i1 < tableDict->iCodeLength; i1++) {
        int clen;
        char* ps;
        if (tableDict->rule[i].rule[i1].iFlag) {
            ps = utf8_get_nth_char(strHZ, tableDict->rule[i].rule[i1].iWhich - 1);
            clen = utf8_char_len(ps);
            strncpy(strTemp, ps, clen);
        } else {
            ps = utf8_get_nth_char(strHZ, iLen - tableDict->rule[i].rule[i1].iWhich);
            clen = utf8_char_len(ps);
            strncpy(strTemp, ps, clen);
        }

        recTemp = tableDict->tableSingleHZ[CalHZIndex(strTemp)];

        if (!recTemp) {
            bCanntFindCode = true;
            break;
        }

        tableDict->strNewPhraseCode[i1] = recTemp->strCode[tableDict->rule[i].rule[i1].iIndex - 1];
    }

    return bCanntFindCode;
}

/*
 *判断某个词组是不是已经在词库中,有返回NULL，无返回插入点
 */
RECORD         *TableHasPhrase(const TableDict* tableDict, const char *strCode, const char *strHZ)
{
    RECORD         *recTemp;
    int             i;

    i = 0;
    while (strCode[0] != tableDict->recordIndex[i].cCode)
        i++;

    recTemp = tableDict->recordIndex[i].record;
    while (recTemp != tableDict->recordHead) {
        if (!recTemp->bPinyin) {
            if (strcmp(recTemp->strCode, strCode) > 0)
                break;
            else if (!strcmp(recTemp->strCode, strCode)) {
                if (!strcmp(recTemp->strHZ, strHZ))     //该词组已经在词库中
                    return NULL;
            }
        }
        recTemp = recTemp->next;
    }

    return recTemp;
}

void TableInsertPhrase(TableDict* tableDict, const char *strCode, const char *strHZ)
{
    RECORD         *insertPoint, *dictNew;

    insertPoint = TableHasPhrase(tableDict, strCode, strHZ);

    if (!insertPoint)
        return;

    dictNew = (RECORD *) fcitx_malloc0(sizeof(RECORD));
    dictNew->strCode = (char *) fcitx_malloc0(sizeof(char) * (tableDict->iCodeLength + 1));
    dictNew->bPinyin = 0;
    strcpy(dictNew->strCode, strCode);
    dictNew->strHZ = (char *) fcitx_malloc0(sizeof(char) * (strlen(strHZ) + 1));
    strcpy(dictNew->strHZ, strHZ);
    dictNew->iHit = 0;
    dictNew->iIndex = tableDict->iTableIndex;

    dictNew->prev = insertPoint->prev;
    insertPoint->prev->next = dictNew;
    insertPoint->prev = dictNew;
    dictNew->next = insertPoint;

    tableDict->iRecordCount++;
}

/*
 * 根据字串删除词组
 */
void TableDelPhraseByHZ(TableDict* tableDict, const char *strHZ)
{
    RECORD         *recTemp;

    recTemp = TableFindPhrase(tableDict, strHZ);
    if (recTemp)
        TableDelPhrase(tableDict, recTemp);
}

void TableDelPhrase(TableDict* tableDict, RECORD * record)
{
    record->prev->next = record->next;
    record->next->prev = record->prev;

    free(record->strCode);
    free(record->strHZ);
    free(record);

    tableDict->iRecordCount--;
}

void TableUpdateHitFrequency(TableDict* tableDict, RECORD * record)
{
    record->iHit++;
    record->iIndex = ++tableDict->iTableIndex;
}

int TableCompareCode(const TableMetaData* tableMetaData, const char *strUser, const char *strDict)
{
    int             i;

    for (i = 0; i < strlen(strUser); i++) {
        if (!strDict[i])
            return strUser[i];
        if (strUser[i] != tableMetaData->cMatchingKey || !tableMetaData->bUseMatchingKey) {
            if (strUser[i] != strDict[i])
                return (strUser[i] - strDict[i]);
        }
    }
    if (tableMetaData->bTableExactMatch) {
        if (strlen(strUser) != strlen(strDict))
            return -999;    //随意的一个值
    }

    return 0;
}

int TableFindFirstMatchCode(TableMetaData* tableMetaData, const char* strCodeInput)
{
    int             i = 0;
    TableDict      *tableDict = tableMetaData->tableDict;

    if (!tableDict->recordHead)
        return -1;

    if (tableMetaData->bUseMatchingKey && (strCodeInput[0] == tableMetaData->cMatchingKey))
        i = 0;
    else {
        while (strCodeInput[0] != tableDict->recordIndex[i].cCode) {
            if (!tableDict->recordIndex[i].cCode)
                break;
            i++;
        }
    }
    tableDict->currentRecord = tableDict->recordIndex[i].record;
    if (!tableDict->currentRecord)
        return -1;

    while (tableDict->currentRecord != tableDict->recordHead) {
        if (!TableCompareCode(tableMetaData, strCodeInput, tableDict->currentRecord->strCode)) {
            return i;
        }
        tableDict->currentRecord = tableDict->currentRecord->next;
        i++;
    }

    return -1;          //Not found
}

boolean IsInputKey(const TableDict* tableDict, int iKey)
{
    char           *p;

    p = tableDict->strInputCode;
    if (!p)
        return false;

    while (*p) {
        if (iKey == *p)
            return true;
        p++;
    }

    if (tableDict->bHasPinyin) {
        if (iKey >= 'a' && iKey <= 'z')
            return true;
    }

    return false;
}

boolean IsEndKey(const TableMetaData* tableMetaData, char cChar)
{
    char           *p;

    p = tableMetaData->strEndCode;
    if (!p)
        return false;

    while (*p) {
        if (cChar == *p)
            return true;
        p++;
    }

    return false;
}

boolean IsIgnoreChar(const TableDict* tableDict, char cChar)
{
    char           *p;

    p = tableDict->strIgnoreChars;
    while (*p) {
        if (cChar == *p)
            return true;
        p++;
    }

    return false;
}

#include "utf8_in_gb18030.h"

static int cmpi(const void * a, const void *b)
{
    return (*((int*)a)) - (*((int*)b));
}

unsigned int CalHZIndex(char *strHZ)
{
    unsigned int iutf = 0;
    int l = utf8_char_len(strHZ);
    unsigned char* utf = (unsigned char*) strHZ;
    unsigned int *res;
    int idx;

    if (l == 2) {
        iutf = *utf++ << 8;
        iutf |= *utf++;
    } else if (l == 3) {
        iutf = *utf++ << 16;
        iutf |= *utf++ << 8;
        iutf |= *utf++;
    } else if (l == 4) {
        iutf = *utf++ << 24;

        iutf |= *utf++ << 16;
        iutf |= *utf++ << 8;
        iutf |= *utf++;
    }

    res = bsearch(&iutf, utf8_in_gb18030, 63360, sizeof(int), cmpi);
    if (res)
        idx = res - utf8_in_gb18030;
    else
        idx = 63361;
    return idx;
}

boolean HasMatchingKey(const TableMetaData* tableMetaData, const char* strCodeInput)
{
    const char *str;

    str = strCodeInput;
    while (*str) {
        if (*str++ == tableMetaData->cMatchingKey)
            return true;
    }
    return false;
}
// kate: indent-mode cstyle; space-indent on; indent-width 0;
