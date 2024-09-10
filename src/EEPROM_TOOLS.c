/* ftdi-eeprom.c
 * Copyright 2016 Evan Nemerson <evan@nemerson.com>
 * Licensed under the GNU GPLv3; see COPYING for details.
 **/

#include <getopt.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include "ftd2xx.h"

int dump_out(char *file_name, FT_HANDLE ftHandle, uint16_t offset, uint16_t size, uint8_t auto_checksum);
int dump_in(char *file_name, FT_HANDLE ftHandle, uint16_t offset, uint16_t size, uint8_t auto_checksum);
int main_process(uint8_t read, uint8_t write, char *output_file_name, char *input_file_name);

static void print_help(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    int c;

    static struct option long_options[] = {
        {"read", 0, 0, 'r'},
        {"write", 0, 0, 'w'},
        {"output_file", 1, 0, 'o'},
        {"input_file", 1, 0, 'i'},
        {"help", 0, 0, 'h'},
        {NULL, 0, NULL, 0}};

    uint8_t c_help = 0;
    uint8_t c_read = 0;
    uint8_t c_write = 0;
    uint8_t c_output_file_named = 0;
    uint8_t c_input_file_named = 0;
    char c_output_file_name[128];
    char c_input_file_name[128];
    memset(c_output_file_name, 0, sizeof(c_output_file_name));
    memset(c_input_file_name, 0, sizeof(c_input_file_name));

    while ((c = getopt_long(argc, argv, "ro:wi:h", long_options, &optind)) != -1)
    {
        switch (c)
        {
        case 'r':
            c_read = 1U;
            break;
        case 'w':
            c_write = 1U;
            break;
        case 'o':
            sprintf_s(c_output_file_name, sizeof(c_output_file_name), "%s", optarg);
            c_output_file_named = 1U;
            break;
        case 'i':
            sprintf_s(c_input_file_name, sizeof(c_input_file_name), "%s", optarg);
            c_input_file_named = 1U;
            break;
        case 'h':
            print_help(argc, argv);
            c_help = 1U;
            return EXIT_SUCCESS;
            break;
        }
    }

    if (c_read)
    {
        if (!c_output_file_named)
        {
            time_t time_s;
            struct tm st;
            time(&time_s);
            localtime_s(&st, &time_s);
            sprintf(c_output_file_name, "./old_%d_%d_%d.bin", st.tm_hour, st.tm_min, st.tm_sec);
        }
    }

    if (c_write)
    {
        if (!c_input_file_named)
        {
            sprintf(c_input_file_name, "./new.bin");
        }
    }

    if (!c_write & !c_read & !c_help)
    {
        print_help(argc, argv);
    }
    else
    {
        main_process(c_read,
                     c_write,
                     c_output_file_name,
                     c_input_file_name);
    }
}

int main_process(
    uint8_t c_read,
    uint8_t c_write,
    char *c_output_file_name,
    char *c_input_file_name)
{
    FT_STATUS ftdi_status;
    FT_DEVICE_LIST_INFO_NODE *ftdi_device_list = NULL;
    DWORD ftdi_device_num = 0;

    ftdi_status = FT_CreateDeviceInfoList(&ftdi_device_num);
    if (ftdi_status == FT_OK)
    {
        printf("\r\nNumber of FTDI devices:%ld\r\n", ftdi_device_num);
    }
    else
    {
        printf("\r\nFailed to get number of devices (error : %ld)\r\n", ftdi_status);
        return -1;
    }

    ftdi_device_list = (FT_DEVICE_LIST_INFO_NODE *)calloc(ftdi_device_num, sizeof(FT_DEVICE_LIST_INFO_NODE));
    if (ftdi_device_list == NULL)
    {
        printf("\r\nFailed to creat list\r\n");
        return -1;
    }

    // If no devices available, return
    if (ftdi_device_num == 0)
    {
        // Wait for a key press
        printf("\r\nno devices found\r\n");
        return -1;
    }

    ftdi_status = FT_GetDeviceInfoList(ftdi_device_list, &ftdi_device_num);
    if (ftdi_status == FT_OK)
    {
        printf("\r\n======================================================\r\n");
        for (uint32_t i = 0; i < ftdi_device_num; i++)
        {
            printf("\r\nDevice Index: %d ", i);
            printf("\r\nFlags: %lx ", ftdi_device_list[i].Flags);
            printf("\r\nType: %ld ", ftdi_device_list[i].Type);
            printf("\r\nID: %lx ", ftdi_device_list[i].ID);
            printf("\r\nLocation ID: %lx ", ftdi_device_list[i].LocId);
            printf("\r\nSerial Number: %s ", ftdi_device_list[i].SerialNumber);
            printf("\r\nDescription: %s ", ftdi_device_list[i].Description);
            printf("\r\n");
        }
        printf("\r\n======================================================\r\n");
    }

    int index = 0;
    if (ftdi_device_num > 1)
    {
        printf("\r\nwhich ot open\r\n");
        scanf("%d", &index);
    }

    FT_HANDLE handler;
    ftdi_status = FT_OpenEx((PVOID)(ftdi_device_list[index].LocId), FT_OPEN_BY_LOCATION, &handler);
    if (ftdi_status != FT_OK)
    {
        printf("\r\nFailed to open device (error %ld)\r\n", ftdi_status);
        return -1;
    }

    if (c_read)
    {
        dump_out(c_output_file_name, handler, 0, 128, 1);
    }

    if (c_write)
    {
        dump_in(c_input_file_name, handler, 0, 128, 1);
    }

    free(ftdi_device_list);

    return 0;
}

int dump_out(char *file_name, FT_HANDLE ftHandle, uint16_t offset, uint16_t size, uint8_t auto_checksum)
{
    if (file_name == NULL)
    {
        printf("\r\n error file name\r\n");
        return -2;
    }

    printf("dump data as:%s\r\n", file_name);
    FILE *fp = NULL;
    fopen_s(&fp, file_name, "wb");
    if (fp == NULL)
    {
        printf("\r\n error open file %s\r\n", file_name);
        return -1;
    }

    // read all data out and write to file
    uint16_t *ee_data;
    ee_data = (uint16_t *)calloc(size, sizeof(uint16_t));
    if (ee_data == NULL)
    {
        printf("\r\n error malloc\r\n");
        return -2;
    }

    for (uint32_t i = 0; i < size; i++)
    {
        FT_ReadEE(ftHandle, i + offset, &ee_data[i]);
        fwrite(&ee_data[i], sizeof(ee_data[i]), 1, fp);
    }
    fflush(fp);
    fclose(fp);
    fp = NULL;

    if (auto_checksum)
    {
        // check data
        uint16_t check_sum = 0xAAAA;
        for (uint16_t i = 0; i < size - 1; i++)
        {
            check_sum = ee_data[i] ^ check_sum;
            check_sum = (check_sum << 1) | (check_sum >> 15);
        }
        if (check_sum != ee_data[size - 1])
        {
            printf("\r\n %s, check sum error \r\n", file_name);
        }
    }

    free(ee_data);

    return 0;
}

int dump_in(char *file_name, FT_HANDLE ftHandle, uint16_t offset, uint16_t size, uint8_t auto_checksum)
{
    if (file_name == NULL)
    {
        printf("\r\n error file name\r\n");
        return -2;
    }
    printf("dump in from:%s\r\n", file_name);

    FILE *fp = NULL;
    fopen_s(&fp, file_name, "rb");
    if (fp == NULL)
    {
        printf("\r\n error open file %s\r\n", file_name);
        return -1;
    }

    // read all data out and write to file
    uint16_t *ee_data;
    ee_data = (uint16_t *)calloc(size, sizeof(uint16_t));
    if (ee_data == NULL)
    {
        printf("\r\n error malloc\r\n");
        return -2;
    }

    // read all data from file
    for (uint32_t i = 0; i < 128; i++)
    {
        fread(&ee_data[i], sizeof(ee_data[i]), 1, fp);
    }
    fclose(fp);
    fp = NULL;

    //
    if (auto_checksum)
    {
        uint16_t check_sum = 0xAAAA;
        for (uint16_t i = 0; i < size - 1; i++)
        {
            check_sum = ee_data[i] ^ check_sum;
            check_sum = (check_sum << 1) | (check_sum >> 15);
        }
        if (check_sum != ee_data[size - 1])
        {
            printf("\r\n %s, check sum error \r\n", file_name);
            ee_data[size - 1] = check_sum;
        }
    }

    FT_EraseEE(ftHandle);
    for (uint32_t i = 0; i < size; i++)
    {
        FT_WriteEE(ftHandle, i + offset, ee_data[i]);
    }

    free(ee_data);

    return 0;
}

static void print_help(int argc, char *argv[])
{
    (void)argc;
    fprintf(stdout, "USAGE: %s [-r [-o <file name>]] [-w [-i <file name>]] [-h]\n", argv[0]);
    fprintf(stdout, "read data from eeprom or write data to eeprom of ftdi chip\n");
    fprintf(stdout, "Options:\n");
    fprintf(stdout, "  -r, --read           read data from eeprom\n");
    fprintf(stdout, "  -o, --output_file    output file name, \"old.bin\" by default\n");
    fprintf(stdout, "  -w, --write          write data to eeprom\n");
    fprintf(stdout, "  -i, --input_file     input file name, \"new.bin\" by default\n");
    fprintf(stdout, "  -h, --help           Print this help screen and exit\n");
    fprintf(stdout, "Report bugs to <https://github.com/nemequ/ftdi-eeprom>\n");
}
