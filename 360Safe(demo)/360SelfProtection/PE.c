#include "PE.h"


PVOID Safe_GetSymbolAddress(PANSI_STRING SymbolName, PVOID NtImageBase)
{

	PVOID pModuleBase;
	ULONG ModuleSize;

	PVOID SymbolAddr, result = NULL;
	result = Safe_GetAndReplaceSymbol(NtImageBase, SymbolName, NULL, &SymbolAddr);

	return result;
}

//************************************     
// ��������: Safe_GetAndReplaceSymbol     
// ����˵�����˺�������PE�ļ���ȡ�䵼�����ŵĵ�ַ 
//			 ��ָ����ReplaceValue������ReplaceValue�滻���ҵ��ķ��ŵ�ֵ
// IDA��ַ ��
// ��    �ߣ�Mr.M    
// �ο���ַ��
// �������ڣ�2019/12/31     
// �� �� ֵ: PVOID NTAPI     
// ��    ��: PVOID ImageBase     
// ��    ��: PANSI_STRING SymbolName     
// ��    ��: PVOID ReplaceValue     
// ��    ��: PVOID * SymbolAddr     
//************************************  
PVOID NTAPI Safe_GetAndReplaceSymbol(PVOID ImageBase, PANSI_STRING SymbolName, PVOID ReplaceValue, PVOID *SymbolAddr)
{

	PCHAR	AddressOfNames, pSymbolName;
	PVOID symbol_address, result;
	ULONG Size, func_index;
	DWORD NameOrdinals, NumberOfNames, Low, Mid, High;
	long	ret;

	PIMAGE_EXPORT_DIRECTORY pIED;


	pIED = (PIMAGE_EXPORT_DIRECTORY)RtlImageDirectoryEntryToData(ImageBase, TRUE, IMAGE_DIRECTORY_ENTRY_EXPORT, &Size);
	if (!pIED)
		return NULL;

	AddressOfNames = (CHAR *)ImageBase + pIED->AddressOfNames;
	NameOrdinals = (DWORD)((CHAR *)ImageBase + pIED->AddressOfNameOrdinals);
	NumberOfNames = pIED->NumberOfNames;

	Low = 0;
	High = NumberOfNames - 1;
	if ((long)High < 0)
		return NULL;

	while (TRUE)
	{
		Mid = (Low + High) >> 1;
		pSymbolName = (PCHAR)ImageBase + *(PULONG)&AddressOfNames[4 * Mid];
		ret = strcmp(SymbolName->Buffer, pSymbolName);
		if (!ret)
			break;

		if (ret > 0)
		{
			Low = Mid + 1;
		}
		else
		{
			High = Mid - 1;
		}
		if (High < Low)
			break;
	}

	result = NULL;

	if (High >= Low && (func_index = *(WORD *)(NameOrdinals + 2 * Mid), func_index < pIED->NumberOfFunctions))
	{

		symbol_address = (PVOID)((PCHAR)ImageBase + 4 * func_index + pIED->AddressOfFunctions);

		result = (CHAR *)ImageBase + *(PULONG)symbol_address;

		*SymbolAddr = symbol_address;

		return result;
	}

	return result;

}

ULONG NTAPI Safe_RvaToVa(IN PVOID pModuleBase, ULONG dwRva)
{
	IMAGE_DOS_HEADER *pDosHeader = (IMAGE_DOS_HEADER*)pModuleBase;

	IMAGE_NT_HEADERS *pNtHeader =   /*����*/
		(IMAGE_NT_HEADERS*)((DWORD)pDosHeader + pDosHeader->e_lfanew);

	// �õ����θ���
	ULONG   dwSectionNumber = pNtHeader->FileHeader.NumberOfSections;

	// �õ���һ������
	IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

	// �������α��ҵ�RVA���ڵ�����
	/*
	* ÿ��ƫ�ƣ����������ļ��У��������ڴ��У����Ǿ������ο�ʼλ�õľ���
	* ������ȵġ�
	* ���ң����α��У�������������ʼƫ�ƣ�
	*  1. �ļ��еĿ�ʼƫ��
	*  2. �ڴ��еĿ�ʼƫ��
	* ������̣�
	*  �ҵ�RVA�������Σ� Ȼ���������RVA���������ڴ��еĿ�ʼλ�õľ��롣
	*  �������������������ļ��еĿ�ʼλ�þ͵õ��ļ�ƫ����
	*/

	for (ULONG i = 0; i < dwSectionNumber; ++i) {

		// �ж�RVA�Ƿ��ڵ�ǰ��������

		DWORD dwSectionEndRva =   /*����*/
			pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData;

		if (dwRva >= pSectionHeader[i].VirtualAddress
			&& dwRva <= dwSectionEndRva) {

			// �����RVA��Ӧ���ڴ�ƫ��
			// ��ʽ��
			// �ļ�ƫ��  =  RVA - ���ε���ʼRVA + ���ε���ʼ�ļ�ƫ��
			// �ڴ�ƫ��  =  �ļ�ƫ�� + pModuleBase
			ULONG dwTemp = dwRva - pSectionHeader[i].VirtualAddress;
			ULONG dwOffset = dwTemp + pSectionHeader[i].PointerToRawData;
			ULONG dwVa = dwOffset + (ULONG)pModuleBase;
			return dwVa;
		}
	}
	return 0;
}



//���PE�ļ�������Ϣ
BOOLEAN NTAPI Safe_CheckPeFile(IN PVOID pModuleBase)
{
	// PE�ļ����еĽṹ�嶼���� IMAGE_ ��ͷ
	//PIMAGE_DOS_HEADER => IMAGE_DOS_HEADER*
	IMAGE_DOS_HEADER* pDosHeader = (IMAGE_DOS_HEADER*)pModuleBase;

	// �жϵ�һ���ֶ��Ƿ�MZ
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
		return FALSE;
	}

	// �ж��Ƿ�����Ч��NTͷ
	IMAGE_NT_HEADERS* pNtHeader =
		(IMAGE_NT_HEADERS*)(pDosHeader->e_lfanew + (DWORD)pDosHeader);

	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return FALSE;
	}

	//��������Ϊ0
	if (!pNtHeader->FileHeader.NumberOfSections)
	{
		return FALSE;
	}
	return TRUE;
}

//������
BOOLEAN NTAPI Safe_17C8A(IN PVOID pModuleBase, IN ULONG ModuleSize)
{
	PIMAGE_DOS_HEADER  			pDH = NULL;
	PIMAGE_NT_HEADERS			pNtH = NULL;
	PIMAGE_DATA_DIRECTORY       pDataDirectory = NULL;
	//1��PE��־λ�ж�
	if (!Safe_CheckPeFile(pModuleBase))
	{
		return FALSE;
	}
	// �õ�DOSͷ
	pDH = (PIMAGE_DOS_HEADER)pModuleBase;
	// �õ�Ntͷ��Ϊ�˵õ���չͷ�����ҵ�Ntͷ��
	pNtH = (PIMAGE_NT_HEADERS)((PCHAR)pModuleBase + pDH->e_lfanew);

	//2����ȡ��Դ���ַ�ʹ�С
    pDataDirectory = (IMAGE_DATA_DIRECTORY*)&pNtH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE];
	//RVAת����Va
	PIMAGE_RESOURCE_DIRECTORY pRoot = Safe_RvaToVa(pModuleBase, pDataDirectory->VirtualAddress);
	IMAGE_RESOURCE_DIRECTORY*    pDir2;// ��ԴĿ¼
	IMAGE_RESOURCE_DIRECTORY*    pDir3;// ��ԴĿ¼

	IMAGE_RESOURCE_DIRECTORY_ENTRY* pEntry1;//Ŀ¼���
	IMAGE_RESOURCE_DIRECTORY_ENTRY* pEntry2;//Ŀ¼���
	IMAGE_RESOURCE_DIRECTORY_ENTRY* pEntry3;//Ŀ¼���

	IMAGE_RESOURCE_DATA_ENTRY*      pDataEntry;// ��Դ�������
	IMAGE_RESOURCE_DIR_STRING_U*    pIdString; // ����Id���ַ���

	// +----------------+
	// |    Ŀ¼        |
	// +----------------+
	//    +-------------+      |- id(���ַ����͵�ID,�����͵�ID)
	//    |   Ŀ¼���   | ==> |
	//    +-------------+      |- ƫ��(����ƫ�Ƶ�Ŀ¼,����ƫ�Ƶ�����)
	//    +-------------+ 
	//    |   Ŀ¼���   | 
	//    +-------------+ 
	//    +-------------+ 
	//    |   Ŀ¼���   | 
	//    +-------------+ 
	/* �ѵ�һ�����е�Ŀ¼��ڶ��������� */
	// �õ���һ��Ŀ¼��ڵĵ�ַ,��һ��Ŀ¼������IMAGE_RESOURCE_DIRECTORY����
	pEntry1 = (IMAGE_RESOURCE_DIRECTORY_ENTRY*)(pRoot + 1);
	for (ULONG i = 0;
		i < pRoot->NumberOfIdEntries + pRoot->NumberOfNamedEntries;
		i++, pEntry1++) {

	}
	return TRUE;
}
