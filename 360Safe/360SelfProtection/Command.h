#include <ntifs.h>
#include "HookPortDeviceExtension.h"

//������Ȥ��ͨ�ô���
NTSTATUS Safe_CommonProc(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp);

//����
NTSTATUS Safe_Shutdown(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//��
NTSTATUS Safe_Read(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);


//������������CLEANUP
NTSTATUS Safe_CreateCloseCleanup(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

//��Ϣѭ����
NTSTATUS Safe_DeviceControl(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);


/**************�ٿذ������б�����****************/
//����������
#define SAFE_INSERTWHITELIST__PID_2003				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0803, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��Ӱ��������� Win2003
#define SAFE_INSERTWHITELIST__PID					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0810, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��Ӱ���������

#define SAFE_DELETEWHITELIST_PID					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0804, METHOD_BUFFERED, FILE_ANY_ACCESS)		//ɾ������������
#define SAFE_DELETEWHITELIST_PID_SESSIONID			CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080F, METHOD_BUFFERED, FILE_ANY_ACCESS)		//ɾ������������
#define SAFE_QUERYWHITE_PID							CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0813, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��ѯ����������

//�Ϸ�����·������Ӱ���������
#define SAFE_CHECK_INSERTWHITELIST__PID				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080E, METHOD_BUFFERED, FILE_ANY_ACCESS)		//�Ϸ�����·������Ӱ���������

//R3��R0ͨѶ���������������
#define SAFE_INSERTSPECIALWHITELIST_PID				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0806, METHOD_BUFFERED, FILE_ANY_ACCESS)		//Safe_InsertSpecialWhiteList_PID
/**************�ٿذ������б�����****************/

/**************δʶ������************************/
#define SAFE_UNKNOWN								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0800, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //
#define SAFE_UNKNOWN1								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0801, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //
#define SAFE_UNKNOWN2								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0802, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //
#define SAFE_UNKNOWN3								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0827, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //
#define SAFE_UNKNOWN6								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0825, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //R3�����ַ�����R0ĳ��ȫ�ֱ���

//��0 or ��1δ���������±�����
#define SAFE_UNKNOWN4								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0828, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //��0�������ݣ� dword_34DAC[0],dword_34DAC[1]
#define SAFE_UNKNOWN5								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0829, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //��0�������ݣ� dword_34DAC[2],dword_34DAC[3]
#define SAFE_UNKNOWN7								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0814, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //��1�������ݣ� dword_34DAC[9]
#define SAFE_UNKNOWN8								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0815, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //��0�������ݣ� dword_34DAC[9]
#define SAFE_UNKNOWN9								CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0823, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //�����������ݣ�dword_34DAC[10]


//��������g_dword_34678
#define SAFE_GET_DWORD_34678						CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0821, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //R3����R0  ����
#define SAFE_SET_DWORD_34678						CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0820, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //R0����R3  ���
/**************δʶ������************************/

/**************��ȡ�汾����**********************/
#define SAFE_GETVER									CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0805, METHOD_BUFFERED, FILE_ANY_ACCESS)	    //���ظ�R3����汾
/**************��ȡ�汾����**********************/

/******����������ʾʱ��Ӧ�ò��ͨ�Ž�������*****/
#define SAFE_INITIALIZE_SETEVENT					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0807, METHOD_BUFFERED, FILE_ANY_ACCESS)     //��ʼ���¼�
/******����������ʾʱ��Ӧ�ò��ͨ�Ž�������*****/

/*************�ٿ�HookPort�����ӿ�����************/
#define SAFE_SET_FAKEFUNCTION						CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0808, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���or ���� Fake����
#define SAFE_GET_FAKEFUNCTION_SWITCH				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0809, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��ȡFake�������״̬��1���㣬0�ҹ�
#define SAFE_GET_ISFILTERFUNFILLEDREADY				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x082B, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��ȡ���˺�����״̬����1����Fake������������0����Fake�����رա��������Ϊ��Դ��բ
/*************�ٿ�HookPort�����ӿ�����************/

/**************�ٿغڰ�������������**************/
#define SAFE_DELETEDRVMKDATALIST					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080A, METHOD_BUFFERED, FILE_ANY_ACCESS)		//ɾ���ڰ���������
#define SAFE_INSERTDRVMKDATALIST					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080B, METHOD_BUFFERED, FILE_ANY_ACCESS)		//��Ӻڰ���������
#define SAFE_RESETDRVMKDATALIST						CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0819, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���úڰ�������������������
/**************�ٿغڰ�������������**************/

/**************�ٿ����������ر�����************/
#define SAFE_OFF_360SAFEBOX_SWITCH					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x080D, METHOD_BUFFERED, FILE_ANY_ACCESS)		//�ر�360safeBoxѡ��
#define SAFE_NO_360SAFEBOX_SWITCH					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0811, METHOD_BUFFERED, FILE_ANY_ACCESS)		//����360safeBoxѡ��

#define SAFE_SET_SPSHADOW0_DATA_DWORD				CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0812, METHOD_BUFFERED, FILE_ANY_ACCESS)		//����Safe_SetRegedit_SpShadow0����

//���� or �ر�Fake_ZwSetSystemInformation��SystemLoadAndCallImage�������
#define SAFE_GET_SYSTEMHOTPATCHINFORMATION_SWITCH   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x0826, METHOD_BUFFERED, FILE_ANY_ACCESS)		//���� or �ر�SystemLoadAndCallImage�ļ��
/**************�ٿ�ĳ���������ر�����************/

/*************ClientLoadLibrary��������************/
#define SAFE_SET_ILLEGALITYDLLPATH					CTL_CODE(FILE_DEVICE_UNKNOWN, 0x082A, METHOD_BUFFERED, FILE_ANY_ACCESS)		//Υ��DLL·����ClientLoadLibraryʹ��
/*************ClientLoadLibrary��������************/


















