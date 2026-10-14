#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mount.h>
#include <ps5/klog.h>

#define SCE_NOTIFICATION_LOCAL_USER_ID_SYSTEM 0xFE

int sceNotificationSend(int userId, bool isLogged, const char* payload);

static void send_notification(const char *msg) {
    char json[1024];
    snprintf(json, sizeof(json),
        "{\n"
        "  \"rawData\": {\n"
        "    \"viewTemplateType\": \"InteractiveToastTemplateB\",\n"
        "    \"channelType\": \"Downloads\",\n"
        "    \"useCaseId\": \"IDC\",\n"
        "    \"toastOverwriteType\": \"Yes\",\n"
        "    \"isImmediate\": true,\n"
        "    \"priority\": 100,\n"
        "    \"viewData\": {\n"
        "      \"icon\": {\n"
        "        \"type\": \"Predefined\",\n"
        "        \"parameters\": {\n"
        "          \"icon\": \"download\"\n"
        "        }\n"
        "      },\n"
        "      \"message\": {\n"
        "        \"body\": \"%s\"\n"
        "      },\n"
        "      \"subMessage\": {\n"
        "        \"body\": \"PS5 Update Blocker\"\n"
        "      }\n"
        "    },\n"
        "    \"platformViews\": {\n"
        "      \"previewDisabled\": {\n"
        "        \"viewData\": {\n"
        "          \"icon\": {\n"
        "            \"type\": \"Predefined\",\n"
        "            \"parameters\": {\n"
        "              \"icon\": \"download\"\n"
        "            }\n"
        "          },\n"
        "          \"message\": {\n"
        "            \"body\": \"%s\"\n"
        "          }\n"
        "        }\n"
        "      }\n"
        "    }\n"
        "  },\n"
        "  \"localNotificationId\": \"UPDATE_BLOCKER\"\n"
        "}",
        msg, msg);

    sceNotificationSend(SCE_NOTIFICATION_LOCAL_USER_ID_SYSTEM, true, json);
}

int main(void) {
    klog_puts("[update_blocker] Starting update unmount payload...\n");

    // Delete any cached PUP files in /update
    unlink("/update/PS5UPDATE.PUP");
    unlink("/update/PS5UPDATE.PUP.net.temp");

    // Unmount /update with MNT_FORCE (0x80000), fallback to standard unmount
    int res = unmount("/update", 0x80000);
    if (res < 0) {
        res = unmount("/update", 0);
    }

    if (res == 0) {
        klog_puts("[update_blocker] /update unmounted successfully.\n");
        send_notification("Update unmounted! Extended storage unlocked.");
    } else {
        klog_printf("[update_blocker] unmount(/update) returned %d (already unmounted or cleaned).\n", res);
        send_notification("/update unmounted / cleaned.");
    }

    return 0;
}
