/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <string.h>


void func_1();
void func_2();

int main(void)
{
/*	printk("Hello World! %s\n", CONFIG_BOARD);
	const char src[50] = "http://www.tutorialspoint.com";
   char dest[50];
   strcpy(dest,"Heloooo!!");
   printf("Before memcpy dest = %s\n", dest);
   memcpy(dest, src, strlen(src)+1);
   printf("After memcpy dest = %s\n", dest);
   *
   */
   printf("before main\n");

//	    simple_buffer("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
   printf("after main\n");
	overflow("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");  
   return(0);
}

/*int32_t main(int32_t argc, int8_t** argv) {
    uint32_t x = 0xA0A0A0A0;

    printf("Hello world %d\n", x);
    func_1();
    return 0;
}*/

void __attribute__ ((noinline)) func_1() {
    uint32_t x = 0xA1A1A1A1;
    void* stack_address = (void*)&x;
    void* frame_address = __builtin_frame_address(0);

    printf("------ func_1 ------\n");
    printf("stack_address = %p\n", stack_address);
    printf("frame_address = %p\n", frame_address);

    func_2();
}

void __attribute__ ((noinline)) func_2() {
    uint32_t x = 0xA2A2A2A2;
    void* stack_address = (void*)&x;
    void* frame_address = __builtin_frame_address(0);
    void* frame_address_func1 = __builtin_frame_address(1);

    printf("------ func_2 ------\n");
    printf("stack_address = %p\n", stack_address);
    printf("frame_address = %p\n", frame_address);
    printf("frame_address_func1 = %p\n", frame_address_func1);



    printf("------ dumping ------\n");
    for (uint32_t i = 0; i < 40; ++i) {
        uint32_t* ptr = (uint32_t*)(stack_address + sizeof(uint32_t) * i);
        printf("%p: %08x\n", ptr, *ptr);
    }
}




void simple_buffer(void *addr){
        printf("simple buffer\n");
	char dst[50];
        memcpy(addr, dst, 50);
}



void overflow(void *addr){
	char dst[5];
	printf("overflow function\n");
	printf("__builtin_frame_address; %x\n", __builtin_frame_address(0));
    	memcpy(addr, dst, 50);
}	
