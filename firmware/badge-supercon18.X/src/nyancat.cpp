#include <xc.h>
#include "badge_user.h"
#include "nyancat.h"


uint32_t play_next_note(uint8_t measure_index, uint8_t note_index, uint8_t *next_note)
{
    uint32_t duration = 0;
    uint8_t  nyancat_index = 0;
    uint8_t  voice_index = 0;

    *next_note = 0;

    if (measure_index > 8)
    {
        return duration;
    }

    nyancat_index = note_index * 4;
    if (nyancat_index < nyancat_measure_limits[measure_index])
    {
        for (voice_index = 0; voice_index < 3; voice_index++)
        {
            sound_set_note(nyancat_measures[measure_index][nyancat_index+voice_index],voice_index);
        }

        duration = nyancat_durations[nyancat_measures[measure_index][nyancat_index+3]];
        *next_note = note_index+1;
    }

    return duration;
}

void nyancat(void)
{
    uint16_t x = 0;
    uint8_t y = 0;

    // 320 pixels wide line, each 32-bit value in 0xNNRRGGBB format where
    // NN = Not used, RR = 8-bit red, GG = 8-bit green, BB = 8-bit blue
    uint32_t line[320];

    uint16_t i = 0;
    uint32_t pixel_idx = 0;
    uint16_t encoded_run = 0;
    uint32_t pal_entry = 0;
    uint16_t pal_run = 0;
    uint8_t frame_idx = 0;

    // Image scaling control. 1=full scale, 2=half scale, 4=quarter, etc.
    const uint8_t multiplier = 4; // Only quarter-resolution version is on badge by default.
    uint8_t multiply_loop=0;

    // Animation speed control
    const uint8_t milliseconds_per_frame = 70;
    uint32_t time_for_next_frame = millis();

    // Audio playback control
    // audio_state is the state machine that sequences measures into a song.
    // Each measure has zero or more 'notes' (different from sheet music notes)
    // Each 'note' is a set of three frequencies to be held for a specific time.
    uint32_t time_for_audio_update = millis();
    uint8_t audio_state = 0;
    uint8_t measure_index = 0;
    uint8_t note_index = 0;
    uint8_t next_note = 0;
    uint8_t mute = 0;

    enable_display_scanning(0); //Shut off auto-scanning of character buffer
    
    while(1) //Loop forever
    {
        // Is it time for the next frame?
        if (millis() >= time_for_next_frame)
        {
            time_for_next_frame += milliseconds_per_frame;

            pixel_idx=0;
            if(frame_idx > 11)
            {
                frame_idx = 0;
            }

            for (y=0;y<240;y+=multiplier)
            {
                x = 0;

                while (x < 320)
                {
                    encoded_run = cat4_frames[frame_idx][pixel_idx];
                    pal_entry = cat4_palette[(encoded_run>>12)&0xF];
                    pal_run = encoded_run&0xFFF;
                    for (i = 0; i < pal_run; i++)
                    {
                        for (multiply_loop=0; multiply_loop<multiplier; multiply_loop++)
                        {
                            line[x+i*multiplier+multiply_loop] = pal_entry;
                        }
                    }
                    x += pal_run*multiplier;
                    pixel_idx++;
                }

                tft_set_write_area(0,y,319,multiplier);
                TFT_24_7789_Write_Command(0x2C);
                for (multiply_loop=0; multiply_loop<multiplier; multiply_loop++)
                {
                    for (i=0;i<320;i++)
                    {
                        pal_entry = line[i];
                        TFT_24_7789_Write_Data3((pal_entry>>16)&0xFF,(pal_entry>>8)&0xFF,(pal_entry>>0)&0xFF);
                    }
                }
            }
            frame_idx++;
        }

        // Is it time for the next audio update?
        if (millis() >= time_for_audio_update)
        {
            if (audio_state != 0xFF)
            {
                measure_index = nyancat_sequence[audio_state];

                time_for_audio_update += play_next_note(measure_index,note_index,&next_note);
                if (next_note==0)
                {
                    audio_state++;
                    if (audio_state >= NYANCAT_SEQUENCES)
                    {
                        audio_state = 0;
                    }
                }
                note_index = next_note;
            }
            else
            {
                // Fallback - silence for 10 seconds.
                sound_set_note(0,0);
                sound_set_note(0,1);
                sound_set_note(0,2);
                time_for_audio_update += 10000;
            }
        }

        // Check for specific keys that change our behavior.
        K_R1 = 0;

        if (K_C9==0) // '0' immediately mutes music.
        {
            mute = 1;
            sound_set_note(0,0);
            sound_set_note(0,1);
            sound_set_note(0,2);
            audio_state = 0xFF;
        }
        else if (K_C6==0 && mute) // '9' will restart music if muted, otherwise no effect.
        {
            mute = 0;

            time_for_audio_update = millis();
            audio_state = 0;
            note_index = 0;
        }

        K_R1 = 1;
    }
    //Badge will need to be reset to continue    
}