#include "xfs/tools/test_mempool/hash_table_mempool.h"
#include <stdio.h>

struct tag_hash_table_test_struct 
{
	uint64_t key;
	uint64_t GetKey() 
	{ 
		return key;
	}
};

int main(int argc,char** argv)
{
	AutoBaseLib auto_base_lib;

	srand(time(NULL)+1000000);
	
	HashTable<tag_hash_table_test_struct> ht;
	ht.Init(10000003);


	struct timeval tv1,tv2;

	uint32_t max_test_num = 2000 * 10000;

	gettimeofday(&tv1,NULL);
	for(uint32_t i=0;i<max_test_num;i++) 
	{
		tag_hash_table_test_struct* t = mempool_NEW(tag_hash_table_test_struct) ;   
//	    tag_hash_table_test_struct* t = new tag_hash_table_test_struct;
		t->key = rand();		
		ht.Add(t); 
	}
	gettimeofday(&tv2,NULL);
	printf("Insert: rand_num=%d cost_time=%u  insert_success=%u\n", max_test_num,
			uint32_t(tv2.tv_sec * 1000 + tv2.tv_usec/1000 - tv1.tv_sec * 1000 - tv1.tv_usec/1000), 
			ht.GetValidCount());

	uint64_t key=0;
	uint32_t count=0;
	gettimeofday(&tv1,NULL);
	for(uint32_t i=0;i<max_test_num;i++)
	{
		key = rand();
		if( ht.Find(key))
		{
			count++;
		}
	}
	gettimeofday(&tv2,NULL);
	printf("Query:  hit_count=%d cost_time=%d \n", count,
			uint32_t(tv2.tv_sec * 1000 + tv2.tv_usec/1000 - tv1.tv_sec * 1000 - tv1.tv_usec/1000));

	printf("mempool_GetMemPoolSize=%d Max_List_Len=%d\n",mempool_GetMemPoolSize(),ht.GetMaxListLen());

	return 0;

}




