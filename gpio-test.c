#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>


#define AR71XX_APB_BASE         0x18000000
#define AR71XX_GPIO_BASE        (AR71XX_APB_BASE + 0x00040000)

#define AR71XX_GPIO_REG_OE		0
#define AR71XX_GPIO_REG_IN 		1 
#define AR71XX_GPIO_REG_OUT		2
#define AR71XX_GPIO_REG_SET		3
#define AR71XX_GPIO_REG_CLEAR	4

#define AR71XX_GPIO_REG_INT_MODE	0x14
#define AR71XX_GPIO_REG_INT_TYPE	0x18
#define AR71XX_GPIO_REG_INT_POLARITY	0x1c
#define AR71XX_GPIO_REG_INT_PENDING	0x20
#define AR71XX_GPIO_REG_INT_ENABLE	0x24
#define AR71XX_GPIO_REG_FUNC		0x28

#define AR71XX_GPIO_COUNT	16


int  mem_fd     = 0;
char *gpio_mmap = NULL;
char *gpio_ram  = NULL;
volatile unsigned int *gpio = NULL;

#define PAGE_SIZE   4096        
#define BLOCK_SIZE  PAGE_SIZE


static void
memread_memory(void *addr, int len, int iosize)
{
	int i;
	i = 0;
	
		while (i < 16 && len) {
			printf(" [*] %i, %08lx",i, *(unsigned long *)addr);
			i += iosize;
			addr += iosize;
			len -= iosize;
		}
	
	printf("\n");
}

void gpio_setpin(unsigned int pin, unsigned int state)
{

	if (state)
	{
		*(gpio+AR71XX_GPIO_REG_OUT)  |= (1 << pin);
		//*(gpio+AR71XX_GPIO_REG_IN)  |= (1 << pin);
	}else{
		*(gpio+AR71XX_GPIO_REG_OUT)  &=~(1 << pin);
		//*(gpio+AR71XX_GPIO_REG_IN)  &=~(1 << pin);
	}
}


unsigned int gpio_getpin(unsigned int pin)
{
    return ((( *(gpio+AR71XX_GPIO_REG_IN)  & (1 << pin)) != 0) ? 1 : 0);
}

void gpio_setpindir(unsigned int pin, unsigned int state)
{
	if (state)
	{
		*(gpio+AR71XX_GPIO_REG_OE)  |= (1 << pin);
	}else{
		*(gpio+AR71XX_GPIO_REG_OE)  &= ~(1 << pin);
	}
}


void dump_register()
{

	
	printf(" [d] AR71XX_GPIO_REG_OE  %08lx \n", *(unsigned long *)gpio);
   	printf(" [d] AR71XX_GPIO_REG_IN  %08lx \n", *(unsigned long *)(gpio+1));
   	printf(" [d] AR71XX_GPIO_REG_OUT %08lx \n", *(unsigned long *)(gpio+2));
	printf(" [d] AR71XX_GPIO_REG_SET %08lx \n", *(unsigned long *)(gpio+3));
	printf(" [d] AR71XX_GPIO_REG_CLEAR %08lx \n", *(unsigned long *)(gpio+3));
	
}

int main(int argc, char *argv[]) {
	
	int j;
   /* open /dev/mem to get acess to physical ram */
   if ((mem_fd = open("/dev/mem", O_RDWR) ) < 0) {
      printf("can't open /dev/mem. Did you run the program with administrator rights?\n");
      exit (-1);
   }
      /* Allocate a block of virtual RAM in our application's address space */
   if ((gpio_ram = malloc(BLOCK_SIZE + (PAGE_SIZE-1))) == NULL) {
      printf("allocation error \n");
      exit (-1);
   }

   if ((unsigned long)gpio_ram % PAGE_SIZE)
   gpio_ram += PAGE_SIZE - ((unsigned long)gpio_ram % PAGE_SIZE);


   gpio_mmap = (unsigned char *)mmap(
      (caddr_t)gpio_ram,
      BLOCK_SIZE,
      PROT_READ|PROT_WRITE,
      MAP_SHARED,
      mem_fd,
      AR71XX_GPIO_BASE
   );

   	//memread_memory(gpio_mmap, 16, 4);
   if ((long)gpio_mmap < 0) {
      printf("unable to map the memory. Did you run the program with administrator rights?\n");
      exit (-1);
   }
   /* Always use a volatile pointer to hardware registers */
   gpio = (volatile unsigned *)gpio_mmap;	
   dump_register();

getchar();

//save current state
int m_register = *(gpio+AR71XX_GPIO_REG_OE);

printf("%i\n",argc );
if (argc == 2)
{
	
	int i_pin = strtol(argv[1], NULL,10);
	printf("[i] gpio in using pin %i\n",  i_pin);

	gpio_setpindir(i_pin,0);
	gpio_setpin(i_pin,0);

		
	while(1)
	{
		//printf("wwew\n");
		printf("[i] gpio input pin: %i value: %i\n", i_pin, gpio_getpin(i_pin) );
		usleep(100000);	
	}

}else if ( argc == 3){
	for (j = 0; j<32; j++) {
		printf("%i -> %x\n",j, 1 << j);

		//gpio_setpindir();


		*(gpio+AR71XX_GPIO_REG_OE) = (1 << j);
		getchar();
	}
}else{

	int o_pin = strtol(argv[3], NULL,10);

	printf("[i] gpio out using pin %i\n",  o_pin);

	gpio_setpindir(o_pin,1);
	gpio_setpin(o_pin,0);

	while(1)
	{
		printf("[d] OFF\n");
		gpio_setpin(o_pin,0);
		usleep(1000000);	
		printf("[d] ON\n");
		gpio_setpin(o_pin,1);
		usleep(1000000);	
			
	}
}

//restore to previous state
*(gpio+AR71XX_GPIO_REG_OE) = m_register;

  	printf("clean up...\n");
	close (mem_fd);
	free(gpio_ram);
}

