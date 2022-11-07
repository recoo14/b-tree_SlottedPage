#include "page.hpp"
#include <iostream> 
#include <cstring> 

void put2byte(void *dest, uint16_t data){
	*(uint16_t*)dest = data;
}

void put8byte(void* dest, uint64_t data) {
	*(uint64_t*)dest = data;
}

uint16_t get2byte(void *dest){
	return *(uint16_t*)dest;
}

page::page(uint16_t type){
	hdr.set_num_data(0);
	hdr.set_data_region_off(PAGE_SIZE-1-sizeof(page*));
	hdr.set_offset_array((void*)((uint64_t)this+sizeof(slot_header)));
	hdr.set_page_type(type);
}

uint16_t page::get_type(){
	return hdr.get_page_type();
}

uint16_t page::get_record_size(void *record){
	uint16_t size = *(uint16_t *)record;
	return size;
}

char *page::get_key(void *record){
	char *key = (char *)((uint64_t)record+sizeof(uint16_t));
	return key;
}

uint64_t page::get_val(void *key){
	uint64_t val= *(uint64_t*)((uint64_t)key+(uint64_t)strlen((char*)key)+1);
	return val;
}

void page::set_leftmost_ptr(page *p){
	leftmost_ptr = p;
}

page *page::get_leftmost_ptr(){
	return leftmost_ptr;	
}

uint64_t page::find(char *key){
	// Please implement this function in project 1.
	uint32_t num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();

	uint16_t off = 0;
	uint64_t stored_val = 0;
	uint64_t val = 0;

	void* stored_key = nullptr;
	void* data_region = nullptr;
	printf("%d\n", num_data);
	for (int i = 0; i < num_data; i++) {
		off = *(uint16_t*)((uint64_t)offset_array + i * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		stored_key = get_key(data_region);
		stored_val = get_val((void*)stored_key);
		char compared_key = strcmp((char*)stored_key, key);

		printf("%s\n", stored_key);

		if (compared_key == '\0') {
			val = stored_val;
			break;
		}
	}
	return val;
	
}

bool page::insert(char *key,uint64_t val){
	// Please implement this function in project 1.

	int num_data = hdr.get_num_data();
	void* offset_array = hdr.get_offset_array();
	uint16_t now_data_region_off = hdr.get_data_region_off();
	void* data_region = 0;
	uint16_t off = 0;
	
	uint16_t input_record_size = (uint16_t)strlen(key) + 1 + sizeof(uint64_t)+ sizeof(uint16_t);

	uint16_t remain_data_region_off = now_data_region_off - input_record_size;
	


	if (is_full(input_record_size)) {
		printf("꽉찼어요\n");
		printf("%d\n", num_data);
		hdr.set_data_region_off(remain_data_region_off);
		off = *(uint16_t*)((uint64_t)offset_array + num_data * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		*((uint8_t*)((uint64_t)data_region + sizeof(uint16_t))) = '\n';

		put2byte(data_region, 0);
	}
	else { // record 넣기( size(2) key(널 포함) value(8))
		//offset array 넣기
		put2byte((uint16_t*)((uint64_t)offset_array + num_data * 2), remain_data_region_off);
		hdr.set_data_region_off(remain_data_region_off);

		off = *(uint16_t*)((uint64_t)offset_array + num_data * 2);
		data_region = (void*)((uint64_t)this + (uint64_t)off);
		//size 넣기
		put2byte(data_region, input_record_size);

		//key 넣기
		int k = 0;
		while (*key != '\0')
		{
			*((uint8_t*)((uint64_t)data_region + sizeof(uint16_t))+k) = *key;
			k++;
			key++;
		}
		*((uint8_t*)((uint64_t)data_region + sizeof(uint16_t)) + k) = '\0';

		k++;

		//value 넣기
		put8byte((uint64_t*)((uint64_t)data_region + sizeof(uint16_t) + k), val);

	}
	num_data++;
	hdr.set_num_data(num_data);
	return true;
}

/*page* page::split(char* key, uint64_t val, char** parent_key) {
	// Please implement this function in project 2.
	page *new_page;
	return new_page;
}*/

bool page::is_full(uint64_t inserted_record_size){
	// Please implement this function in project 1.
	// 저장 가능한 사이즈 = data_region off - offset_array() = 남아있는 주소
	
	uint32_t data_region_size = hdr.get_data_region_off();
	uint32_t num_data = hdr.get_num_data();
	//uint16_t last_index = (num_data - 1) * 2;

	printf("( %d, %d )\n", data_region_size, inserted_record_size);

	if (data_region_size < inserted_record_size + 10) return true;
	else return false;
}

void page::defrag(){
	page *new_page = new page(get_type());
	int num_data = hdr.get_num_data();
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint16_t off=0;
	uint64_t stored_val=0;
	void *data_region=nullptr;

	for(int i=0; i<num_data/2; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		new_page->insert((char*)stored_key,stored_val);
	}	
	new_page->set_leftmost_ptr(get_leftmost_ptr());

	memcpy(this, new_page, sizeof(page));
	delete new_page;

}

void page::print(){
	uint32_t num_data = hdr.get_num_data();
	uint16_t off=0;
	uint16_t record_size= 0;
	void *offset_array=hdr.get_offset_array();
	void *stored_key=nullptr;
	uint64_t stored_val=0;

	printf("## slot header\n");
	printf("Number of data :%d\n",num_data);
	printf("offset_array : |");
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		printf(" %d |",off);
	}
	printf("\n");

	void *data_region=nullptr;
	for(int i=0; i<num_data; i++){
		off= *(uint16_t *)((uint64_t)offset_array+i*2);	
		data_region = (void *)((uint64_t)this+(uint64_t)off); //key를 가르키는 주소
		record_size = get_record_size(data_region);
		stored_key = get_key(data_region);
		stored_val= get_val((void *)stored_key);
		printf("==========================================================\n");
		printf("| data_sz:%u | key: %s | val :%lu |\n",record_size,(char*)stored_key, stored_val,strlen((char*)stored_key));

	}
}




