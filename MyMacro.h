#ifndef __MYMACRO_H__
#define __MYMACRO_H__

// �ַ�����
#define SAVE_PATH                       L"Temp\\"
#define DOWNLOADING                     L" - ���������ļ�, ���Ժ�..."
#define PUSHING                         L" - �������, ���Ժ�..."
#define WARNING                         L"����"
#define TIP                             L"��ʾ"

// ��ʱ��ʱ��
#define ID_EVENT_DOWNLOADING            0
#define ID_EVENT_PUSHING                1
#define TIMER_SECOND                    50 

// �Զ�����Ϣ
#define WM_FILE_INFO_UPADTE             (WM_USER+1)
#define WM_LOG_GIT_INFO                 (WM_USER+2) 
#define WM_COMPLETE_DOWNLOAD            (WM_USER+3)
#define WM_SHOW_FILE_INFO               (WM_USER+4)
#define WM_COPY_INFO                    (WM_USER+5)
#define WM_DOWNLOAD_FINISHED            (WM_USER+6)

#define RECVPACK_SIZE                   2048
#define THREADPOOL_SIZE                 5


#endif

