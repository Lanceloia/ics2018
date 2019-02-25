#include "nemu.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
		Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
		guest_to_host(addr); \
		})

uint8_t pmem[PMEM_SIZE];

//Lanceloia
unsigned long int get_PMEM_SIZE(){
	return (unsigned long int)PMEM_SIZE;
}

uint8_t* get_pmem(){
	return pmem;
}

/* Memory accessing interfaces */

extern int is_mmio(paddr_t addr);
extern uint32_t mmio_read(paddr_t addr, int len, int map_No);
extern void mmio_write(paddr_t addr, int len, uint32_t data, int map_No);

uint32_t paddr_read(paddr_t addr, int len) {
	int map_No = is_mmio(addr);
	if(map_No == -1)
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
	else
		return mmio_read(addr, len, map_No);
	/* Copy */
	//return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
	/* Copy */
}

void paddr_write(paddr_t addr, uint32_t data, int len) {
	int map_No = is_mmio(addr);
	if(map_No == -1)
		memcpy(guest_to_host(addr), &data, len);
	else
		mmio_write(addr, len, data, map_No);
}

paddr_t page_translate(vaddr_t addr){

	paddr_t ret_addr = addr;

	if(cpu.CR0.PG){
		uint32_t DIR_offset = ((addr >> 22) & 0x000003ff) << 2;
		uint32_t PAGE_offset = ((addr >> 12) & 0x000003ff) << 2;
		uint32_t DATA_offset = ((addr >> 0) & 0x00000fff);
		
		//DIR_base: ye mu lu biao shou di zhi
		uint32_t DIR_base = cpu.CR3.as_32bits_reg & 0xfffff000;
		
		//PAGE_base: ye mu lu xiang  //Di Yi Ji Ye Biao
		uint32_t PAGE_base = paddr_read((DIR_base & 0xfffff000) + DIR_offset, 4);
		
		if(!(PAGE_base & 0x1)) {// check this is odd
			printf("\nvaddr: 0x%08x\n", addr);
			printf("eip: 0x%08x\n", cpu.eip);
			assert(0);
		}
		
		//DATA_base Ye Kuang Ji Di Zhi  //Di Er Ji Ye Biao
		uint32_t DATA_base = paddr_read((PAGE_base & 0xfffff000) + PAGE_offset, 4);
		
		if(!(DATA_base & 0x1)) {// check this is odd
			printf("\nvaddr: 0x%08x\n", addr);
			printf("eip: 0x%08x\n", cpu.eip);
			assert(0);
		}
		
		ret_addr = (DATA_base&0xfffff000) + DATA_offset;
	}

	return ret_addr;
}

uint32_t vaddr_read(vaddr_t addr, int len) {
	int left = (0x1000 - (addr & 0xfff));
	int data_1, data_2;
	if(left < len){
		data_1 = paddr_read(page_translate(addr), left);
		data_2 = paddr_read(page_translate(addr+left), (len - left));
		//printf("\ndata_1 %x, data_2 %x \n", data_1, data_2);
		//assert(0);
		return (data_2 << (left * 8)) + data_1;
	}
	return paddr_read(page_translate(addr), len);
}

void vaddr_write(vaddr_t addr, uint32_t data, int len) {
	int left = (0x1000 - (addr & 0xfff));
	assert(left >= len);
	paddr_write(page_translate(addr), data, len);
}
