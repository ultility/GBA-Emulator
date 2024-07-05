#include "LCD-video-controller.h"

void LCD_video_controller_init(struct LCD_video_controller* lcd)
{
    for (int i = 0; i < LCD_VIDEO_CONTROLLER_REGISTERS_COUNT; i++)
    {
        lcd->registers[i] = 0;
    }
}

void LCD_video_controller_process_request(struct LCD_video_controller* lcd, struct request_data* request)
{
    int reg = request->address - 0x4000000;
    reg /= sizeof(HALF_WORD);
    switch (request->request_type)
    {
        case input: // return value
            break;
        case output: // set value
            lcd->registers[reg] = request->data.half_word;
            break;
    }
}