/*
  this file is an implement of a html parser
 */
#define NAME_LEN 128
int parse_html(char *buf, char *tag, char *attr)
{
    
}
char *next_tag(char *buf)
{
    char *tag_start = strchr(buf, '<');
    if (!tag_start)
    {
        return NULL;
    }
    /* skip comment */
    if (startswith(tag_start, "<!--"))
    {
        
    }
    char tag_name[NAME_LEN];
    char *ptr = tag_start + 1;
    for (i = 0; i < NAME_LEN - 1 && *ptr != ' '; ++i, ++ptr)
    {
        tag_name[i] = *ptr;
    }
    tag_name[i] = '\0';
    char *tag_end = strchr(tag_start, '>')
}
