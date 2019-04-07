/* radio.c 2019 04 06 END */
/* referance: https://www.raspberrypi.org/forums/viewtopic.php?t=53680 */
/* By hxdyxd */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>


#define I2C_DEV       "/dev/i2c-1"
#define I2C_TEA5767_ID   0x60


unsigned char *radio_freq_set(unsigned char radio[5], double frequency)
{
    unsigned char frequencyH = 0;
    unsigned char frequencyL = 0;

    unsigned int frequencyB;
    //double frequency = strtod(argv[1], NULL);

    frequencyB = 4*(frequency*1000000+225000)/32768; //calculating PLL word
    frequencyH = frequencyB >> 8;
    frequencyL = frequencyB & 0XFF;

    // data to be sent
    radio[0]=frequencyH; //FREQUENCY H
    radio[1]=frequencyL; //FREQUENCY L
    radio[2]=0xB0; //3 byte (0xB0): high side LO injection is on,.
    radio[3]=0x10; //4 byte (0x10) : Xtal is 32.768 kHz
    radio[4]=0x00; //5 byte0x00)
    return radio;
}


int main( int argc, char *argv[])
{
    static unsigned char radio[5] = {0};

    printf("RPi - tea5767 Philips FM Tuner v0.4 \n");
    if(argc < 2) {
        printf("error parameters \r\n");
        return -1;
    }

    int fd;
    //open i2c device
    if ((fd = open(I2C_DEV, O_RDWR)) < 0) {
        printf("error opening i2c dev %s \r\n", I2C_DEV);
        return -1;
    }

    //set slave id
    if (ioctl (fd, I2C_SLAVE, I2C_TEA5767_ID) < 0) {
        printf("error set i2c slave id %02x \r\n", I2C_TEA5767_ID);
        return -1;
    }

    double frequency = 87.5;
    if(argv[1][0] == 's') {
        //search
        for(double i = 87.5; i <= 108; i += 0.1) {
            write(fd, radio_freq_set(radio, i), 5);
            usleep(50000);
            //read value
            int rlen = read(fd, radio, 5);
            if(rlen != 5) {
                printf("error read i2c \r\n");
                break;
            }
            unsigned char level = radio[3] >> 4;
            if(level > 5) {
                printf("%.1f level is %d, available \r\n", i, level);
                frequency = i;
            }
        }
    } else {
        frequency = strtod(argv[1], NULL);
    }

    //set frequency
    printf("Frequency = ");
    printf("%f", frequency);
    printf("\n");

    write(fd, radio_freq_set(radio, frequency), 5);
    usleep(50000);

    int rlen = read(fd, radio, 5);
    if(rlen == 5) {
        for(int i=0; i<rlen; i++) {
            printf("%02x ", radio[i]);
        }
        printf("\r\n");
    }

    close(fd);

    return 0;
}
