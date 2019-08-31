#pragma once

#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>

#include <stdint.h>
#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "pdb.h"
#include "pdb_error.h"

#define PDB_SYS_DEV_CNT   100000

#define IP_ADDR_STR_LEN   16      //IP��ַ�ַ������ȣ�Ŀǰֻ����IPv4


#define PDB_KB_BYTES(k)     ((k) * 1024)
#define PDB_MB_BYTES(m)     ((size_t)(m) * 1024 * 1024)
#define PDB_GB_BYTES(g)     ((size_t)(g) * 1024 * 1024 * 1024)

#define PDB_PART_TYPE_NORMAL_STR    "normal"
#define PDB_PART_TYPE_COMPRESS_STR  "compress"
#define PDB_PART_TYPE_NORMAL_VAL    1
#define PDB_PART_TYPE_COMPRESS_VAL  2

#define NORMAL_DATA_FILE_HEAD_STR   "PDB NORMAL 1"
#define COMPRESS_DATA_FILE_HEAD_STR "PDB COMPRESS 1"

#define PDB_MAJOR_VER_VAL        1
#define PDB_MINOR_VER_VAL        3
#define PDB_BUILD_VER_VAL        0

#define PDB_BOOL_FALSE           0
#define PDB_BOOL_TRUE            1

#define NORMAL_PAGE_SIZE       (64 * 1024)
#define SYNC_PAGE_CNT          64

#define MAKE_DATAPART_MASK(tabCode, partCode)  ((((uint64_t)tabCode & 0xFF) << 56) | (((uint64_t)partCode & 0xFFFFFF) << 32))
#define MAKE_DATATABLE_MASK(tabCode)           (((uint64_t)tabCode & 0xFF) << 56)


#define DEVID_FIELD_NAME            "devid"
#define TSTAMP_FIELD_NAME           "tstamp"

#define DOUBLE_PRECISION         ((double)0.0000000001)

#define NORMAL_IDX_FILE_EXTEND               ".idx"       // ͨ�������ļ���չ��
#define NORMAL_DATA_FILE_EXTEND              ".dat"       // ͨ�������ļ���չ��
#define COMPRESS_DATA_FILE_EXTEND            ".cdat"      //


#define PDB_USER_ROLE_READONLY_STR    "readOnly"
#define PDB_USER_ROLE_WRITEONLY_STR   "writeOnly"
#define PDB_USER_ROLE_READWRITE_STR   "readWrite"
#define PDB_USER_ROLE_ADMIN_STR       "admin"


///////////////////////////////�����ļ���غ����////////////////////////


#define PDB_PWD_CRC32_LEN      4  //�����CRC�볤��
#define PDB_ROLE_LEN           4  //��ɫ�ĳ���

#define PDB_DEVID_INDEX         0        // ���������±�
#define PDB_TSTAMP_INDEX        1        // ʱ������±�


#define PDB_QUERY_DEFAULT_COUNT            1000
#define PDB_QUERY_MAX_COUNT                10000

#define PDB_TABLE_MAX_FIELD_COUNT          (860)         // һ������������

#define PDB_MAX_PACKET_BODY_LEN      (4 * 1024 * 1024)  //ÿ��������󳤶�
#define PDB_MAX_PACKET_REC_CNT       (1000)             //һ�������������ļ�¼����

//ÿ����¼� 8K
#define PDB_MAX_REC_LEN              8192

typedef struct _PdbStr
{
  const char* pStr_;
  size_t len_;
}PdbStr;

typedef struct _PdbBlob
{
  const uint8_t* pBlob_;
  size_t len_;
}PdbBlob;

typedef uint8_t PdbByte;

typedef struct _FieldInfoFormat
{
  char fieldName_[PDB_FILED_NAME_LEN];   //�ֶ���
  int32_t fieldType_;   //�ֶ�����
  char padding_[12];
}FieldInfoFormat;
