#ifndef ipcio_h
#define ipcio_h
#include <SDKDDKVer.h>
#include <string.h>

//��Ч���û����ݳ���
#define MAX_IPC_BUF 4096

typedef enum tag_OP_MSG{
	OP_BEGIN = 0,
	OP_CONNECT, //���յ�����
	OP_CONNECT_ACK, //��Ӧ����
	OP_EXCHANGE, //���ݽ���
	OP_DISCONNECT, //���յ��Ͽ�
	OP_DISCONNECT_ACK, //��Ӧ�Ͽ�
	OP_PREPARE_STOP_RECV, //ֹͣ����
	OP_CHECK, //����Ƿ�����˳������߳�
	OP_ERR_REMOVE, //��������,�Ƴ������Ͷ���
	OP_END,
}OP_MSG;

//IPC��ǰ״̬
typedef enum tag_IPC_STATE{
	IPC_STATE_BEGIN,
	IPC_STATE_CONNECT_WAIT, //����һ�˷������������,����ȴ�״̬
	IPC_STATE_ESTABLISHED, //��������״̬
	IPC_STATE_CLOSE_WAIT, //����һ�˷��͹ر�,����رյȴ�
	IPC_STATE_CLOSED,  //�����ѹر�
	IPC_STATE_ERROR, //��������
	IPC_STATE_END,
}IPC_STATE;


struct _IPC_MESSAGE
{
	int m_iID; //���ݰ�ID������һ�����ݵĲ�����������
	unsigned int m_nLen; //�����ܳ���
	unsigned int m_nEffectLen; //��ǰ������Ч����
	//unsigned int m_nRemindPack; //��ʣn�����ݷְ��� 0:��ǰ�յ��������һ���ְ�
	unsigned int m_nTotalPack;//�ܹ����ٸ��ְ�
	unsigned int m_nOrder; //�������
	unsigned int m_nOffset; //����������ݵ�ƫ����
	unsigned char m_buf[MAX_IPC_BUF]; //**ע** �޸Ĵ�С,��MAX_IPC_BUF�궨�����޸�
	_IPC_MESSAGE(){
		//m_nRemindPack = 0;
		m_iID = 0;
		m_nEffectLen = 0;
		m_nOrder = 0;
		m_nOffset = 0;
		m_nTotalPack = 0;
	}
	_IPC_MESSAGE(int iID, unsigned int nLen, unsigned int nOrder,  unsigned int nTotalPack, unsigned int nOffset){
		//m_nRemindPack = nRemindPack;
		m_iID = iID;
		m_nLen = nLen;
		m_nOrder = nOrder;
		m_nOffset = nOffset;
		m_nTotalPack = nTotalPack;
	}
};

#endif