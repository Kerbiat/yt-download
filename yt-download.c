#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

extern int errno;

/*
 * Execute command
 * Copy stdout into a string
 */
void shell(char *command, char *output, int max_length)
{
    FILE *output_stream;
    output_stream = popen(command, "r");

    if (output_stream == NULL) {
        perror("ERROR");
        exit(1);
    }

    int ch;
    int i = 0;

    while ((ch = fgetc(output_stream)) != EOF
            && i < max_length) {
        output[i] = ch;
        i++;
    }

    output[i] = '\0';

    if (pclose(output_stream) == 1) {
        perror("ERROR");
        exit(1);       
    }
}

int main(const int argc, char const *argv[])
{   
    time_t start_time = time(NULL); 


    /* CONFIG */

    /* Video directory path */
    char DIRECTORY[] = "/home/main/Videos/";


    /* Check dependencies */

    int status;

    status = system("which youtube-dl >/dev/null");

    if (status != 0) {
        fprintf(stderr, "ERROR: youtube-dl is not in the PATH\n");
        exit(1);
    }

    status = system("which ffmpeg >/dev/null");
    
    if (status != 0) {
        fprintf(stderr, "ERROR: ffmpeg is not in the PATH\n");
        exit(1);
    }


    /* Get video link from clipboard */

    char link[1024];
    
    shell("xsel -bo 2>/dev/null", link, sizeof(link));

    char yt_link_head[32] = "https://www.youtube.com/watch?v=";

    if (strlen(link) < 32 || strncmp(link, yt_link_head, 32) != 0) {
        fprintf(stderr, "ERROR: Clipboard does not contain a youtube link\n");
        exit(1);
    }

    size_t max_length = sizeof(link);

    /* Ommit parameters in youtube link after '&' */
    for (int i = 0; link[i] != '\0' && i < max_length; i++) {
        if (link[i] == '&')
            link[i] = '\0';
    }

    /* Remove temporary files in case they exist */
    /* youtube-dl will not overwrite existing files on download */

    char cmd_rm[1024];

    strcpy(cmd_rm, "rm -f \"");
    strcat(cmd_rm, DIRECTORY);
    strcat(cmd_rm, "temp-video.webm\" \"");
    strcat(cmd_rm, DIRECTORY);
    strcat(cmd_rm, "temp-audio.webm\"");

    system(cmd_rm);


    /* Download video */

    char cmd_video[1024];

    strcpy(cmd_video, "youtube-dl -f 302/247/244/243/242/279 -o \"");
    strcat(cmd_video, DIRECTORY);
    strcat(cmd_video, "temp-video.webm\" \"");
    strcat(cmd_video, link);
    strcat(cmd_video, "\" 1>/dev/null");

    status = system(cmd_video);

    if (status != 0) {
        fprintf(stderr, "ERROR: youtube-dl failed to download video\n");
        exit(1);
    }


    /* Download audio */

    char cmd_audio[1024];

    strcpy(cmd_audio, "youtube-dl -f 251/250/249 -o \"");
    strcat(cmd_audio, DIRECTORY);
    strcat(cmd_audio, "temp-audio.webm\" \"");
    strcat(cmd_audio, link);
    strcat(cmd_audio, "\" 1>/dev/null");

    status = system(cmd_audio);

    if (status != 0) {
        fprintf(stderr, "ERROR: youtube-dl failed to download audio\n");
        exit(1);
    }


    /* Combine video and audio */

    char cmd_ffmpeg[1024];

    strcpy(cmd_ffmpeg, "ffmpeg -i ");
    strcat(cmd_ffmpeg, DIRECTORY);
    strcat(cmd_ffmpeg, "temp-video.webm -i ");
    strcat(cmd_ffmpeg, DIRECTORY);
    strcat(cmd_ffmpeg, "temp-audio.webm -c copy ");
    strcat(cmd_ffmpeg, "-map 0:v:0 -map 1:a:0 -shortest -y ");
    strcat(cmd_ffmpeg, DIRECTORY);
    strcat(cmd_ffmpeg, "temp-result.webm 2>/dev/null");

    status = system(cmd_ffmpeg);

    if (status != 0) {
        fprintf(stderr, "ERROR: ffmpeg failed to combine files\n");
        exit(1);
    }


    /* Remove temporary files */

    system(cmd_rm);


    double running_time = difftime(time(NULL), start_time);
    printf("Success. Ran in %.0f seconds\n", running_time);

    exit(0);
}