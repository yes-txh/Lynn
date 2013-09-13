// TestSvrpublib.cpp : Defines the entry point for the console application.
//

#include "common/baselib/svrpublib/twse_type_def.h"
#include "common/baselib/svrpublib/server_publib.h"
#include <string>
using std::string;
// using namespace google;
DECLARE_USING_LOG_LEVEL_NAMESPACE;

using namespace xfs::base;

// 8-bit table
static unsigned char crc8_table[256] = {
    0  , 7  , 14 , 9  , 28 , 27 , 18 , 21 ,
    56 , 63 , 54 , 49 , 36 , 35 , 42 , 45 ,
    112, 119, 126, 121, 108, 107, 98 , 101,
    72 , 79 , 70 , 65 , 84 , 83 , 90 , 93 ,
    224, 231, 238, 233, 252, 251, 242, 245,
    216, 223, 214, 209, 196, 195, 202, 205,
    144, 151, 158, 153, 140, 139, 130, 133,
    168, 175, 166, 161, 180, 179, 186, 189,
    199, 192, 201, 206, 219, 220, 213, 210,
    255, 248, 241, 246, 227, 228, 237, 234,
    183, 176, 185, 190, 171, 172, 165, 162,
    143, 136, 129, 134, 147, 148, 157, 154,
    39 , 32 , 41 , 46 , 59 , 60 , 53 , 50 ,
    31 , 24 , 17 , 22 , 3  , 4  , 13 , 10 ,
    87 , 80 , 89 , 94 , 75 , 76 , 69 , 66 ,
    111, 104, 97 , 102, 115, 116, 125, 122,
    137, 142, 135, 128, 149, 146, 155, 156,
    177, 182, 191, 184, 173, 170, 163, 164,
    249, 254, 247, 240, 229, 226, 235, 236,
    193, 198, 207, 200, 221, 218, 211, 212,
    105, 110, 103, 96 , 117, 114, 123, 124,
    81 , 86 , 95 , 88 , 77 , 74 , 67 , 68 ,
    25 , 30 , 23 , 16 , 5  , 2  , 11 , 12 ,
    33 , 38 , 47 , 40 , 61 , 58 , 51 , 52 ,
    78 , 73 , 64 , 71 , 82 , 85 , 92 , 91 ,
    118, 113, 120, 127, 106, 109, 100, 99 ,
    62 , 57 , 48 , 55 , 34 , 37 , 44 , 43 ,
    6  , 1  , 8  , 15 , 26 , 29 , 20 , 19 ,
    174, 169, 160, 167, 178, 181, 188, 187,
    150, 145, 152, 159, 138, 141, 132, 131,
    222, 217, 208, 215, 194, 197, 204, 203,
    230, 225, 232, 239, 250, 253, 244, 243
};

// #define GP  0x107   // x^8 + x^2 + x + 1 
// #define DI  0x07
// static int made_table=0;
// 
// static void init_crc8()
// // Should be called before any other crc function.  
// {
//    int i,j;
//    unsigned char crc;
//
//    if (!made_table) {
//        for (i=0; i<256; i++) {
//            crc = i;
//            for (j=0; j<8; j++)
//                crc = (crc << 1) ^ ((crc & 0x80) ? DI : 0);
//            crc8_table[i] = crc & 0xFF;            
//            printf("%-3d, ", crc8_table[i]);
//            if((i + 1) % 8 == 0)
//                printf("\r\n");
//        }
//        made_table=1;
//    }
// }


// For a byte array whose accumulated crc value is stored in *crc, computes
// resultant crc obtained by appending m to the byte array
inline void CRC8(unsigned char *crc, unsigned char m) {
    *crc = crc8_table[(*crc) ^ m];
    //*crc &= 0xFF;
}

uint32_t FastCRC1(uint32_t old_crc, unsigned char* dat, uint32_t len) {
    uint64_t crc = 0;
    unsigned char* p = (unsigned char*)&crc;
    unsigned char* c0 = &p[0];
    unsigned char* c1 = &p[1];
    unsigned char* c2 = &p[2];
#if 0
	unsigned char* c3 = &p[3];    
	unsigned char* c4 = &p[4];    
	unsigned char* c5 = &p[5];    
	unsigned char* c6 = &p[6];    
	unsigned char* c7 = &p[7];    
#endif
    // 假设是长度是3的倍数
    uint32_t count = len / 3;
	unsigned char* d = dat;
    for (uint32_t u = 0; u < count; ++u) {
        //d = dat + u * 3;
		
        *c0 = crc8_table[(*c0) ^ d[0]];
        *c1 = crc8_table[(*c1) ^ d[1]];
        *c2 = crc8_table[(*c2) ^ d[2]];
        
		d +=3;
    }

    return (int32_t)crc;
}

uint32_t CRC32_use_crc8(unsigned char* dat, uint32_t len) {
    uint32_t crc = 0;
    uint8_t crc_val  =0 ;
    for (uint32_t u = 0; u < len; u++)
        CRC8(&crc_val, dat[u]);

    crc = crc_val;
    return crc;
}

void test_crc8_speed() {
    uint32_t buff_len = 900*1024*1024;
    unsigned char* buff = new unsigned char[buff_len];
    memset(buff, 0xfa, buff_len);

    struct timeval t0 = {0};
    struct timeval t1 = {0};
    lite_gettimeofday(&t0, NULL);

    LOG(INFO) << "calc crc8 start";

    uint32_t hash = CRC32_use_crc8(buff, buff_len);

    LOG(INFO) << "calc crc8 finished.";

    lite_gettimeofday(&t1, NULL);

    uint32_t cost = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
    float f_cost = float(cost) / float(1000000);
    float speed = float(buff_len) / float(1024*1024) / f_cost;

    LOG(INFO) << "crc8 speed is " << speed <<" MB/s, cost:" << f_cost
        <<" seconds, cost:" << cost << " microseconds.\r\n";
      
    LOG(INFO) << "hash result, hash = " << hash;

    delete []buff;
    buff = NULL;
}

void test_fast_crc8_speed() {
    uint32_t buff_len = 900*1024*1024;
    unsigned char* buff = new unsigned char[buff_len];
    memset(buff, 0xfa, buff_len);

    struct timeval t0 = {0};
    struct timeval t1 = {0};
    lite_gettimeofday(&t0, NULL);

    LOG(INFO) << "calc fast crc8 start";

    uint32_t hash = FastCRC1(0, buff, buff_len);

    LOG(INFO) << "calc fast crc8 finished.";

    lite_gettimeofday(&t1, NULL);

    uint32_t cost = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
    float f_cost = float(cost) / float(1000000);
    float speed = float(buff_len)/float(1024*1024) / f_cost;

    LOG(INFO) << "fast crc8 speed is"<< speed << " MB/s, cost:" << f_cost
        <<" seconds, cost:" << cost << " microseconds.";
        
    LOG(INFO) << "hash result, hash = " << hash;

    delete []buff;
    buff = NULL;
}

void test_crc32_speed() {
    uint32_t buff_len = 900*1024*1024;
    unsigned char* buff = new unsigned char[buff_len];
    memset(buff, 0xfa, buff_len);

    struct timeval t0 = {0};
    struct timeval t1 = {0};
    lite_gettimeofday(&t0, NULL);

    LOG(INFO) << "calc crc32 start";

    uint32_t hash = FastCRC(0xffffffff, buff, buff_len);

    LOG(INFO) << "calc crc32 finished.";

    lite_gettimeofday(&t1, NULL);

    uint32_t cost = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
    float f_cost = float(cost) / float(1000000);
    float speed = float(buff_len)/float(1024*1024) / f_cost;

    LOG(INFO) << "fast crc speed is " << speed << " MB/s, cost:" << f_cost
        <<" seconds, cost:" << cost << " microseconds.";
    LOG(INFO) << "hash result, hash = " << hash;

    delete []buff;
    buff = NULL;
}

void test_hash_speed() {
    uint32_t buff_len = 800*1024*1024;
    unsigned char* buff = new unsigned char[buff_len];
    memset(buff, 0xfa, buff_len);

    struct timeval t0 = {0};
    struct timeval t1 = {0};
    lite_gettimeofday(&t0, NULL);

    LOG(INFO) << "calc hash start";

    uint32_t hash = GetCRC32(0xffffffff, buff, buff_len);

    LOG(INFO) << "calc hash finished.";

    lite_gettimeofday(&t1, NULL);

    uint32_t cost = (t1.tv_sec - t0.tv_sec) * 1000000 + t1.tv_usec - t0.tv_usec;
    float f_cost = float(cost) / float(1000000);
    float speed = float(buff_len)/float(1024*1024) / f_cost;

    LOG(INFO) << "crc32 speed is " << speed << " MB/s, cost:" << f_cost
        <<" seconds, cost:" << cost << " microseconds.";
    LOG(INFO) << "hash result, hash = " << hash;

    delete []buff;
    buff = NULL;
}

int main(int argc, char** argv) {
    
    AutoBaseLib auto_baselib;
   
    google::AllowCommandLineReparsing();
    google::ParseCommandLineFlags(&argc, &argv, true);

    test_crc32_speed();

    test_crc8_speed();

    test_fast_crc8_speed();

    test_hash_speed();

    string t1 = "this is string 11";
    string t2 = "this is string 2";
    string t3 = t1 + t2;
    
	uint32_t crc1 = FastCRC(0xFFFFFFFF,(unsigned char*)t3.c_str(),t3.size()); 
	
    uint32_t crc2 = FastCRC(0xFFFFFFFF,(unsigned char*)t1.c_str(),t1.size());
	
    crc2 = FastCRC(crc2,(unsigned char*)t2.c_str(),t2.size()); 
	
    if (crc1 == crc2)
        LOG(INFO) << "*** crc is right crc =" << crc1;
    else
        LOG(ERROR) << "*** crc is wrong";

	uint32_t buff_len = 1204 * 1024 * 561;
    unsigned char* p = new unsigned char[buff_len];
    unsigned char* buff = p;
	for (uint32_t i = 0; i < buff_len;) {
		p[i] = rand() % 231;
		i += rand() % 6271;
	}
	uint32_t crc3 = FastCRC(0xFFFFFFFF, buff, buff_len);
    uint32_t crc4;
	uint32_t len = rand() % 10243;
	crc4 = FastCRC(0xFFFFFFFF,buff,len);
	buff_len -= len;
	buff += len;
	while (buff_len > 0) {
		len = rand() % 10247;
		if(buff_len < len)
			len = buff_len;
		crc4 = FastCRC(crc4, buff, len);
		buff_len -= len;
		buff += len;
	}
	
	delete []p;

	if (crc3 == crc4)
		LOG(INFO) << "*** crc is right crc =" << crc3;
	else
		LOG(ERROR) << "*** omg, crc is wrong";
	return 0;
}

