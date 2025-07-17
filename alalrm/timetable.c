#include "timetable.h"

#include "cJSON.h"
#include <string.h>
#include <stdio.h>

const static char config_empty[] = "{\"code\":252,\"PlanFeed\":{\"plantable\":[],\"once\":[]}}";

// 闹钟文本
static char *alarm_text = NULL;

// 闹钟列表
static tt_list_t *tt_manager = NULL;

// hhmmss字符串转int
static uint32_t str_to_time(const char *str)
{
    int h, m, s;
    sscanf(str, "%2d%2d%2d", &h, &m, &s);
    return h * 10000 + m * 100 + s;
}

// 7位字符串转bitmask，bit0=周日
static uint8_t str_to_weekmask(const char *str)
{
    uint8_t mask = 0;
    for (int i = 0; i < 7; i++) {
        if (str[i] == '1') {
            mask |= (1 << (6 - i)); // 注意：date_str[0] 是周日 → bit6
        }
    }
    return mask;
}

static int hhmmss_to_sec(int hhmmss)
{
    int hh = hhmmss / 10000;
    int mm = (hhmmss / 100) % 100;
    int ss = hhmmss % 100;
    return hh * 3600 + mm * 60 + ss;
}

void alarm_Malloc(void)
{
    // 文本存储空间
    alarm_text = malloc(JSON_BUFFER_SIZE);
    if(!alarm_text)
    {
        goto err_text;
    }
    memset(alarm_text,0,JSON_BUFFER_SIZE);
    
    // 闹钟链表空间
    tt_manager = malloc(sizeof(tt_list_t));
    if(!tt_manager)
    {
        goto err_manager;
    }
    memset(tt_manager,0,sizeof(tt_list_t));
    return;
err_manager:
free(alarm_text);
err_text:
return;
}

void alarm_ReadFile(void)
{
    FILE *fp = NULL;
    fp = fopen(ALARM_PATH,"r");
    if(fp)
    {
        fread(alarm_text,JSON_BUFFER_SIZE,1,fp);
        fclose(fp);
    }
    else
    {
        fp = fopen(ALARM_PATH,"w+");
        if(fp == NULL)
        {
            N_Deg("fopen fail\n");
            return -1;
        }
        else
        {
            fwrite(config_empty,sizeof(config_empty),1,fp);
            fclose(fp);
            
            fp = NULL;
            fp = fopen(ALARM_PATH,"r");
            if(fp == NULL)
            {
                N_Deg("fopen fail\n");
                return -1;
            }
            fread(alarm_text,JSON_BUFFER_SIZE,1,fp);
            fclose(fp);
        }
    }
    return;
}

int alarm_parse_json(void)
{
    memset(tt_manager, 0, sizeof(tt_list_t));
    if (!alarm_text || !tt_manager) return -1;

    cJSON *root = cJSON_Parse(alarm_text);
    if (!root) return -2;

    cJSON *planfeed = cJSON_GetObjectItem(root, "PlanFeed");
    if (!planfeed) {
        cJSON_Delete(root);
        return -3;
    }

    // 处理计划闹钟
    cJSON *plantable = cJSON_GetObjectItem(planfeed, "plantable");
    if (plantable) {
        int size = cJSON_GetArraySize(plantable);
        for (int i = 0; i < size && tt_manager->count < ALARM_TOTAL_MAX; i++) {
            cJSON *item = cJSON_GetArrayItem(plantable, i);
            if (!item) continue;

            cJSON *time = cJSON_GetObjectItem(item, "time");
            cJSON *date = cJSON_GetObjectItem(item, "date");
            cJSON *sta  = cJSON_GetObjectItem(item, "sta");
            cJSON *measure = cJSON_GetObjectItem(item, "measure");

            if (time && date && sta && measure) {
                tt_entry_t *entry = &tt_manager->list[tlist->count++];
                entry->plan.time = str_to_time(time->valuestring);
                entry->plan.repeat = str_to_weekmask(date->valuestring);
                entry->plan.enabled = sta->valueint;
                entry->plan.measure = measure->valueint;
                entry->is_once = 0;
            }
        }
    }

    // 处理一次性闹钟
    cJSON *once = cJSON_GetObjectItem(planfeed, "once");
    if (once) {
        int size = cJSON_GetArraySize(once);
        for (int i = 0; i < size && tt_manager->count < ALARM_TOTAL_MAX; i++) {
            cJSON *item = cJSON_GetArrayItem(once, i);
            if (!item) continue;

            cJSON *time = cJSON_GetObjectItem(item, "time");
            cJSON *sta  = cJSON_GetObjectItem(item, "sta");
            cJSON *measure = cJSON_GetObjectItem(item, "measure");

            if (time && sta && measure) {
                tt_entry_t *entry = &tt_manager->list[tt_manager->count++];
                entry->once.time = str_to_time(time->valuestring);
                entry->once.enabled = sta->valueint;
                entry->once.measure = measure->valueint;
                entry->is_once = 1;
            }
        }
    }

    cJSON_Delete(root);
    return 0;
}

int alarm_find_next(const tt_list_t *alist, int now_sec, int weekday, tt_trigger_t *trigger)
{
    uint32_t min_time = UINT32_MAX;
    int found_index = -1;
    int alarm_sec = 0;
    int cmp_sec = UINT32_MAX;
    uint8_t found_once = 0;

    uint8_t tomorrow = (weekday + 1) % 7;
    const tt_entry_t *e = NULL;

    for (int i = 0; i < alist->count; i++) {
        e = &alist->list[i];
        if (!e->is_once) {
            if (!e->plan.enabled) continue;

            alarm_sec = hhmmss_to_sec(e->plan.time);
            if (((e->plan.repeat >> (6 - weekday)) & 0x01) && alarm_sec >= now_sec) {
                cmp_sec = alarm_sec;
            } else if ((e->plan.repeat >> (6 - tomorrow)) & 0x01) {
                cmp_sec = alarm_sec + 86400;
            }

            if (cmp_sec < min_time) {
                min_time = cmp_sec;
                found_index = i;
                found_once = 0;
            }
        } else {
            if (!e->once.enabled) continue;

            alarm_sec = hhmmss_to_sec(e->once.time);
            cmp_sec = (alarm_sec >= now_sec) ? alarm_sec : alarm_sec + 86400;

            if (cmp_sec < min_time) {
                min_time = cmp_sec;
                found_index = i;
                found_once = 1;
            }
        }
    }

    if (found_index < 0) return 0;

    trigger->time = alist->list[found_index].is_once ?
                    alist->list[found_index].once.time :
                    alist->list[found_index].plan.time;

    trigger->is_once = found_once;
    trigger->index = found_index;
    return 1;
}

inline void alarm_handle_once_trigger( const tt_trigger_t *trigger)
{
    if (trigger->is_once && trigger->index < tt_manager->count)
    {
        if(tt_manager->list[trigger->index].is_once == 1)
            tt_manager->list[trigger->index].once.enabled = 0;
    }
}

void alarm_to_json_raw(void)
{
    if (!alarm_text) return NULL;

    char *p = alarm_text;
    FILE *fp = NULL;
    int len = 0;
    len += snprintf(p + len, JSON_BUFFER_SIZE - len, "{\"code\":252,\"PlanFeed\":{\"plantable\":[");
    int first_plan = 1;
    int first_once = 1;

    for (int i = 0; i < tt_manager->count; i++) {
        const tt_entry_t *e = &tt_manager->list[i];
        if (!e->is_once) {
            const tt_plan_t *a = &e->plan;
            char date_str[8] = {0};
            for (int j = 0; j < 7; j++) {
                date_str[j] = ((a->repeat >> (6 - j)) & 0x01) ? '1' : '0';
            }
            len += snprintf(p + len, JSON_BUFFER_SIZE - len,
                            "%s{\"time\":\"%06d\",\"date\":\"%s\",\"sta\":%d,\"measure\":%d}",
                            first_plan ? "" : ",", a->time, date_str, a->enabled, a->measure);
            first_plan = 0;
        }
    }

    len += snprintf(p + len, JSON_BUFFER_SIZE - len, "],\"once\":[");
    for (int i = 0; i < tt_manager->count; i++) {
        const tt_entry_t *e = &tt_manager->list[i];
        if (e->is_once) {
            const tt_once_t *a = &e->once;
            len += snprintf(p + len, JSON_BUFFER_SIZE - len,
                            "%s{\"time\":\"%06d\",\"sta\":%d,\"measure\":%d}",
                            first_once ? "" : ",", a->time, a->enabled, a->measure);
            first_once = 0;
        }
    }

    len += snprintf(p + len, JSON_BUFFER_SIZE - len, "]}}");

    fp = fopen(ALARM_PATH,"w+");
    if(fp == NULL)
    {
        N_Deg("fopen fail\n");
        return -1;
    }
    else
    {
        fwrite(config_empty,sizeof(config_empty),1,fp);
        fclose(fp);
        
        fp = NULL;
        fp = fopen(ALARM_PATH,"r");
        if(fp == NULL)
        {
            N_Deg("fopen fail\n");
            return -1;
        }
        fread(alarm_text,JSON_BUFFER_SIZE,1,fp);
        fclose(fp);
    }
    return ;
}

