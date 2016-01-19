#ifndef ipcio_h
#define ipcio_h
#include <SDKDDKVer.h>
#include <string.h>

//有效的用户数据长度
#define MAX_IPC_BUF 4096

typedef enum tag_OP_MSG{
	OP_BEGIN = 0,
	OP_CONNECT, //接收到连接
	OP_CONNECT_ACK, //响应连接
	OP_EXCHANGE, //数据交换
	OP_DISCONNECT, //接收到断开
	OP_DISCONNECT_ACK, //响应断开
	OP_PREPARE_STOP_RECV, //停止接收
	OP_CHECK, //检查是否可以退出接收线程
	OP_ERR_REMOVE, //发生错误,移除出发送队列
	OP_END,
}OP_MSG;

//IPC当前状态
typedef enum tag_IPC_STATE{
	IPC_STATE_BEGIN,
	IPC_STATE_CONNECT_WAIT, //向另一端发送连接请求后,进入等待状态
	IPC_STATE_ESTABLISHED, //建立连接状态
	IPC_STATE_CLOSE_WAIT, //向另一端发送关闭,进入关闭等待
	IPC_STATE_CLOSED,  //连接已关闭
	IPC_STATE_ERROR, //发生错误
	IPC_STATE_END,
}IPC_STATE;


struct _IPC_MESSAGE
{
	int m_iID; //数据包ID，用与一组数据的拆包与组包处理
	unsigned int m_nLen; //数据总长度
	unsigned int m_nEffectLen; //当前包的有效长度
	//unsigned int m_nRemindPack; //还剩n个数据分包。 0:当前收到的是最后一个分包
	unsigned int m_nTotalPack;//总共多少个分包
	unsigned int m_nOrder; //包的序号
	unsigned int m_nOffset; //相对整个数据的偏移量
	unsigned char m_buf[MAX_IPC_BUF]; //**注** 修改大小,从MAX_IPC_BUF宏定义中修改
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